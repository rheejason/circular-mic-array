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

/* DOA */
#define FRAME_LEN			512U								// samples per mic per DOA frame (10.7 ms)
#define N_FFT				(2U * FRAME_LEN)					// zero-padded so correlation doesn't wrap
#define DOA_K				9U									// frames summed per update
#define DOA_K_LO			7U									// ceil(300 * N_FFT / FS_HZ)
#define DOA_K_HI			85U									// floor(4000 * N_FFT / FS_HZ)
#define DOA_NB				(DOA_K_HI - DOA_K_LO + 1U)			// 79 bins in band
#define NUM_PAIRS			(NUM_MICS * (NUM_MICS - 1U) / 2U)
#define SPEED_OF_SOUND		343.0f
#define DOA_MAX_SHIFT		14U									// 13 + 1 so parabola can get true delay max 12.59
#define DOA_GATE_DB			10.0f
#define DOA_FLOOR_RISE_DB	0.05f

/* Beamformer (STFT, 50% overlap) */
#define BF_FFT				512U								// samples per frame = FFT size (10.7ms)
#define BF_HOP				(BF_FFT / 2U)						// new frame every 256 samples (5.3ms)
#define BF_BINS				(BF_FFT / 2U)						// complex slots in packed spectrum (slot 0 = DC + Nyquist)

#endif /* ARRAY_CONFIG_H */
