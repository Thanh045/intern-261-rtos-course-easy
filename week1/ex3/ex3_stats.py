ex3_stats.py
import csv
import sys

SAMPLE_COUNT = 800
PERIOD_MS = 13

if len(sys.argv) != 2:
    sys.exit("Cach chay: python tools/ex3_stats.py duong_dan.csv")

csv_path = sys.argv[1]

with open(csv_path, encoding="utf-8-sig", newline="") as file:
    # Bo qua dong trong va dong chu thich.
    lines = [
        line for line in file
        if line.strip() and not line.lstrip().startswith(";")
        and not line.lstrip().startswith("#")
    ]

reader = csv.reader(lines)
rows = list(reader)

# Tim dong tieu de co cot thoi gian va D0.
header_index = None

for index, row in enumerate(rows):
    names = [cell.strip().lower() for cell in row]

    time_columns = [
        i for i, name in enumerate(names)
        if name in ("time", "time [s]", "time (s)", "time[s]")
    ]

    if time_columns and "d0" in names:
        header_index = index
        time_column = time_columns[0]
        signal_column = names.index("d0")
        break

if header_index is None:
    sys.exit(
        "Khong tim thay cot Time va D0. "
        "Gui vai dong dau CSV de kiem tra dinh dang."
    )

edges = []
previous_level = None
previous_time = None

for row in rows[header_index + 1:]:
    if not row or not any(cell.strip() for cell in row):
        continue

    try:
        time_s = float(row[time_column].strip())
        level = int(row[signal_column].strip())
    except (ValueError, IndexError):
        sys.exit(f"Dong CSV khong hop le: {row}")

    if level not in (0, 1):
        sys.exit("Cot D0 phai chi chua gia tri 0 hoac 1.")

    if previous_time is not None and time_s < previous_time:
        sys.exit("Thoi gian CSV khong tang dan.")

    if previous_level is not None and level != previous_level:
        edges.append(time_s)

    previous_level = level
    previous_time = time_s

if len(edges) < SAMPLE_COUNT + 1:
    sys.exit(
        f"Chi co {len(edges)} canh; "
        f"can it nhat {SAMPLE_COUNT + 1} canh."
    )

# Dung 801 canh dau tien de tinh 800 khoang.
selected = edges[:SAMPLE_COUNT + 1]

intervals_ms = [
    (selected[i + 1] - selected[i]) * 1000
    for i in range(SAMPLE_COUNT)
]

minimum = min(intervals_ms)
maximum = max(intervals_ms)
average = sum(intervals_ms) / SAMPLE_COUNT
jitter_us = (maximum - minimum) * 1000

elapsed_ms = (selected[-1] - selected[0]) * 1000
drift_ms = elapsed_ms - SAMPLE_COUNT * PERIOD_MS

print(f"So khoang:        {SAMPLE_COUNT}")
print(f"Min:              {minimum:.6f} ms")
print(f"Max:              {maximum:.6f} ms")
print(f"Trung binh:       {average:.9f} ms")
print(f"Jitter max-min:   {jitter_us:.3f} us")
print(f"Drift vs 13 ms:   {drift_ms:+.6f} ms")