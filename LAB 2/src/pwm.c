/**
 * @file    pwm.c
 * @brief   PWM Module Implementation
 *
 * Uses TIM3 CH1 on PA6 (AF2) to generate a PWM signal.
 * The frequency is set via tim_setTimerFreq() and the duty cycle
 * is calculated as a fraction of ARR.
 *
 * @author  Carlos Cayetano
 * @version 1.0
 */

#include "pwm.h"

/**
 * @brief Cached ARR value — needed to compute CCR from duty percentage.
 */
static uint32_t pwm_arr = 0;

/**
 * @brief Initializes TIM3 and PA6 for PWM output.
 */
tim_status_t pwm_init(uint32_t freq_hz)
{
    tim_status_t status;

    if (freq_hz == 0) return TIM_INVALID;

    /* --- Configure GPIO PA6 as TIM3_CH1 (AF2) --- */
    status = (tim_status_t)gpio_initPort(PWM_GPIO_PORT);
    if (status != TIM_OK) return status;

    status = (tim_status_t)gpio_setAlternateFunction(PWM_GPIO_PORT,
                                                     PWM_GPIO_PIN,
                                                     PWM_GPIO_AF);
    if (status != TIM_OK) return status;

    /* --- Initialize TIM3 --- */
    status = tim_initTimer(PWM_TIM);
    if (status != TIM_OK) return status;

    /* --- Configure frequency --- */
    status = tim_setTimerFreq(PWM_TIM, freq_hz);
    if (status != TIM_OK) return status;

    /* Cache ARR for duty cycle calculations */
    pwm_arr = PWM_TIM->ARR;

    /* --- Configure PWM mode 1 on channel 1 --- */
    status = tim_setTimerCompareMode(PWM_TIM, PWM_CHANNEL, TIM_COMPARE_MODE_PWM1);
    if (status != TIM_OK) return status;

    /* --- Set initial duty cycle to 0% --- */
    status = tim_setTimerCompareChannelValue(PWM_TIM, PWM_CHANNEL, 0);

    return status;
}

/**
 * @brief Sets frequency and duty cycle.
 *
 * CCR = (duty_pct / 100) * (ARR + 1)
 *
 * PWM mode 1: output HIGH while CNT < CCR → duty = CCR / (ARR + 1)
 */
tim_status_t pwm_setSignal(uint32_t freq_hz, uint8_t duty_pct)
{
    tim_status_t status;

    if (freq_hz == 0)    return TIM_INVALID;
    if (duty_pct > 100U) return TIM_INVALID;

    /* Reconfigure frequency if needed */
    status = tim_setTimerFreq(PWM_TIM, freq_hz);
    if (status != TIM_OK) return status;

    pwm_arr = PWM_TIM->ARR;

    /* Calculate compare value from duty cycle percentage */
    uint32_t ccr = ((pwm_arr + 1U) * duty_pct) / 100U;

    status = tim_setTimerCompareChannelValue(PWM_TIM, PWM_CHANNEL, ccr);

    return status;
}

/**
 * @brief Enables the compare channel output and starts the timer.
 */
tim_status_t pwm_start(void)
{
    tim_status_t status;

    status = tim_enableTimerCompareChannel(PWM_TIM, PWM_CHANNEL);
    if (status != TIM_OK) return status;

    status = tim_enableTimer(PWM_TIM);

    return status;
}

/**
 * @brief Disables the compare channel output and stops the timer.
 */
tim_status_t pwm_stop(void)
{
    tim_status_t status;

    status = tim_disableTimerCompareChannel(PWM_TIM, PWM_CHANNEL);
    if (status != TIM_OK) return status;

    status = tim_disableTimer(PWM_TIM);

    return status;
}
