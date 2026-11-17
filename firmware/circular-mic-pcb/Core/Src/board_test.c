// Board bring-up test: all 16 LEDs on, steady, at a low level. Any LED that stays dark needs fixing.

#include "board_test.h"
#include "main.h"
#include "tlc59116.h"

extern I2C_HandleTypeDef hi2c1;

#define LED_BRIGHTNESS 255U  // 0 = off, 255 = full. 255 keeps the outputs on for steady multimeter readings; 64 for normal viewing

volatile uint32_t g_led_init_fails;   // counts up while the TLC59116F does not answer on I2C

void board_test_init(void)
{
    while (tlc59116_init(&hi2c1) != HAL_OK || tlc59116_set_all(&hi2c1, LED_BRIGHTNESS) != HAL_OK) {
        g_led_init_fails++;
        HAL_Delay(500);
    }
}

void board_test_loop(void)
{
}
