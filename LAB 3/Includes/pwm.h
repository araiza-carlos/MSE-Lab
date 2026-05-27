/**
 * @file    pwm.h
 * @brief   PWM Module API
 *
 * High-level abstraction for generating PWM signals using a hardware
 * timer in output compare PWM mode. Internally uses the TIM driver
 * and the GPIO driver for alternate function configuration.
 *
 * Target: TIM3 Channel 1 on PA6 (AF2) — Nucleo-F411RE
 *
 * Pin mapping (from UM1724 user manual):
 *   PA6 → TIM3_CH1 → Alternate Function 2 (AF2)
 *
 * @author  Vania Leal 
 * @version 1.0
 */

#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include "tim_driver.h"
#include "gpio_driver.h"

/* -----------------------------------------------------------------------
 * Hardware Mapping
 * --------------------------------------------------------------------- */
#define PWM_TIM         TIM3            /**< Timer for PWM generation      */
#define PWM_CHANNEL     TIM_CHANNEL_1   /**< Capture/compare channel       */
#define PWM_GPIO_PORT   GPIOA           /**< GPIO port for PWM output      */
#define PWM_GPIO_PIN    GPIO_PIN_6      /**< GPIO pin for PWM output       */
#define PWM_GPIO_AF     2U              /**< Alternate function: TIM3_CH1  */

/* -----------------------------------------------------------------------
 * Function Prototypes
 * --------------------------------------------------------------------- */

/**
 * @brief  Initializes the PWM peripheral and GPIO pin.
 *         - Enables TIM3 and configures it for the given frequency.
 *         - Configures PA6 in alternate function mode (AF2).
 *         - Sets PWM mode 1 on channel 1.
 *         - Does NOT start the timer (call pwm_start() separately).
 * @param  freq_hz   PWM frequency in Hz (e.g. 1000 for 1 kHz).
 * @return TIM_OK on success, TIM_INVALID on bad parameters.
 */
tim_status_t pwm_init(uint32_t freq_hz);

/**
 * @brief  Sets the PWM duty cycle and updates the compare value.
 * @param  freq_hz    PWM frequency in Hz (must match pwm_init frequency).
 * @param  duty_pct   Duty cycle as a percentage (0 … 100).
 * @return TIM_OK on success, TIM_INVALID on bad parameters.
 */
tim_status_t pwm_setSignal(uint32_t freq_hz, uint8_t duty_pct);

/**
 * @brief  Starts the PWM output (enables timer and compare channel).
 * @return TIM_OK on success.
 */
tim_status_t pwm_start(void);

/**
 * @brief  Stops the PWM output (disables compare channel and timer).
 * @return TIM_OK on success.
 */
tim_status_t pwm_stop(void);

#endif /* PWM_H */
