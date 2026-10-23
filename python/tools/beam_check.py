"""Compare two recordings of the same source: beam pointed at it vs pointed away.

    python python/tools/beam_check.py look0 look180

Per band: how much quieter the beam got when pointed away. Each recording's beam level is
measured relative to the average of its 4 mics, so a source that got a bit louder or quieter
between the two recordings doesn't matter (the mics don't depend on where the beam points).
"""
import sys
from pathlib import Path

import numpy as np

FS = 48000
BLOCK = 48
NUM_MICS = 4
CAPTURES = Path(__file__).resolve().parents[1] / "captures"
BANDS = [(250, 500), (500, 1000), (1000, 1500), (1500, 2500), (2500, 4000)]


def load(name):
    a = np.load(CAPTURES / f"{name}.npy").astype(float)
    blocks = a.reshape(-1, BLOCK, a.shape[1])
    good = ~np.all(blocks[:, :, :NUM_MICS] == 0, axis=(1, 2)) & ~np.all(blocks[:, :, NUM_MICS] == 0, axis=1)
    print(f"{name}: {good.mean() * 100:.1f}% of blocks usable")
    a = blocks[good].reshape(-1, a.shape[1])        # drop USB gaps and beam dropouts
    return a[:, :NUM_MICS].mean(axis=1), a[:, NUM_MICS]


def band_db(x, lo, hi):
    X = np.fft.rfft(x)
    f = np.fft.rfftfreq(len(x), 1 / FS)
    return 10 * np.log10(np.sum(np.abs(X[(f >= lo) & (f < hi)]) ** 2) + 1e-20)


def beam_vs_mics(name):
    mic, beam = load(name)
    return [band_db(beam, lo, hi) - band_db(mic, lo, hi) for lo, hi in BANDS]


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    toward, away = beam_vs_mics(sys.argv[1]), beam_vs_mics(sys.argv[2])
    print(f"\n{'band (Hz)':>12}  {'toward':>7}  {'away':>7}  {'rejection':>9}")
    for (lo, hi), t, a in zip(BANDS, toward, away):
        print(f"{lo:>5}-{hi:<6}  {t:+6.1f}  {a:+6.1f}  {a - t:+8.1f} dB")
    print("\ntoward/away = beam level minus mic-average level. Toward should be ~0 dB;"
          "\nrejection should be negative, largest around 1.5-2.5 kHz.")


if __name__ == "__main__":
    main()
