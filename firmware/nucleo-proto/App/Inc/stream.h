/*
 * stream.h
 *
 *  Created on: Oct 18, 2026
 *      Author: jasonr8
 */

#ifndef INC_STREAM_H_
#define INC_STREAM_H_

#include <stdint.h>

void stream_push(const int16_t *block, const int16_t *beam);	 // add 1 ms of mics + beam (called from audio interrupt)
void stream_task(void);					 // send waiting blocks over USB (called from main loop)

/* Debug values (Live Expressions) */
extern volatile uint32_t g_stream_drops; // blocks dropped because ring was full (should be 0)


#endif /* INC_STREAM_H_ */
