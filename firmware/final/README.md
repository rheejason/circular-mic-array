# final: 8-mic board firmware

Start as a copy of `nucleo-proto/`. Everything after capture (`stream.c`, `doa.c`, USB CDC) is reused; only the clock,
the SAI setup and `mic_capture.c` change.

## CubeMX (.ioc) changes from nucleo-proto

| Setting | nucleo-proto | final board |
|---|---|---|
| HSE | 8 MHz bypass (ST-LINK MCO) | **16 MHz crystal** (HSE "Crystal/Ceramic Resonator") |
| PLL1 | M = 1, N = 120, P = 2 → 480 MHz | **M = 2**, N = 120, P = 2 → 480 MHz (same VCO, 8 MHz input) |
| PLL3 → SAI1 | M = 2, N = 49, FRACN = 1245, P = 4 → 49.152 MHz | **M = 4**, N = 49, FRACN = 1245, P = 4 → 49.152 MHz (−0.45 ppm) |
| USB clock | HSI48 + CRS synced to USB SOF | unchanged |
| Supply | LDO, VOS0 | unchanged (H753/H743 LQFP-144 has no SMPS) |
| PA9 VBUS, PA8 SOF | assigned | remove (not connected on the board; `vbus_sensing_enable` is already DISABLE) |
| PDM2PCM, CRC | used | remove |
| SAI1 block A | PDM, 4 mics | **Master receive, free protocol, TDM-8** (below) |
| Pins | PE2 CK1, PE4 D2, PE6 D1 | **PE4 SAI1_FS_A, PE5 SAI1_SCK_A, PE6 SAI1_SD_A**, MCLK off, GPIO speed high |
| LEDs | LD1 PB0, LD3 PB14 | same pins if the board LEDs are added there |
| I2C1 | — | PB6 SCL, PB7 SDA, 400 kHz (TLC59116F at 0x60) |

## SAI1 block A for the ADAU7118

The ADAU7118 runs in standalone hardware mode: TDM-8, 32-bit slots, 24-bit data, 64x decimation, data delayed 1 BCLK
after FSYNC, SDATA changes on BCLK falling edge. The STM32 is the clock master.

| HAL field | Value |
|---|---|
| AudioMode | SAI_MODEMASTER_RX |
| Protocol | SAI_FREE_PROTOCOL |
| DataSize | SAI_DATASIZE_32 (24-bit sample in the top 24 bits) |
| FirstBit | SAI_FIRSTBIT_MSB |
| ClockStrobing | SAI_CLOCKSTROBING_RISINGEDGE (sample on rising edge) |
| AudioFrequency | SAI_AUDIO_FREQUENCY_48K (HAL computes MCKDIV = 4 → BCLK 12.288 MHz) |
| MckOutput | disabled |
| FrameLength / ActiveFrameLength | 256 / 1 |
| FSDefinition / FSPolarity / FSOffset | SAI_FS_STARTFRAME / SAI_FS_ACTIVE_HIGH / SAI_FS_BEFOREFIRSTBIT |
| SlotSize / SlotNumber / SlotActive | SAI_SLOTSIZE_32B / 8 / 0xFF |
| DMA | circular, word (32-bit) memory and peripheral size, buffer in AXI SRAM |

Do not raise BCLK above 12.288 MHz: the level shifter round trip leaves about 8 ns of margin.

## mic_capture.c

One 1 ms block = 48 frames × 8 slots = 384 `int32_t` words. Replace the PDM2PCM loop with a remap + gain to the same
`int16` interleaved block that `stream_push` and `doa_push` already take:

```c
#define CAPTURE_GAIN_BITS 2U   // +12 dB before cutting to 16 bits; tune so speech isn't buried in 16-bit noise

// TDM slot of each DOA mic index (mic m at 45*m deg CCW from +x, component side up).
// Slot k = ADAU7118 channel k; the SELECT = 1V8 mic of each data line comes first, as on the rig.
// m:    0    1    2    3    4    5    6    7
// MK:   MK3  MK2  MK1  MK8  MK7  MK6  MK5  MK4
static const uint8_t slot_of_mic[NUM_MICS] = {3, 0, 1, 6, 7, 4, 5, 2};

for (uint32_t n = 0; n < BLOCK_SAMPLES; n++) {
    for (uint32_t m = 0; m < NUM_MICS; m++) {
        int32_t x = (int32_t)tdm[n * 8U + slot_of_mic[m]] >> (16U - CAPTURE_GAIN_BITS);
        pcm_block[n * NUM_MICS + m] = (int16_t)__SSAT(x, 16);
    }
}
```

Confirm `slot_of_mic` with a tap test (tap each mic, check which channel spikes). The first 64 frames after the
clocks start are silent (ADAU7118 start-up), so ignore the first few blocks.

## Other constants

| File | Change |
|---|---|
| `array_config.h` | NUM_MICS 8, MIC_RADIUS_M 0.055f, DOA_MAX_SHIFT 17 (max delay 15.4 samples + 1); remove PDM_* |
| `stream.c` | MAGIC "MIC8"; packet = 8 + 48 × 8 × 2 = 776 bytes per ms (fits USB full speed). If `g_stream_drops` rises, send 2–4 blocks per `CDC_Transmit_FS` |
| `python/tools/stream_rx.py` | NUM_MICS = 8, MAGIC = b"MIC8" |
