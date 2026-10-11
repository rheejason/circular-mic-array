# Circular Beamforming Microphone Array

UIUC ECE 395: an 8-mic circular beamforming microphone array (steerable conference-table mic
and acoustic imager).

| Folder | What | Tooling |
|---|---|---|
| `firmware/nucleo-proto/` | Bench prototype: NUCLEO-H753ZI + 4 Adafruit PDM mics on a 3D-printed 45 mm ring | STM32CubeIDE |
| `firmware/final/` | Firmware for the custom 8-mic STM32H743 board | STM32CubeIDE |
| `firmware/common/` | Hardware-independent C shared by both: DOA (GCC-PHAT), beamformer, ring buffer | CMSIS-DSP |
| `hardware/kicad/` | 8-mic board: 8 × T5838 PDM mics → ADAU7118 → STM32H743 SAI | KiCad |
| `hardware/mechanical/` | Bench rig mount and enclosure (STEP / 3MF exports) | Fusion 360 |
| `hardware/bom/` | Bills of materials | |