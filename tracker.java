// tracker.java
import java.io.*;
import java.nio.file.*;
import java.time.*;
import java.time.format.*;
import java.util.*;
import com.google.gson.*; // install gson

public class tracker {
    private static final String RESET = "\u001B[0m";
    private static final String GREEN = "\u001B[92m";
    private static final String RED = "\u001B[91m";
    private static final String YELLOW = "\u001B[93m";
    private static final String BLUE = "\u001B[94m";

    private static String colorize(String text, String color) {
        return color + text + RESET;
    }

    private static class Session {
        String start;
        String end;
        int duration;
    }

    private static class Data {
        List<Session> sessions = new ArrayList<>();
        Session current;
    }

    private static String dataFile = System.getProperty("user.home") + "/.timetracker.json";

    private static Data loadData() throws IOException {
        Path path = Paths.get(dataFile);
        if (!Files.exists(path))
            return new Data();
        String json = new String(Files.readAllBytes(path));
        Gson gson = new Gson();
        return gson.fromJson(json, Data.class);
    }

    private static void saveData(Data data) throws IOException {
        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        String json = gson.toJson(data);
        Files.write(Paths.get(dataFile), json.getBytes());
    }

    private static String nowISO() {
        return LocalDateTime.now().format(DateTimeFormatter.ISO_LOCAL_DATE_TIME);
    }

    private static long secondsSince(String iso) {
        LocalDateTime dt = LocalDateTime.parse(iso, DateTimeFormatter.ISO_LOCAL_DATE_TIME);
        return Duration.between(dt, LocalDateTime.now()).getSeconds();
    }

    private static String formatDuration(long seconds) {
        long h = seconds / 3600;
        long m = (seconds % 3600) / 60;
        long s = seconds % 60;
        return String.format("%02d:%02d:%02d", h, m, s);
    }

    private static String formatTime(String iso) {
        LocalDateTime dt = LocalDateTime.parse(iso, DateTimeFormatter.ISO_LOCAL_DATE_TIME);
        return dt.format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"));
    }

    public static void main(String[] args) throws IOException {
        if (args.length < 1) {
            System.out.println(colorize("Usage: java tracker <start|stop|status|log>", YELLOW));
            return;
        }
        String cmd = args[0].toLowerCase();
        Data data = loadData();

        switch (cmd) {
            case "start": {
                if (data.current != null) {
                    String start = data.current.start;
                    String end = nowISO();
                    long dur = secondsSince(start);
                    Session s = new Session();
                    s.start = start;
                    s.end = end;
                    s.duration = (int) dur;
                    data.sessions.add(s);
                    System.out.println(colorize("Previous session auto-stopped (duration: " + formatDuration(dur) + ")", YELLOW));
                }
                data.current = new Session();
                data.current.start = nowISO();
                saveData(data);
                System.out.println(colorize("Session started.", GREEN));
                break;
            }
            case "stop": {
                if (data.current == null) {
                    System.out.println(colorize("No active session.", RED));
                    return;
                }
                String start = data.current.start;
                String end = nowISO();
                long dur = secondsSince(start);
                Session s = new Session();
                s.start = start;
                s.end = end;
                s.duration = (int) dur;
                data.sessions.add(s);
                data.current = null;
                saveData(data);
                System.out.println(colorize("Session stopped. Duration: " + formatDuration(dur), GREEN));
                break;
            }
            case "status": {
                if (data.current == null) {
                    System.out.println(colorize("No active session.", YELLOW));
                } else {
                    long dur = secondsSince(data.current.start);
                    System.out.println(colorize("Active session since: " + formatTime(data.current.start), BLUE));
                    System.out.println(colorize("Elapsed: " + formatDuration(dur), GREEN));
                }
                break;
            }
            case "log": {
                if (data.sessions.isEmpty()) {
                    System.out.println(colorize("No sessions in history.", YELLOW));
                } else {
                    System.out.println(colorize("Session history:", BLUE));
                    for (int i = 0; i < data.sessions.size(); i++) {
                        Session s = data.sessions.get(i);
                        String start = formatTime(s.start);
                        String end = formatTime(s.end);
                        String dur = formatDuration(s.duration);
                        System.out.printf("%2d. %s -> %s  (%s)%n", i+1, start, end, dur);
                    }
                }
                break;
            }
            default:
                System.out.println(colorize("Unknown command: " + cmd, RED));
                System.out.println("Available: start, stop, status, log");
        }
    }
}
