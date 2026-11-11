// TLC59116F 16-channel I2C LED driver (U3). All 16 ring LEDs hang off it; no LED is on an MCU pin.
// LED k = output OUTk. Going up in k walks around the ring.
#ifndef TLC59116_H
#define TLC59116_H

#include "stm32h7xx_hal.h"

#define TLC59116_NUM_LEDS 16U

// Wake the chip and put every output in PWM mode, all LEDs off. HAL_OK if the chip answered.
HAL_StatusTypeDef tlc59116_init(I2C_HandleTypeDef *hi2c);

// Brightness of LED k (0..15): 0 = off, 255 = fully on.
HAL_StatusTypeDef tlc59116_set(I2C_HandleTypeDef *hi2c, uint8_t k, uint8_t brightness);

// Same brightness on all 16 LEDs.
HAL_StatusTypeDef tlc59116_set_all(I2C_HandleTypeDef *hi2c, uint8_t brightness);

#endif
