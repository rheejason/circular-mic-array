/*
 * beamformer.h
 *
 *  Created on: Oct 22, 2026
 *      Author: jasonr8
 */

#ifndef INC_BEAMFORMER_H_
#define INC_BEAMFORMER_H_

#include <stdint.h>
#include "array_config.h"

int  beam_init(void);
void beam_push(const int16_t *block);	// add one 1ms block (called from audio interrupt)
void beam_pop(int16_t *dst);			// take 1ms of beam output, BLOCK_SAMPLES (called from audio interrupt)
void beam_task(void);					// process a hop when one is ready (called from main loop)

/* Debug values (Live Expressions) */
extern volatile float g_beam_az;			// 0-360
extern volatile uint32_t g_beam_cycles;		// CPU cycles of last hop (one hop = 5.3ms = 2.56M cycles)
extern volatile uint32_t g_beam_overruns;	// hops main loop missed (expected 0)
extern volatile uint32_t g_beam_underruns;	// times output FIFO ran dry (expected 0)

#endif /* INC_BEAMFORMER_H_ */
