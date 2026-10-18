# Firmware

| Folder | Board | Capture path |
|---|---|---|
| `nucleo-proto/` | NUCLEO-H753ZI bench rig, 4 mics | PDM 1.536 MHz → SAI1 (PDM mode) → PDM2PCM ÷32 → 48 kHz |
| `final/` | Custom 8-mic board, STM32H743 | 8 PDM mics → ADAU7118 → TDM8 → SAI |
| `common/` | Both | No HAL calls: GCC-PHAT DOA, delay-and-sum beamformer, ring buffer |

Board-specific code (capture, DMA callbacks, serial dump, the array geometry header) lives in each
board's project. Anything that only does math goes in `common/` so it moves to the final board unchanged.

## nucleo-proto app code (`nucleo-proto/App/`)

| File | Job |
|---|---|
| `array_config.h` | Mic count, geometry, sample rate, block size |
| `app_main.c` | Called from `main.c`: starts capture, blinks the green LED |
| `mic_capture.c` | SAI1 + DMA ping-pong buffer, PDM → PCM per mic, 1 ms blocks |

SAI1 settings that matter for 4 PDM mics (CubeMX's defaults are wrong): frame length 32, 2 slots,
first bit LSB, frame sync polarity **active high**, audio frequency 192 kHz.

## STM32CubeIDE setup

- Keep the CubeIDE **workspace folder outside this repo**. Import a project with
  *File → Import → Existing Projects into Workspace* and point it at `firmware/nucleo-proto/`.
- Commit the `.ioc`, `.project`, `.cproject`, `Core/`, `Drivers/` and `Middlewares/`, so a fresh clone
  builds without regenerating. Build folders (`Debug/`, `Release/`) are ignored.
- In CubeMX, tick *Generate peripheral initialization as a pair of .c/.h files per peripheral*.
  Only edit `main.c` inside `USER CODE` blocks; put everything else in your own files.
- Add `firmware/common/` to the project's include and source paths (linked folder).
