# Bench prototype: NUCLEO-H753ZI + 4 PDM mics

4 × Adafruit PDM breakout 3492 (ST MP34DT01-M) on a 3D-printed ring, 45 mm radius to each sound hole.
Mic 0 at 0° (+x), then 90°, 180°, 270° counter-clockwise seen from above.

## Wiring

Five wires from the Nucleo to a breadboard hub, then jumpers from the hub to the mics.
The Nucleo stays off the breadboard (its double-row headers would short).

| Nucleo pin | Header | Hub row | Goes to |
|---|---|---|---|
| 3V3 | CN8 | + rail | 3V on all 4 mics; SEL on mics 1 and 3 |
| GND | CN8 | − rail | GND on all 4 mics; SEL on mics 0 and 2 |
| PE2 = SAI1_CK1 | CN9 | CLK | CLK on all 4 mics (3.072 MHz) |
| PE6 = SAI1_D1 | CN9 | DATA A | DAT on mic 0 (left) and mic 1 (right) |
| PE4 = SAI1_D2 | CN9 | DATA B | DAT on mic 2 (left) and mic 3 (right) |

- The two mics on one data line need opposite SEL, or both drive the wire at once.
- Keep the CLK path under ~20 cm, with GND next to it.
- Confirm the pins in CubeMX and their CN8/CN9 positions in ST UM2407 before plugging in.

## CubeMX outline

- New project from the board selector: NUCLEO-H753ZI.
- SAI1 kernel clock 49.152 MHz (PLL2P or PLL3P, FRACN on); SAI bit clock 12.288 MHz = 4 × 3.072 MHz PDM clock.
- SAI1 block A: PDM, mic pairs 1–2, CK1, master receive, free protocol, 32-bit frame, all mic slots active.
- DMA SAI1_A circular, 2 ms buffer (3072 bytes). Put the buffer in AXI SRAM or RAM_D2, **not DTCM**.
- PDM2PCM middleware (needs CRC on): one instance per mic, decimation 64, 48 samples per call.
- Virtual COM port USART3 (PD8/PD9), 921600 baud, for capture dumps.
- Cross-check SAI PDM settings with ST AN5027.

## Bring-up order

1. Multimeter: no 3V–GND short on any mic or on the hub.
2. Scope: 3.072 MHz on CLK; DATA A/B toggle and change when you tap near a mic.
3. DMA buffer is not all zeros.
4. ~1 s capture dumped to the PC as interleaved int16, channel order mic 0, 1, 2, 3.
5. Clap test from ≥ 0.5 m at a known angle: arrival order and delays match for all 6 pairs.
6. Measure the real hole positions on the ring and update the array geometry.
7. GCC-PHAT DOA on the board, then delay-and-sum.
