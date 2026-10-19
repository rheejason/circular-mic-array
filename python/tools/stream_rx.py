"""Receive the rig's USB audio stream (STM32 Virtual ComPort).

    python python/tools/stream_rx.py rate              # byte rate for 5 s
    python python/tools/stream_rx.py check -s 10       # packet framing and seq gaps for 10 s
    python python/tools/stream_rx.py record -s 10 -o snap_0deg   # save .npy + .wav to python/captures/
"""
import argparse
import struct
import sys
import time
import wave
from datetime import datetime
from pathlib import Path

import numpy as np
import serial
from serial.tools import list_ports

# Must match the firmware (array_config.h, stream.c)
NUM_MICS = 4
BLOCK_SAMPLES = 48
FS = 48000
MAGIC = b"MIC4"
HEADER = struct.Struct("<4sI")                                  # magic, seq
PACKET_BYTES = HEADER.size + BLOCK_SAMPLES * NUM_MICS * 2       # 392

WARMUP_PACKETS = 50     # skip blocks that sat in the board's ring while the port was closed
NO_DATA_TIMEOUT = 2.0   # seconds without any bytes before giving up
CAPTURES = Path(__file__).resolve().parents[1] / "captures"


def find_port():
    for p in list_ports.comports():
        if p.vid == 0x0483 and p.pid == 0x5740:
            return p.device
    sys.exit("STM32 Virtual ComPort not found. Is the CN13 (user USB) cable plugged in?")


def open_port(port):
    ser = serial.Serial(port, timeout=0.1)      # baud rate is ignored for USB CDC
    ser.reset_input_buffer()
    return ser


def read_some(ser, last_data):
    data = ser.read(ser.in_waiting or 1)
    if data:
        return data, time.monotonic()
    if time.monotonic() - last_data > NO_DATA_TIMEOUT:
        sys.exit("No data from the board. Is the streaming firmware flashed and running?")
    return data, last_data


def packets(ser, stats):
    """Yield (seq, pcm[48, 4]) per packet. Resyncs on MAGIC if the byte stream gets out of step."""
    buf = bytearray()
    last_data = time.monotonic()
    while True:
        data, last_data = read_some(ser, last_data)
        buf += data
        while True:
            i = buf.find(MAGIC)
            if i < 0:
                stats["skipped_bytes"] += max(len(buf) - 3, 0)
                del buf[:-3]                    # keep a possible partial MAGIC
                break
            if len(buf) - i < PACKET_BYTES:
                stats["skipped_bytes"] += i
                del buf[:i]
                break
            stats["skipped_bytes"] += i
            pkt = bytes(buf[i:i + PACKET_BYTES])
            del buf[:i + PACKET_BYTES]
            _, seq = HEADER.unpack_from(pkt)
            pcm = np.frombuffer(pkt, dtype="<i2", offset=HEADER.size).reshape(BLOCK_SAMPLES, NUM_MICS)
            yield seq, pcm


def receive(ser, seconds):
    """Collect `seconds` of audio. Missing blocks are filled with zeros so timing stays correct."""
    want = int(round(seconds * 1000))
    stats = {"skipped_bytes": 0, "gaps": 0, "missing": 0}
    blocks = []
    last = None
    for k, (seq, pcm) in enumerate(packets(ser, stats)):
        if k == WARMUP_PACKETS:
            stats["skipped_bytes"] = 0          # partial packet at port open is expected
        if k >= WARMUP_PACKETS:
            lost = (seq - last - 1) & 0xFFFFFFFF
            if lost:
                stats["gaps"] += 1
                stats["missing"] += lost
                blocks.extend([np.zeros_like(pcm)] * min(lost, want - len(blocks)))
            blocks.append(pcm)
            if len(blocks) >= want:
                break
        last = seq
    return np.concatenate(blocks[:want]), stats


def print_stats(stats, seconds):
    print(f"packets: {int(seconds * 1000)}   seq gaps: {stats['gaps']} ({stats['missing']} blocks missing)"
          f"   resync bytes: {stats['skipped_bytes']}")


def print_levels(audio):
    for m in range(NUM_MICS):
        ch = audio[:, m].astype(float)
        print(f"  mic {m}: rms {ch.std():8.1f}   peak {np.abs(ch).max():6.0f}")


def write_wav(path, samples):
    samples = np.ascontiguousarray(samples, dtype="<i2")
    with wave.open(str(path), "wb") as w:
        w.setnchannels(samples.shape[1] if samples.ndim == 2 else 1)
        w.setsampwidth(2)
        w.setframerate(FS)
        w.writeframes(samples.tobytes())


def cmd_rate(ser, seconds):
    time.sleep(0.2)
    ser.reset_input_buffer()
    n = 0
    t0 = time.monotonic()
    last_data = t0
    while (t := time.monotonic() - t0) < seconds:
        data, last_data = read_some(ser, last_data)
        n += len(data)
    print(f"{n:,} bytes in {t:.2f} s = {n / t:,.0f} bytes/s (expected {PACKET_BYTES * 1000:,})")


def cmd_check(ser, seconds):
    audio, stats = receive(ser, seconds)
    print_stats(stats, seconds)
    print_levels(audio)
    ok = stats["gaps"] == 0 and stats["skipped_bytes"] == 0
    print("PASS" if ok else "FAIL")


def cmd_record(ser, seconds, name):
    name = name or datetime.now().strftime("stream_%Y%m%d_%H%M%S")
    print(f"recording {seconds} s ...")
    audio, stats = receive(ser, seconds)
    print_stats(stats, seconds)
    print_levels(audio)

    CAPTURES.mkdir(parents=True, exist_ok=True)
    np.save(CAPTURES / f"{name}.npy", audio)
    write_wav(CAPTURES / f"{name}_{NUM_MICS}ch.wav", audio)
    for m in range(NUM_MICS):
        write_wav(CAPTURES / f"{name}_mic{m}.wav", audio[:, m])
    print(f"saved {CAPTURES / name}.npy and .wav files")


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("test", choices=["rate", "check", "record"])
    p.add_argument("-s", "--seconds", type=float, default=5.0)
    p.add_argument("-o", "--out", help="file name for record (default: stream_<time>)")
    p.add_argument("-p", "--port", help="serial port (default: find the STM32 automatically)")
    args = p.parse_args()

    with open_port(args.port or find_port()) as ser:
        if args.test == "rate":
            cmd_rate(ser, args.seconds)
        elif args.test == "check":
            cmd_check(ser, args.seconds)
        else:
            cmd_record(ser, args.seconds, args.out)


if __name__ == "__main__":
    main()
