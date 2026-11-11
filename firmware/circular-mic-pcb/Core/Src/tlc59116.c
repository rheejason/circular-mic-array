#include "tlc59116.h"

// A0..A3 are all tied to GND, so the 7-bit address is 0x60. HAL wants it shifted left by one.
#define TLC_ADDR        (0x60U << 1)

#define REG_MODE1       0x00U   // power-up value 0x11: oscillator off (SLEEP = 1)
#define REG_PWM0        0x02U   // PWM0..PWM15 = 0x02..0x11, one brightness byte per output
#define REG_LEDOUT0     0x14U   // LEDOUT0..3 = 0x14..0x17, 2 bits per output
#define LEDOUT_ALL_PWM  0xAAU   // 0b10 for each of the 4 outputs: follow the PWMx register

#define TLC_TIMEOUT_MS  10U

static HAL_StatusTypeDef write_reg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(hi2c, TLC_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, TLC_TIMEOUT_MS);
}

HAL_StatusTypeDef tlc59116_init(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef st = write_reg(hi2c, REG_MODE1, 0x00U);   // SLEEP = 0: start the oscillator
    if (st != HAL_OK) {
        return st;
    }
    HAL_Delay(1);                                               // oscillator needs 500 us

    st = tlc59116_set_all(hi2c, 0);
    for (uint8_t i = 0; i < 4U && st == HAL_OK; i++) {
        st = write_reg(hi2c, REG_LEDOUT0 + i, LEDOUT_ALL_PWM);
    }
    return st;
}

HAL_StatusTypeDef tlc59116_set(I2C_HandleTypeDef *hi2c, uint8_t k, uint8_t brightness)
{
    if (k >= TLC59116_NUM_LEDS) {
        return HAL_ERROR;
    }
    return write_reg(hi2c, REG_PWM0 + k, brightness);
}

HAL_StatusTypeDef tlc59116_set_all(I2C_HandleTypeDef *hi2c, uint8_t brightness)
{
    HAL_StatusTypeDef st = HAL_OK;
    for (uint8_t k = 0; k < TLC59116_NUM_LEDS && st == HAL_OK; k++) {
        st = write_reg(hi2c, REG_PWM0 + k, brightness);
    }
    return st;
}
