/*
 * array_config.h
 *
 * Array geometry and audio constants for the 4-mic bench rig. Keep in sync with the Python side.
 */
#ifndef ARRAY_CONFIG_H
#define ARRAY_CONFIG_H

/* Geometry: mic 0 at 0 deg (+x), counter-clockwise, 90 deg apart */
#define NUM_MICS            4U
#define MIC_RADIUS_M        0.045f

/* Audio */
#define FS_HZ               48000U
#define BLOCK_SAMPLES       48U         /* samples per mic per block = 1 ms at 48 kHz */
#define PDM_DECIMATION      64U         /* 3.072 MHz PDM clock / 64 = 48 kHz */

/* PDM bytes for one block, all mics: 48 samples * 64 bits / 8 bits per byte * 4 mics = 1536 */
#define PDM_BLOCK_BYTES     (BLOCK_SAMPLES * PDM_DECIMATION / 8U * NUM_MICS)

/* RAM recording (AXI SRAM is 512 KB, so ~1.3 s max at 4 ch x 16 bit x 48 kHz) */
#define REC_SECONDS         1U
#define REC_SAMPLES         (FS_HZ * REC_SECONDS)   /* per mic */

#endif /* ARRAY_CONFIG_H */
