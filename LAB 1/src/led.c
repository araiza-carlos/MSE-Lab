/**
 * @file    led.c
 * @brief   LED Module Implementation
 *
 * Uses the GPIO driver to control the user LED on the Nucleo-F411RE (PA5).
 *
 * @author  Carlos Cayetano
 * @version 1.0
 */

#include "led.h"


gpio_status_t led_init(void)
{
    gpio_status_t status;

    /*  clock  */
    status = gpio_initPort(LED_PORT);
    if (status != GPIO_OK) return status;

    /*  output */
    status = gpio_setPinMode(LED_PORT, LED_PIN, GPIO_MODE_OUTPUT);
    if (status != GPIO_OK) return status;

    /*  LED off */
    status = gpio_clearPin(LED_PORT, LED_PIN);

    return status;
}

gpio_status_t led_on(void)
{
    return gpio_setPin(LED_PORT, LED_PIN);
}

gpio_status_t led_off(void)
{
    return gpio_clearPin(LED_PORT, LED_PIN);
}

gpio_status_t led_toggle(void)
{
    return gpio_togglePin(LED_PORT, LED_PIN);
}
