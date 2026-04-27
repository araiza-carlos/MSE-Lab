/**
 * @file    button.c
 * @brief   Button Module Implementation
 *
 * Uses the GPIO driver to read the user button on the Nucleo-F411RE (PC13).
 * The button is active LOW: pin reads LOW when pressed.
 *
 * @author  Carlos Humberto Araiza Quintana
 * @version 1.0
 */

#include "button.h"


gpio_status_t button_init(void)
{
    gpio_status_t status;

    /* Enable clock for button port */
    status = gpio_initPort(BUTTON_PORT);
    if (status != GPIO_OK) return status;

    /* Configure pin as digital input */
    status = gpio_setPinMode(BUTTON_PORT, BUTTON_PIN, GPIO_MODE_INPUT);

    return status;
}

gpio_status_t button_get_state(button_state_t *state)
{
    if (state == NULL) return GPIO_INVALID;

    gpio_pin_state_t pin_state;
    gpio_status_t status = gpio_readPin(BUTTON_PORT, BUTTON_PIN, &pin_state);

    if (status != GPIO_OK) return status;

    /* Active LOW: pin LOW means button is pressed */
    *state = (pin_state == GPIO_PIN_LOW) ? BUTTON_PRESSED : BUTTON_RELEASED;

    return GPIO_OK;
}
