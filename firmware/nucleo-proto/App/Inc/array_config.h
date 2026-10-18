/*
 * array_config.h
 */
#ifndef ARRAY_CONFIG_H
#define ARRAY_CONFIG_H

/* Geometry: mic 0 at 0 deg (+x), then counter-clockwise every 90 deg */
#define NUM_MICS            4U
#define MIC_RADIUS_M        0.045f

/* Audio: 1.536 MHz PDM clock / 32 = 48 kHz */
#define FS_HZ               48000U
#define PDM_DECIMATION      32U
#define BLOCK_SAMPLES       48U     // samples per mic per block (1 ms)

// PDM bytes in one block, all mics: 48 samples x 32 bits / 8 x 4 mics = 768
#define PDM_BLOCK_BYTES     (BLOCK_SAMPLES * PDM_DECIMATION / 8U * NUM_MICS)

#endif /* ARRAY_CONFIG_H */
