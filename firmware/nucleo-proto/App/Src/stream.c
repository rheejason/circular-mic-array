/*
 * stream.c
 *
 *  Created on: Oct 18, 2026
 *      Author: jasonr8
 */

#include <string.h>
#include "stream.h"
#include "array_config.h"
#include "usbd_cdc_if.h"

#define RING_LEN	32U				// packets in the ring = 32 ms of audio
#define MAX_BATCH	4U				// packets per USB transfer when behind (4 x 488 bytes; keep < 8, see stream_task)
#define MAGIC		0x4243494DU		// bytes "MICB" (mics + beam), marks start of each packet

typedef struct {
	uint32_t magic;
	uint32_t seq;
	int16_t pcm[BLOCK_SAMPLES * NUM_MICS];	// m0 m1 m2 m3 m0 ...
	int16_t beam[BLOCK_SAMPLES];			// beamformer output
} packet_t;									// 488 bytes


extern USBD_HandleTypeDef hUsbDeviceFS;

/* Debug values */
volatile uint32_t g_stream_drops;

/* Ring buffer */
static packet_t ring[RING_LEN];
static volatile uint32_t head;		// next slot to fill (interrupt changes this)
static volatile uint32_t tail;		// oldest unsent slot (main loop changes this)
static uint32_t seq;
static uint32_t sending;			// packets in the USB transfer going out now (0 = idle)


/* Private functions */

static int usb_busy(void)
{
	USBD_CDC_HandleTypeDef *cdc = hUsbDeviceFS.pClassData;
	return cdc == NULL || cdc->TxState != 0;
}


/* Public functions */

void stream_push(const int16_t *block, const int16_t *beam)
{
	uint32_t next = (head + 1U) % RING_LEN;

	if (next == tail) {			// ring full, drop this block
		g_stream_drops++;
		seq++;
		return;
	}

	ring[head].magic = MAGIC;
	ring[head].seq = seq++;
	memcpy(ring[head].pcm, block, sizeof ring[head].pcm);
	memcpy(ring[head].beam, beam, sizeof ring[head].beam);
	head = next;
}

void stream_task(void)
{
	if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) {		// not plugged in
		return;
	}

	if (sending) {
		if (usb_busy()) {		// last transfer still going out
			return;
		}
		tail = (tail + sending) % RING_LEN;
		sending = 0;
	}

	// Send every waiting packet in one transfer (up to MAX_BATCH), so a slow main loop pass
	// is caught up instead of dropping blocks. Only packets next to each other in the ring:
	// at the end of the ring, the rest goes in the next transfer.
	// 8 packets would be a multiple of 64 bytes (the USB packet size) and need an extra empty packet.
	uint32_t h = head;
	if (tail != h) {
		uint32_t n = (h > tail) ? (h - tail) : (RING_LEN - tail);
		if (n > MAX_BATCH) {
			n = MAX_BATCH;
		}
		if (CDC_Transmit_FS((uint8_t*)&ring[tail], (uint16_t)(n * sizeof(packet_t))) == USBD_OK) {
			sending = n;
		}
	}
}
