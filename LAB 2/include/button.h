/**
 * @file    button.h
 * @brief   Button Module API
 *
 * Reads a push-button connected to a GPIO pin.
 * uses the GPIO driver.
 *
 * Target: Nucleo-F411RE — User Button (B1) on PC13
 * Note:   The user button is active LOW (pressed = 0, released = 1).
 *
 * @author  Vania Leal
 * @version 1.0
 */

#ifndef BUTTON_H
#define BUTTON_H

#include "gpio_driver.h"
#include <stddef.h> 
/* -----------------------------------------------------------------------
 * Button Hardware Mapping (Nucleo-F411RE)
 * --------------------------------------------------------------------- */
#define BUTTON_PORT     GPIOC
#define BUTTON_PIN      GPIO_PIN_13

/* -----------------------------------------------------------------------
 * Button logical states
 * --------------------------------------------------------------------- */
typedef enum
{
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED  = 1
} button_state_t;

/* -----------------------------------------------------------------------
 * Function Prototypes
 * --------------------------------------------------------------------- */

/**
 * @brief  Initializes the button GPIO pin as a digital input.
 *         Enables port clock and sets pin mode to input.
 * @return GPIO_OK on success.
 */
gpio_status_t button_init(void);

/**
 * @brief  Returns the logical state of the button.
 *         Accounts for active-low wiring: IDR LOW = BUTTON_PRESSED.
 * @param  state  Output: BUTTON_PRESSED or BUTTON_RELEASED.
 * @return GPIO_OK on success, GPIO_INVALID if state pointer is NULL.
 */
gpio_status_t button_get_state(button_state_t *state);

#endif /* BUTTON_H */
