/**
 * @file    main.c
 * @brief   Application: ADXL345 → LED tilt control (STM32F411RE)
 * @author  Kheara Kieley y Vania Leal 
 * @date    May 2026
 *
 * Wiring:
 *   PB8  → SCL  (ADXL345)  + 4.7 kΩ pull-up to 3.3 V
 *   PB9  → SDA  (ADXL345)  + 4.7 kΩ pull-up to 3.3 V
 *   3.3V → VCC  (ADXL345)
 *   GND  → GND  (ADXL345)
 *   GND  → SDO  (ADXL345)  → address 0x53
 *   VCC  → CS   (ADXL345)  → enables I2C mode
 *   PA5  → LED LD2 (Nucleo board)
 */

#include "stm32f4xx.h"
#include "gpio_driver.h"
#include "i2c.h"
#include "sensor.h"
#include <stdint.h>

/* ─── LED ──────────────────────────────────────────────────────────────────── */
#define LED_PORT   GPIOA
#define LED_PIN    GPIO_PIN_5

/* |Z| < 180 LSB (~0.7 g) → board is tilted → LED ON */
#define TILT_THRESHOLD_Z   180

/* ─── Simple busy-wait delay ───────────────────────────────────────────────── */
static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0U; i < ms * 8400U; i++)
    {
        __asm__("nop");
    }
}

/* ─── LED helpers ──────────────────────────────────────────────────────────── */
static void led_on (void) { gpio_setPin  (LED_PORT, LED_PIN); }
static void led_off(void) { gpio_clearPin(LED_PORT, LED_PIN); }

static void led_flash(uint8_t times, uint32_t on_ms, uint32_t off_ms)
{
    for (uint8_t i = 0U; i < times; i++)
    {
        led_on();  delay_ms(on_ms);
        led_off(); delay_ms(off_ms);
    }
}

/* ─── LED initialization ───────────────────────────────────────────────────── */
static void led_init(void)
{
    gpio_initPort(LED_PORT);
    gpio_setPinMode(LED_PORT, LED_PIN, GPIO_MODE_OUTPUT);
    gpio_clearPin  (LED_PORT, LED_PIN);
}

/* ─── Integer absolute value for int16 ────────────────────────────────────── */
static int16_t abs16(int16_t v)
{
    return (v < 0) ? (int16_t)(-v) : v;
}

/* ─── Main ─────────────────────────────────────────────────────────────────── */
int main(void)
{
    /* 1. Initialize LED on PA5 */
    led_init();

    /* 2. Initialize I2C1 at 100 kHz — PB8 (SCL) and PB9 (SDA) configured inside i2c_init */
    i2c_init();

    /* 3. Initialize ADXL345 accelerometer */
    int8_t status = sensor_init();

    if (status != ADXL345_OK)
    {
        /* Initialization failed: rapid continuous blink */
        while (1) { led_flash(1U, 100U, 100U); }
    }

    /* 3 blinks = initialization successful */
    led_flash(3U, 300U, 200U);

    /* 4. Main loop: read accelerometer and control LED based on tilt */
    while (1)
    {
        ADXL345_Data_t accel;

        if (sensor_readData(&accel) == ADXL345_OK)
        {
            if (abs16(accel.z) < TILT_THRESHOLD_Z)
                led_on();    /* board is tilted  */
            else
                led_off();   /* board is flat    */
        }

        delay_ms(100U);
    }

    return 0;
}