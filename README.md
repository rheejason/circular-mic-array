# Audio Zoom Lens

UIUC ECE 395: an 8-mic circular beamforming microphone array (steerable conference-table mic
and acoustic imager).

| Folder | What | Tooling |
|---|---|---|
| `python/` | DSP prototype: verify the beamforming and DOA math before porting | Python, pytest |
| `firmware/` | STM32H743 firmware (NUCLEO-H743 bench rig, then the custom board) | STM32CubeIDE |
| `hardware/` | Mic array PCB (ADAU7118, 8 PDM mics, LCD, LED ring) | KiCad |

The Python prototype is the reference: firmware is tested against vectors it exports.
