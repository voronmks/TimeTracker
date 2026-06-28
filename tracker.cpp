// tracker.cpp
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <json/json.h> // sudo apt-get install libjsoncpp-dev

using namespace std;

const string RESET = "\033[0m";
const string GREEN = "\033[92m";
const string RED = "\033[91m";
const string YELLOW = "\033[93m";
const string BLUE = "\033[94m";

string colorize(const string& text, const string& color) {
    return color + text + RESET;
}

string getDataFile() {
    const char* home = getenv("HOME");
    if (!home) home = getenv("USERPROFILE");
    return string(home) + "/.timetracker.json";
}

string nowISO() {
    auto now = chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", localtime(&tt));
    return string(buf);
}

int secondsSince(const string& iso) {
    tm tm = {};
    stringstream ss(iso);
    ss >> get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    time_t t = mktime(&tm);
    return (int)difftime(time(nullptr), t);
}

string formatDuration(int seconds) {
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
    return string(buf);
}

string formatTime(const string& iso) {
    tm tm = {};
    stringstream ss(iso);
    ss >> get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return string(buf);
}

Json::Value loadData() {
    ifstream f(getDataFile());
    if (!f) {
        Json::Value root;
        root["sessions"] = Json::arrayValue;
        root["current"] = Json::nullValue;
        return root;
    }
    Json::Value root;
    f >> root;
    return root;
}

void saveData(const Json::Value& root) {
    ofstream f(getDataFile());
    f << root.toStyledString();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << colorize("Usage: tracker <start|stop|status|log>", YELLOW) << endl;
        return 1;
    }
    string cmd = argv[1];
    Json::Value data = loadData();

    if (cmd == "start") {
        if (!data["current"].isNull()) {
            string start = data["current"]["start"].asString();
            string end = nowISO();
            int dur = secondsSince(start);
            Json::Value session;
            session["start"] = start;
            session["end"] = end;
            session["duration"] = dur;
            data["sessions"].append(session);
            cout << colorize("Previous session auto-stopped (duration: " + formatDuration(dur) + ")", YELLOW) << endl;
        }
        data["current"] = Json::Value(Json::objectValue);
        data["current"]["start"] = nowISO();
        saveData(data);
        cout << colorize("Session started.", GREEN) << endl;
    }
    else if (cmd == "stop") {
        if (data["current"].isNull()) {
            cout << colorize("No active session.", RED) << endl;
            return 1;
        }
        string start = data["current"]["start"].asString();
        string end = nowISO();
        int dur = secondsSince(start);
        Json::Value session;
        session["start"] = start;
        session["end"] = end;
        session["duration"] = dur;
        data["sessions"].append(session);
        data["current"] = Json::nullValue;
        saveData(data);
        cout << colorize("Session stopped. Duration: " + formatDuration(dur), GREEN) << endl;
    }
    else if (cmd == "status") {
        if (data["current"].isNull()) {
            cout << colorize("No active session.", YELLOW) << endl;
        } else {
            string start = data["current"]["start"].asString();
            int dur = secondsSince(start);
            cout << colorize("Active session since: " + formatTime(start), BLUE) << endl;
            cout << colorize("Elapsed: " + formatDuration(dur), GREEN) << endl;
        }
    }
    else if (cmd == "log") {
        if (data["sessions"].size() == 0) {
            cout << colorize("No sessions in history.", YELLOW) << endl;
        } else {
            cout << colorize("Session history:", BLUE) << endl;
            for (unsigned int i = 0; i < data["sessions"].size(); ++i) {
                Json::Value s = data["sessions"][i];
                string start = formatTime(s["start"].asString());
                string end = formatTime(s["end"].asString());
                string dur = formatDuration(s["duration"].asInt());
                cout << setw(2) << (i+1) << ". " << start << " -> " << end << "  (" << dur << ")" << endl;
            }
        }
    }
    else {
        cout << colorize("Unknown command: " + cmd, RED) << endl;
        cout << "Available: start, stop, status, log" << endl;
    }
    return 0;
}
