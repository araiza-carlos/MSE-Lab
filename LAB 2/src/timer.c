/**
 * @file    timer.c
 * @brief   Timer Module Implementation — Delay Generation
 *
 * Uses TIM2 via the TIM driver to generate accurate blocking delays.
 *
 * @author  Vania Leal
 * @version 1.0
 */

#include "timer.h"

tim_status_t timer_init(void)
{
    return tim_initTimer(TIMER_DELAY_TIM);
}

tim_status_t timer_delay_ms(uint32_t ms)
{
    tim_status_t status;

    if (ms == 0 || ms > 65535) return TIM_INVALID;

    status = tim_setTimerMs(TIMER_DELAY_TIM, ms);
    if (status != TIM_OK) return status;

    /* Start timer */
    status = tim_enableTimer(TIMER_DELAY_TIM);
    if (status != TIM_OK) return status;


    status = tim_waitTimer(TIMER_DELAY_TIM);
    if (status != TIM_OK) return status;

    /* Stop timer */
    status = tim_disableTimer(TIMER_DELAY_TIM);

    return status;
}
