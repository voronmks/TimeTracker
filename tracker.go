// tracker.go
package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"time"
)

const (
	reset  = "\033[0m"
	green  = "\033[92m"
	red    = "\033[91m"
	yellow = "\033[93m"
	blue   = "\033[94m"
)

func colorize(text, color string) string {
	return color + text + reset
}

type Session struct {
	Start    string `json:"start"`
	End      string `json:"end,omitempty"`
	Duration int    `json:"duration,omitempty"`
}

type Data struct {
	Sessions []Session `json:"sessions"`
	Current  *Session  `json:"current"`
}

func getDataFile() string {
	home, _ := os.UserHomeDir()
	return filepath.Join(home, ".timetracker.json")
}

func loadData() (*Data, error) {
	path := getDataFile()
	if _, err := os.Stat(path); os.IsNotExist(err) {
		return &Data{Sessions: []Session{}, Current: nil}, nil
	}
	bytes, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var data Data
	if err := json.Unmarshal(bytes, &data); err != nil {
		return nil, err
	}
	return &data, nil
}

func saveData(data *Data) error {
	bytes, err := json.MarshalIndent(data, "", "  ")
	if err != nil {
		return err
	}
	return ioutil.WriteFile(getDataFile(), bytes, 0644)
}

func formatDuration(seconds int) string {
	h := seconds / 3600
	m := (seconds % 3600) / 60
	s := seconds % 60
	return fmt.Sprintf("%02d:%02d:%02d", h, m, s)
}

func formatTime(iso string) string {
	t, _ := time.Parse(time.RFC3339, iso)
	return t.Format("2006-01-02 15:04:05")
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println(colorize("Usage: tracker <start|stop|status|log>", yellow))
		os.Exit(1)
	}
	cmd := os.Args[1]
	data, err := loadData()
	if err != nil {
		fmt.Println(colorize("Error loading data: "+err.Error(), red))
		os.Exit(1)
	}

	switch cmd {
	case "start":
		if data.Current != nil {
			// auto-stop previous
			start := data.Current.Start
			now := time.Now().Format(time.RFC3339)
			startTime, _ := time.Parse(time.RFC3339, start)
			duration := int(time.Since(startTime).Seconds())
			data.Sessions = append(data.Sessions, Session{
				Start:    start,
				End:      now,
				Duration: duration,
			})
			fmt.Println(colorize(f"Previous session auto-stopped (duration: {formatDuration(duration)})", yellow))
		}
		data.Current = &Session{Start: time.Now().Format(time.RFC3339)}
		if err := saveData(data); err != nil {
			fmt.Println(colorize("Error saving: "+err.Error(), red))
			os.Exit(1)
		}
		fmt.Println(colorize("Session started.", green))

	case "stop":
		if data.Current == nil {
			fmt.Println(colorize("No active session.", red))
			os.Exit(1)
		}
		start := data.Current.Start
		now := time.Now().Format(time.RFC3339)
		startTime, _ := time.Parse(time.RFC3339, start)
		duration := int(time.Since(startTime).Seconds())
		data.Sessions = append(data.Sessions, Session{
			Start:    start,
			End:      now,
			Duration: duration,
		})
		data.Current = nil
		if err := saveData(data); err != nil {
			fmt.Println(colorize("Error saving: "+err.Error(), red))
			os.Exit(1)
		}
		fmt.Println(colorize(f"Session stopped. Duration: {formatDuration(duration)}", green))

	case "status":
		if data.Current == nil {
			fmt.Println(colorize("No active session.", yellow))
		} else {
			startTime, _ := time.Parse(time.RFC3339, data.Current.Start)
			duration := int(time.Since(startTime).Seconds())
			fmt.Println(colorize("Active session since: "+formatTime(data.Current.Start), blue))
			fmt.Println(colorize("Elapsed: "+formatDuration(duration), green))
		}

	case "log":
		if len(data.Sessions) == 0 {
			fmt.Println(colorize("No sessions in history.", yellow))
		} else {
			fmt.Println(colorize("Session history:", blue))
			for i, s := range data.Sessions {
				start := formatTime(s.Start)
				end := formatTime(s.End)
				dur := formatDuration(s.Duration)
				fmt.Printf("%2d. %s -> %s  (%s)\n", i+1, start, end, dur)
			}
		}

	default:
		fmt.Println(colorize("Unknown command: "+cmd, red))
		fmt.Println("Available: start, stop, status, log")
	}
}
