# GuardianRX — Google Sheets Schema

Expected columns (left → right):

1. ts_iso       (ISO8601 timestamp, local tz offset included)
2. deviceId     (string, e.g., "rx-001")
3. container    (integer, pillbox container id, 1–4)
4. event        (string, e.g., reminder_fired, taken_ack, late, missed)
5. med_name     (string, "Metformin 500mg")
6. dosage       (string, "1 tab after meal")
7. due_hhmm     (string, HH:MM of scheduled time)
8. result       (string, TAKEN/LATE/MISSED/INFO)
9. rssi         (integer, optional Wi-Fi signal strength)

> note: sheet header row must match these exactly for apps script to append correctly.
