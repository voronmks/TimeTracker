# tracker.py
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import json
import time
from datetime import datetime
from pathlib import Path

# ANSI colors
COLORS = {
    'green': '\033[92m',
    'red': '\033[91m',
    'yellow': '\033[93m',
    'blue': '\033[94m',
    'reset': '\033[0m'
}

def colorize(text, color):
    return f"{COLORS.get(color, '')}{text}{COLORS['reset']}"

DATA_FILE = Path.home() / '.timetracker.json'

def load_data():
    if not DATA_FILE.exists():
        return {'sessions': [], 'current': None}
    with open(DATA_FILE, 'r') as f:
        return json.load(f)

def save_data(data):
    with open(DATA_FILE, 'w') as f:
        json.dump(data, f, indent=2)

def format_duration(seconds):
    hours = seconds // 3600
    minutes = (seconds % 3600) // 60
    secs = seconds % 60
    return f"{hours:02d}:{minutes:02d}:{secs:02d}"

def format_time(iso_str):
    dt = datetime.fromisoformat(iso_str)
    return dt.strftime('%Y-%m-%d %H:%M:%S')

def main():
    if len(sys.argv) < 2:
        print(colorize("Usage: tracker.py <start|stop|status|log>", 'yellow'))
        sys.exit(1)

    cmd = sys.argv[1].lower()
    data = load_data()

    if cmd == 'start':
        if data['current'] is not None:
            # Автоматически завершаем предыдущую сессию
            old_start = data['current']['start']
            now = datetime.now().isoformat()
            duration = int((datetime.now() - datetime.fromisoformat(old_start)).total_seconds())
            data['sessions'].append({
                'start': old_start,
                'end': now,
                'duration': duration
            })
            print(colorize(f"Предыдущая сессия автоматически завершена (длительность: {format_duration(duration)})", 'yellow'))
        data['current'] = {'start': datetime.now().isoformat()}
        save_data(data)
        print(colorize("Сессия начата.", 'green'))

    elif cmd == 'stop':
        if data['current'] is None:
            print(colorize("Нет активной сессии.", 'red'))
            sys.exit(1)
        start = data['current']['start']
        now = datetime.now().isoformat()
        duration = int((datetime.now() - datetime.fromisoformat(start)).total_seconds())
        data['sessions'].append({
            'start': start,
            'end': now,
            'duration': duration
        })
        data['current'] = None
        save_data(data)
        print(colorize(f"Сессия завершена. Длительность: {format_duration(duration)}", 'green'))

    elif cmd == 'status':
        if data['current'] is None:
            print(colorize("Нет активной сессии.", 'yellow'))
        else:
            start = datetime.fromisoformat(data['current']['start'])
            now = datetime.now()
            duration = int((now - start).total_seconds())
            print(colorize(f"Активная сессия с {format_time(data['current']['start'])}", 'blue'))
            print(colorize(f"Прошло: {format_duration(duration)}", 'green'))

    elif cmd == 'log':
        if not data['sessions']:
            print(colorize("История пуста.", 'yellow'))
        else:
            print(colorize("История сессий:", 'blue'))
            for i, s in enumerate(data['sessions'], 1):
                start = format_time(s['start'])
                end = format_time(s['end'])
                dur = format_duration(s['duration'])
                print(f"{i:2}. {start} -> {end}  ({dur})")

    else:
        print(colorize(f"Неизвестная команда: {cmd}", 'red'))
        print("Доступные команды: start, stop, status, log")

if __name__ == '__main__':
    main()
