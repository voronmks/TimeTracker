#!/usr/bin/env ruby
# tracker.rb
# encoding: UTF-8

require 'json'
require 'date'
require 'fileutils'

COLORS = {
  green: "\e[92m",
  red: "\e[91m",
  yellow: "\e[93m",
  blue: "\e[94m",
  reset: "\e[0m"
}

def colorize(text, color)
  "#{COLORS[color]}#{text}#{COLORS[:reset]}"
end

DATA_FILE = File.join(Dir.home, '.timetracker.json')

def load_data
  return { 'sessions' => [], 'current' => nil } unless File.exist?(DATA_FILE)
  JSON.parse(File.read(DATA_FILE))
end

def save_data(data)
  File.write(DATA_FILE, JSON.pretty_generate(data))
end

def now_iso
  Time.now.iso8601
end

def seconds_since(iso)
  (Time.now - Time.iso8601(iso)).to_i
end

def format_duration(seconds)
  h = seconds / 3600
  m = (seconds % 3600) / 60
  s = seconds % 60
  format('%02d:%02d:%02d', h, m, s)
end

def format_time(iso)
  Time.iso8601(iso).strftime('%Y-%m-%d %H:%M:%S')
end

def main
  if ARGV.empty?
    puts colorize('Usage: tracker.rb <start|stop|status|log>', :yellow)
    exit 1
  end

  cmd = ARGV[0].downcase
  data = load_data

  case cmd
  when 'start'
    if data['current']
      start = data['current']['start']
      now = now_iso
      dur = seconds_since(start)
      data['sessions'] << { 'start' => start, 'end' => now, 'duration' => dur }
      puts colorize("Previous session auto-stopped (duration: #{format_duration(dur)})", :yellow)
    end
    data['current'] = { 'start' => now_iso }
    save_data(data)
    puts colorize('Session started.', :green)

  when 'stop'
    unless data['current']
      puts colorize('No active session.', :red)
      exit 1
    end
    start = data['current']['start']
    now = now_iso
    dur = seconds_since(start)
    data['sessions'] << { 'start' => start, 'end' => now, 'duration' => dur }
    data['current'] = nil
    save_data(data)
    puts colorize("Session stopped. Duration: #{format_duration(dur)}", :green)

  when 'status'
    if data['current']
      dur = seconds_since(data['current']['start'])
      puts colorize("Active session since: #{format_time(data['current']['start'])}", :blue)
      puts colorize("Elapsed: #{format_duration(dur)}", :green)
    else
      puts colorize('No active session.', :yellow)
    end

  when 'log'
    if data['sessions'].empty?
      puts colorize('No sessions in history.', :yellow)
    else
      puts colorize('Session history:', :blue)
      data['sessions'].each_with_index do |s, i|
        start = format_time(s['start'])
        end_t = format_time(s['end'])
        dur = format_duration(s['duration'])
        puts format("%2d. %s -> %s  (%s)", i+1, start, end_t, dur)
      end
    end

  else
    puts colorize("Unknown command: #{cmd}", :red)
    puts 'Available: start, stop, status, log'
  end
end

main if __FILE__ == $0
