// tracker.cs
using System;
using System.IO;
using System.Text.Json;
using System.Collections.Generic;

class Tracker
{
    static string Colorize(string text, string color)
    {
        string col = color switch
        {
            "green" => "\x1b[92m",
            "red" => "\x1b[91m",
            "yellow" => "\x1b[93m",
            "blue" => "\x1b[94m",
            _ => "\x1b[0m"
        };
        return col + text + "\x1b[0m";
    }

    class Session
    {
        public string Start { get; set; }
        public string End { get; set; }
        public int Duration { get; set; }
    }

    class Data
    {
        public List<Session> Sessions { get; set; } = new List<Session>();
        public Session Current { get; set; }
    }

    static string DataFile => Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), ".timetracker.json");

    static Data LoadData()
    {
        if (!File.Exists(DataFile))
            return new Data();
        string json = File.ReadAllText(DataFile);
        return JsonSerializer.Deserialize<Data>(json) ?? new Data();
    }

    static void SaveData(Data data)
    {
        string json = JsonSerializer.Serialize(data, new JsonSerializerOptions { WriteIndented = true });
        File.WriteAllText(DataFile, json);
    }

    static string NowISO() => DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ss");

    static int SecondsSince(string iso) => (int)(DateTime.Now - DateTime.Parse(iso)).TotalSeconds;

    static string FormatDuration(int seconds)
    {
        var ts = TimeSpan.FromSeconds(seconds);
        return $"{ts.Hours:D2}:{ts.Minutes:D2}:{ts.Seconds:D2}";
    }

    static string FormatTime(string iso) => DateTime.Parse(iso).ToString("yyyy-MM-dd HH:mm:ss");

    static void Main(string[] args)
    {
        if (args.Length < 1)
        {
            Console.WriteLine(Colorize("Usage: tracker <start|stop|status|log>", "yellow"));
            return;
        }
        string cmd = args[0].ToLower();
        Data data = LoadData();

        switch (cmd)
        {
            case "start":
                if (data.Current != null)
                {
                    string start = data.Current.Start;
                    string end = NowISO();
                    int dur = SecondsSince(start);
                    data.Sessions.Add(new Session { Start = start, End = end, Duration = dur });
                    Console.WriteLine(Colorize($"Previous session auto-stopped (duration: {FormatDuration(dur)})", "yellow"));
                }
                data.Current = new Session { Start = NowISO() };
                SaveData(data);
                Console.WriteLine(Colorize("Session started.", "green"));
                break;

            case "stop":
                if (data.Current == null)
                {
                    Console.WriteLine(Colorize("No active session.", "red"));
                    return;
                }
                string start2 = data.Current.Start;
                string end2 = NowISO();
                int dur2 = SecondsSince(start2);
                data.Sessions.Add(new Session { Start = start2, End = end2, Duration = dur2 });
                data.Current = null;
                SaveData(data);
                Console.WriteLine(Colorize($"Session stopped. Duration: {FormatDuration(dur2)}", "green"));
                break;

            case "status":
                if (data.Current == null)
                    Console.WriteLine(Colorize("No active session.", "yellow"));
                else
                {
                    int dur = SecondsSince(data.Current.Start);
                    Console.WriteLine(Colorize($"Active session since: {FormatTime(data.Current.Start)}", "blue"));
                    Console.WriteLine(Colorize($"Elapsed: {FormatDuration(dur)}", "green"));
                }
                break;

            case "log":
                if (data.Sessions.Count == 0)
                    Console.WriteLine(Colorize("No sessions in history.", "yellow"));
                else
                {
                    Console.WriteLine(Colorize("Session history:", "blue"));
                    for (int i = 0; i < data.Sessions.Count; i++)
                    {
                        var s = data.Sessions[i];
                        string start = FormatTime(s.Start);
                        string end = FormatTime(s.End);
                        string dur = FormatDuration(s.Duration);
                        Console.WriteLine($"{i+1,2}. {start} -> {end}  ({dur})");
                    }
                }
                break;

            default:
                Console.WriteLine(Colorize($"Unknown command: {cmd}", "red"));
                Console.WriteLine("Available: start, stop, status, log");
                break;
        }
    }
}
