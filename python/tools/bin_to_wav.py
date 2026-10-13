"""Convert a raw rig capture into WAV files you can play. Standard library only (no installs).

The capture is rec_buf dumped from the debugger: interleaved int16, little-endian,
m0 m1 m2 m3 m0 m1 ... at 48 kHz.

usage:  python3 bin_to_wav.py rec.bin [--mics 4] [--fs 48000] [--gain-db 0]

Writes next to the input:
    rec_4ch.wav               all mics in one file (most players only play the first 2 channels)
    rec_mic0.wav ... mic3.wav one mono file per mic, for listening to each mic on its own
and prints the peak and RMS level of each mic.
"""
import argparse
import array
import math
import sys
import wave
from pathlib import Path


def write_wav(path, samples, channels, fs):
    with wave.open(str(path), "wb") as w:
        w.setnchannels(channels)
        w.setsampwidth(2)
        w.setframerate(fs)
        w.writeframes(samples.tobytes())


def apply_gain(samples, gain_db):
    g = 10 ** (gain_db / 20)
    return array.array("h", (max(-32768, min(32767, round(s * g))) for s in samples))


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("bin", type=Path)
    p.add_argument("--mics", type=int, default=4)
    p.add_argument("--fs", type=int, default=48000)
    p.add_argument("--gain-db", type=float, default=0.0, help="boost for listening (clips at full scale)")
    args = p.parse_args()

    if sys.byteorder != "little":
        sys.exit("expects a little-endian host")

    data = array.array("h")
    data.frombytes(args.bin.read_bytes())
    if len(data) % args.mics:
        sys.exit(f"{len(data)} samples is not a multiple of {args.mics} mics; is the dump the right size?")

    n = len(data) // args.mics
    print(f"{args.bin.name}: {n} samples per mic = {n / args.fs:.3f} s")

    stem = args.bin.with_suffix("")
    out = apply_gain(data, args.gain_db) if args.gain_db else data
    write_wav(f"{stem}_{args.mics}ch.wav", out, args.mics, args.fs)

    for m in range(args.mics):
        ch = data[m::args.mics]
        peak = max(abs(s) for s in ch)
        rms = math.sqrt(sum(s * s for s in ch) / n)
        print(f"  mic {m}: peak {peak:6d}   rms {rms:8.1f}")
        write_wav(f"{stem}_mic{m}.wav", out[m::args.mics], 1, args.fs)


if __name__ == "__main__":
    main()
