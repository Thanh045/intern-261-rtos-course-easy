"""Do thoi gian giua thong bao DEADLOCK va BOOT qua UART STM32.

Cai thu vien: py -m pip install pyserial
Chay: py do_watchdog.py --port COM3 --baud 115200
Dung bang Ctrl+C.
"""
import argparse
import time

import serial


def main():
    bo_tham_so = argparse.ArgumentParser(description="Do DEADLOCK -> BOOT qua UART")
    bo_tham_so.add_argument("--port", default="COM3", help="Cong serial (mac dinh COM3)")
    bo_tham_so.add_argument("--baud", type=int, default=115200, help="Baud rate")
    tham_so = bo_tham_so.parse_args()

    print(f"Dang nghe {tham_so.port} @ {tham_so.baud} baud. Nhan Ctrl+C de dung.", flush=True)
    thoi_diem_deadlock = None
    so_lan_do = 0
    try:
        with serial.Serial(tham_so.port, tham_so.baud, timeout=1) as cong_uart:
            while True:
                du_lieu = cong_uart.readline()
                if not du_lieu:
                    continue
                dong = du_lieu.decode("utf-8", errors="replace").strip()
                thoi_diem = time.perf_counter()
                if dong == "DEADLOCK":
                    thoi_diem_deadlock = thoi_diem
                    print(f"{thoi_diem:.3f} | DEADLOCK", flush=True)
                elif dong == "BOOT":
                    print(f"{thoi_diem:.3f} | BOOT", flush=True)
                    if thoi_diem_deadlock is not None:
                        so_lan_do += 1
                        khoang_ms = (thoi_diem - thoi_diem_deadlock) * 1000.0
                        print(f"  Lan do {so_lan_do}: DEADLOCK -> BOOT = {khoang_ms:.1f} ms", flush=True)
                        thoi_diem_deadlock = None
                # Bo qua cac dong UART khac
    except KeyboardInterrupt:
        print("\nDa dung do.")
    except serial.SerialException as loi:
        print(f"Loi cong UART: {loi}")
        raise SystemExit(1)


if __name__ == "__main__":
    main()
