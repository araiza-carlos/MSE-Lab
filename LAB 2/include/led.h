/**
 * @file    led.h
 * @brief   LED Module API
 *
 * High-level abstraction for controlling an LED connected to a GPIO pin.
 * Internally uses the GPIO driver.
 *
 * Target: Nucleo-F411RE — User LED on PA5
 *
 * @author  Carlos Cayetano
 * @version 1.0
 */

#ifndef LED_H
#define LED_H

#include "gpio_driver.h"

/* -----------------------------------------------------------------------
 * LED Hardware Mapping (Nucleo-F411RE)
 * --------------------------------------------------------------------- */
#define LED_PORT    GPIOA
#define LED_PIN     GPIO_PIN_8

/* -----------------------------------------------------------------------
 * Function Prototypes
 * --------------------------------------------------------------------- */

/**
 * @brief  Initializes the LED GPIO pin as a push-pull output.
 *         Enables port clock and sets pin mode to output.
 *         LED is turned off after initialization.
 * @return GPIO_OK on success.
 */
gpio_status_t led_init(void);

/**
 * @brief  Turns the LED on (drives pin HIGH).
 * @return GPIO_OK on success.
 */
gpio_status_t led_on(void);

/**
 * @brief  Turns the LED off (drives pin LOW).
 * @return GPIO_OK on success.
 */
gpio_status_t led_off(void);

/**
 * @brief  Toggles the LED state.
 * @return GPIO_OK on success.
 */
gpio_status_t led_toggle(void);

#endif /* LED_H */
