/*
 * doa.h
 *
 *  Created on: Oct 19, 2026
 *      Author: jasonr8
 */

#ifndef INC_DOA_H_
#define INC_DOA_H_

#include <stdint.h>
#include "array_config.h"

int  doa_init(void);
void doa_push(const int16_t *block);
void doa_task(void);

/* Debug values (Live Expressions) */
extern volatile float 	 g_doa_az;					// azimuth (0-360), CCW from mic 0, from SRP-PHAT
/* extern volatile float g_doa_az_ls; */		// old least squares, not used
extern float 			 g_srp_map[SRP_N_ANGLES];	// score of each angle, index d = angle / 2 deg
extern volatile uint32_t g_doa_valid;				// 1 if last update passed energy gate
/* extern volatile float g_doa_tau_us[NUM_PAIRS]; */	// old least squares, not used
extern volatile float 	 g_doa_level_db;			// in-band level of last update
extern volatile float 	 g_doa_floor_db;			// tracked noise floor
extern volatile uint32_t g_doa_updates;				// updates ~10/s
extern volatile uint32_t g_doa_cycles;				// CPU cycle of last doa_task()
extern volatile uint32_t g_doa_overruns;			// frames main loop missed, expected 0

#endif /* INC_DOA_H_ */
