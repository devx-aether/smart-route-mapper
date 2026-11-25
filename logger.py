import serial, sqlite3, json, time

# Replace 'COM3' with your ESP32 port (e.g., COM4 or /dev/cu.usbserial-0001)
PORT = "COM3"
ser = serial.Serial(PORT, 115200, timeout=1)

# Creates a real local SQLite database file
conn = sqlite3.connect("road_bumps.db")
conn.execute("""
    CREATE TABLE IF NOT EXISTS bumps (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        timestamp TEXT,
        impact REAL,
        severity TEXT
    )
""")
conn.commit()
print("Listening to ESP32 and logging to road_bumps.db...")

while True:
    line = ser.readline().decode("utf-8", errors="ignore").strip()
    if line.startswith("[MOCK DB LOG]:"):
        data = json.loads(line.replace("[MOCK DB LOG]:", "").strip())
        ts = time.strftime("%Y-%m-%d %H:%M:%S")
        conn.execute(
            "INSERT INTO bumps (timestamp, impact, severity) VALUES (?, ?, ?)",
            (ts, data["impact"], data["severity"])
        )
        conn.commit()
        print(f"Saved to DB -> {ts} | Impact: {data['impact']} | Severity: {data['severity']}")