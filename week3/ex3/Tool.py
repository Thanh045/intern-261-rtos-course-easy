#!/usr/bin/env python3
"""Week 3 / Ex 3: continuous UART command test for STM32 FreeRTOS.

Requires: python -m pip install pyserial
Example: python uart_stress_test.py --port COM5 --count 500
"""
import argparse
import re
import sys
import threading
import time

try:
    import serial
except ImportError:
    sys.exit("Chua cai pyserial. Chay: python -m pip install pyserial")

ACK_RE = re.compile(rb"^OK (P\d+#(\d+))$")


def main():
    parser = argparse.ArgumentParser(description="Gui lien tuc lenh UART P100#N den STM32 va dem ACK")
    parser.add_argument("--port", required=True, help="Vi du COM5 (Windows) hoac /dev/ttyUSB0 (Linux)")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--count", type=int, default=500, help="So lenh muon gui (mac dinh 500)")
    parser.add_argument("--period", type=int, default=100, help="Chu ky cho lenh P, don vi ms")
    parser.add_argument("--gap-ms", type=float, default=0, help="Khoang nghi giua cac lenh (0 = khong nghi)")
    parser.add_argument("--wait", type=float, default=3, help="So giay cho phan hoi sau khi gui xong")
    args = parser.parse_args()

    if args.count < 1 or args.period < 1 or args.gap_ms < 0 or args.wait < 0:
        parser.error("count va period phai >= 1; gap-ms va wait phai >= 0")
    if len(f"P{args.period}#{args.count}".encode("ascii")) > 15:
        parser.error("Lenh vuot MAX_CMD_LEN=16 (toi da 15 ky tu truoc \\r); giam count/period")

    received = set()
    unexpected = []
    stop = threading.Event()
    lock = threading.Lock()
    sent = 0

    try:
        with serial.Serial(args.port, args.baud, timeout=0.1, write_timeout=3) as ser:
            # Most USB-UART bridges do not reset STM32; allow initial setup and clear stale RX.
            time.sleep(0.3)
            ser.reset_input_buffer()

            def receive_acks():
                buffer = bytearray()
                while not stop.is_set():
                    try:
                        chunk = ser.read(ser.in_waiting or 1)
                    except serial.SerialException:
                        stop.set()
                        return
                    if not chunk:
                        continue
                    buffer.extend(chunk)
                    while b"\n" in buffer:
                        raw_line, _, remainder = buffer.partition(b"\n")
                        buffer = bytearray(remainder)
                        line = raw_line.strip()
                        match = ACK_RE.fullmatch(line)
                        if match and match.group(1) == f"P{args.period}#{int(match.group(2))}".encode("ascii"):
                            seq = int(match.group(2))
                            if 1 <= seq <= args.count:
                                with lock:
                                    received.add(seq)
                        elif line:
                            with lock:
                                if len(unexpected) < 10:
                                    unexpected.append(line.decode("ascii", errors="replace"))
                    if len(buffer) > 256:
                        buffer.clear()

            worker = threading.Thread(target=receive_acks, daemon=True)
            worker.start()
            print(f"Port: {args.port} | Baud: {args.baud} | Lenh: P{args.period}#N | Gap: {args.gap_ms} ms")
            start = time.monotonic()
            try:
                for seq in range(1, args.count + 1):
                    command = f"P{args.period}#{seq}\r".encode("ascii")
                    ser.write(command)
                    sent += 1
                    if args.gap_ms:
                        time.sleep(args.gap_ms / 1000)
            except (serial.SerialException, serial.SerialTimeoutException) as exc:
                print(f"Loi khi gui: {exc}", file=sys.stderr)

            ser.flush()
            elapsed = time.monotonic() - start
            end = time.monotonic() + args.wait
            while time.monotonic() < end and not stop.is_set():
                with lock:
                    complete = len(received) >= sent
                if complete:
                    break
                time.sleep(0.02)
            stop.set()
            worker.join(timeout=0.3)

            with lock:
                valid_ack = sum(1 for seq in received if seq <= sent)
                missing = [seq for seq in range(1, sent + 1) if seq not in received]
                notes = list(unexpected)
            print("\n===== KET QUA =====")
            print(f"So lenh gui thanh cong vao cong serial: {sent}")
            print(f"So lenh nhan duoc phan hoi OK:      {valid_ack}")
            print(f"So lenh khong co ACK:                {sent - valid_ack}")
            print(f"Thoi gian gui:                     {elapsed:.3f} s")
            print(f"Toc do gui trung binh:             {sent / elapsed:.1f} lenh/s" if elapsed > 0 else "")
            print(f"Vi du so thu tu thieu ACK:         {missing[:20]}")
            if notes:
                print(f"Dong phan hoi khac (toi da 10):    {notes}")
            print("Luu y: Khong co ACK KHONG dong nghia chac chan mat byte.")
            print("Kiem tra them bien so_byte_mat, so_lenh_dung, so_lenh_sai, so_lenh_qua_dai trong debugger.")
    except serial.SerialException as exc:
        sys.exit(f"Khong mo duoc cong UART: {exc}")


if __name__ == "__main__":
    main()
