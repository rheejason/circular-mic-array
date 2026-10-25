# 8-mic board: layout checklist

Status as of the 2026-10-25 review of commit `7fe543d` ("PCB complete"). KiCad 10.0.7 ERC/DRC: 0 unconnected items,
no errors; only library-path and cosmetic warnings. No LCD on this revision.

## 1. Still to do before ordering

| Done | Item | Why |
|---|---|---|
| [x] | **Mounting holes**: 3 × M3 NPTH at 41.5 mm radius, 54° / 174° / 294° — placed and locked; DRC 0 errors | |
| [x] | Mounting-hole references H1/H2/H3, hidden, *Not in schematic* (applied 2026-10-25; DRC 0 errors, parity only the 13 TP labels) | |
| [ ] | **Upside-down mount:** enclosure floor clearance ≥ 12 mm under the components (J1/J3 headers ~8.5 mm, USB-C 7.5 mm + right-angle plug); translucent LED ring at the 0–4 mm band *below* the board edge | Parts now hang down |
| [ ] | **Top cover: fabric or thin perforated grille (decided)**, ≥ 30 % open, resting 2.5–3 mm above the board top face (on the screw heads or on bosses of that height). No gaskets | Fabric is acoustically transparent; a thin high-open-area grille over a 3 mm gap resonates above ~10 kHz, outside the speech/DOA band |
| [ ] | **Mounting**: board components-down on 3 printed posts from the enclosure floor at H1–H3 (Ø 6.5 mm max at H3, tall enough to clear J1/J3/J2 + USB plug, ~15–20 mm), M3 heat-set inserts, low-profile M3 button-head screws (1.65 mm head) from the top face | |
| [x] | 4 GND thermal vias (0.6/0.3 mm) in U5's tab pad — added 2026-10-25 | Not strictly needed (≈ 60 °C typical junction), cheap margin |
| [ ] | Trim the J1/J3 leads and J2 shell tabs on the top face to < 1.5 mm, so they stay under the grille gap | They poke out of the face that now points up |
| [ ] | Add GCT's USB4145 STEP model to J2 | J2 has no 3D model, so the enclosure fit check can't see it |
| [x] | Pin names on silkscreen: J1 (3V3, GND, SWDIO, SWCLK, NRST, below the pins) and J3 (GND, FS, BCLK, SDATA, RX, TX, left of the pins) — added 2026-10-25; DRC 0 errors, no silkscreen warnings | |
| [ ] | Run *Update PCB from Schematic* once | 13 test-point value mismatches (cosmetic, but they end up in the BOM / position files) |
| [ ] | Copy the 5 custom symbols and `T5838.pretty` into the project; point `sym-lib-table` / `fp-lib-table` at `${KIPRJMOD}` | Tables point at `C:/Users/sstim/Downloads/...` (ERC/DRC warnings, Update PCB breaks on other computers) |
| [ ] | Fill in the title block (title, rev, date) | Cosmetic |
| [ ] | Optional: green/red LED + 2.2 k on PB0 / PB14 | Sign of life before USB works; J3's UART now covers most of this |
| [ ] | Optional: ADAU7118 exposed-pad paste reduced to ~60 % (window it) if reflowing by hand | Full paste on the pad can float the 3 × 3 mm part |

## 2. Done and verified

| Item | Result |
|---|---|
| 4 layers: F.Cu signal / In1 GND plane (97 % fill) / In2 +3V3 plane (95 %) / B.Cu signal | ✓ |
| Board outline 140 mm circle; mic holes at 55.000 mm, 45° apart | ✓ |
| ADAU7118 near centre, U7 6.6 mm from it; BCLK / FSYNC / SDATA 14–19 mm, PE4–6 5–7 mm | ✓ |
| 0.1 µF ≤ 2.5 mm from every mic VDD; STM32 VDD caps 2–4 mm; VCAP caps at pins 71/106 | ✓ |
| No copper or vias inside any mic sealing ring or sound hole | ✓ |
| Crystal ~3 mm from PH0/PH1; footprint matches ECS ECX-53Q land (1.9 × 2.4 pads, 2.2 gap) | ✓ |
| USB-C USB4145 footprint matches GCT recommended layout (slots 1.10 × 0.60, pegs Ø0.71, pads 0.30 × 0.71) | ✓ |
| USBLC6 pins 1↔6 and 3↔4 are joined inside the chip (ST DS4260 functional diagram), so flow-through routing is correct | ✓ |
| LEDs at 65 mm radius facing outward, OUTk at 281.25° + 22.5°·k (counter-clockwise, MK3 = 0°) | ✓ |
| U7 B4 / A4 left open: allowed, every data I/O has a 71 kΩ internal pull-down (SN74AXC4T774 datasheet Rev C §7.3.9) | ✓ |
| J3 debug header: GND, FSYNC, BCLK, SDATA (1.8 V side), PB11 USART3_RX, PB10 USART3_TX | ✓ |
| Test pads: PDM_CLK0/1, PDM_DAT0–3, +1V8, +3V3, +5V, 4 × GND | ✓ |
| All parts on the top side (single-sided assembly) | ✓ |

## 3. Assembly notes

- Stencil (top only) + reflow or hot plate, or PCBA. The T5838 (LGA) and ADAU7118 (0.5 mm LFCSP) can't be done with an iron.
- No-clean flux; don't wash the mics or blow air into the sound holes.
- J2 shell stakes and J1/J3 are through-hole: solder them from the bottom after reflow.
