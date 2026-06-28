// tracker.js
#!/usr/bin/env node
'use strict';

const fs = require('fs');
const path = require('path');
const os = require('os');

const COLORS = {
    green: '\x1b[92m',
    red: '\x1b[91m',
    yellow: '\x1b[93m',
    blue: '\x1b[94m',
    reset: '\x1b[0m'
};

function colorize(text, color) {
    return COLORS[color] + text + COLORS.reset;
}

const DATA_FILE = path.join(os.homedir(), '.timetracker.json');

function loadData() {
    if (!fs.existsSync(DATA_FILE)) {
        return { sessions: [], current: null };
    }
    return JSON.parse(fs.readFileSync(DATA_FILE, 'utf8'));
}

function saveData(data) {
    fs.writeFileSync(DATA_FILE, JSON.stringify(data, null, 2));
}

function formatDuration(seconds) {
    const h = Math.floor(seconds / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    const s = seconds % 60;
    return `${String(h).padStart(2,'0')}:${String(m).padStart(2,'0')}:${String(s).padStart(2,'0')}`;
}

function formatTime(iso) {
    const d = new Date(iso);
    return d.toLocaleString('ru-RU', { year: 'numeric', month: '2-digit', day: '2-digit', hour: '2-digit', minute: '2-digit', second: '2-digit' });
}

function main() {
    const args = process.argv.slice(2);
    if (args.length < 1) {
        console.log(colorize('Usage: tracker.js <start|stop|status|log>', 'yellow'));
        process.exit(1);
    }
    const cmd = args[0].toLowerCase();
    const data = loadData();

    switch (cmd) {
        case 'start': {
            if (data.current) {
                // auto-stop
                const start = data.current.start;
                const now = new Date().toISOString();
                const duration = Math.floor((new Date(now) - new Date(start)) / 1000);
                data.sessions.push({ start, end: now, duration });
                console.log(colorize(`Previous session auto-stopped (duration: ${formatDuration(duration)})`, 'yellow'));
            }
            data.current = { start: new Date().toISOString() };
            saveData(data);
            console.log(colorize('Session started.', 'green'));
            break;
        }
        case 'stop': {
            if (!data.current) {
                console.log(colorize('No active session.', 'red'));
                process.exit(1);
            }
            const start = data.current.start;
            const now = new Date().toISOString();
            const duration = Math.floor((new Date(now) - new Date(start)) / 1000);
            data.sessions.push({ start, end: now, duration });
            data.current = null;
            saveData(data);
            console.log(colorize(`Session stopped. Duration: ${formatDuration(duration)}`, 'green'));
            break;
        }
        case 'status': {
            if (!data.current) {
                console.log(colorize('No active session.', 'yellow'));
            } else {
                const start = data.current.start;
                const duration = Math.floor((new Date() - new Date(start)) / 1000);
                console.log(colorize(`Active session since: ${formatTime(start)}`, 'blue'));
                console.log(colorize(`Elapsed: ${formatDuration(duration)}`, 'green'));
            }
            break;
        }
        case 'log': {
            if (data.sessions.length === 0) {
                console.log(colorize('No sessions in history.', 'yellow'));
            } else {
                console.log(colorize('Session history:', 'blue'));
                data.sessions.forEach((s, i) => {
                    const start = formatTime(s.start);
                    const end = formatTime(s.end);
                    const dur = formatDuration(s.duration);
                    console.log(`${String(i+1).padStart(2)}. ${start} -> ${end}  (${dur})`);
                });
            }
            break;
        }
        default:
            console.log(colorize(`Unknown command: ${cmd}`, 'red'));
            console.log('Available: start, stop, status, log');
    }
}

main();
