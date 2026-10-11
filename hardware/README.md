# Hardware

| Folder | What |
|---|---|
| `kicad/` | 8-mic board KiCad project |
| `mechanical/` | Bench rig ring and enclosure: STEP and 3MF exports (keep the Fusion source link in a note here) |
| `bom/` | Bills of materials |

## 8-mic board (planned)

- 8 × TDK InvenSense T5838 (MMICT5838-00-012), bottom port, uniform circle, radius 55 mm (tentative).
  Each mic needs a 0.5–1 mm PCB hole; the hole centre is the mic's acoustic position.
- 2 mics per PDM data line (opposite SELECT), 4 lines into an ADAU7118 → TDM8 → STM32H743 SAI.
- PDM clock 3.072 MHz (÷64 = 48 kHz).
- Still to decide: LCD part (round GC9A01 planned), LED ring, enclosure, which mics share a data line.
- Check in the datasheets before layout: ADAU7118 clocking and PDM clock fan-out, T5838 mode/threshold pins.

## KiCad

Schematic and PCB files don't merge, so only one person edits a sheet or board at a time.
Pull before opening KiCad and push when you're done.
