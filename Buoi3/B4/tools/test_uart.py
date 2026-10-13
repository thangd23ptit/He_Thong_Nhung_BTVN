#!/usr/bin/env python3
"""Kiem tra du lieu ADC tu STM32: dem so mau/giay, min/max, do loi dinh dang.
Dung:  python3 tools/test_uart.py /dev/ttyUSB0
Can:   pip install pyserial
"""
import sys, time, serial

port = sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyUSB0"
ser = serial.Serial(port, 115200, timeout=0.1)
print(f"Dang doc {port} @115200 ... Ctrl+C de dung")

buf = b""
samples = []          # (thoi_gian, gia_tri)
bad = 0
t_report = time.time()
try:
    while True:
        buf += ser.read(512)
        while b"\n\r" in buf:
            line, buf = buf.split(b"\n\r", 1)
            try:
                v = int(line)
                if not 0 <= v <= 4095:
                    raise ValueError
                samples.append((time.time(), v))
            except ValueError:
                bad += 1
        if time.time() - t_report >= 1.0:
            now = time.time()
            recent = [v for t, v in samples if now - t <= 1.0]
            if recent:
                print(f"mau/giay ~ {len(recent):3d} | min={min(recent):4d} "
                      f"max={max(recent):4d} | last={recent[-1]:4d} "
                      f"({recent[-1]*3.3/4095:.2f}V) | loi={bad}")
            t_report = now
            samples = [(t, v) for t, v in samples if now - t <= 1.0]
except KeyboardInterrupt:
    pass
