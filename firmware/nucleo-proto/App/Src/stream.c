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
#define MAGIC		0x3443494DU		// bytes "MIC4", marks start of each packet

typedef struct {
	uint32_t magic;
	uint32_t seq;
	int16_t pcm[BLOCK_SAMPLES * NUM_MICS];
} packet_t;


extern USBD_HandleTypeDef hUsbDeviceFS;

/* Debug values */
volatile uint32_t g_stream_drops;

/* Ring buffer */
static packet_t ring[RING_LEN];
static volatile uint32_t head;		// next slot to fill (interrupt changes this)
static volatile uint32_t tail;		// oldest unset slot (main loop changes this)
static uint32_t seq;
static uint8_t sending;				// 1 while USB sending ring[tail]


/* Private functions */

static int usb_busy(void)
{
	USBD_CDC_HandleTypeDef *cdc = hUsbDeviceFS.pClassData;
	return cdc == NULL || cdc->TxState != 0;
}


/* Public functions */

void stream_push(const int16_t *block)
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
	head = next;
}

void stream_task(void)
{
	if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) {		// not plugged in
		return;
	}

	if (sending) {
		if (usb_busy()) {		// last packet still going out
			return;
		}
		tail = (tail + 1U) % RING_LEN;
		sending = 0;
	}

	if (tail != head) {
		if (CDC_Transmit_FS((uint8_t*)&ring[tail], sizeof(packet_t)) == USBD_OK) {
			sending = 1;
		}
	}

}
