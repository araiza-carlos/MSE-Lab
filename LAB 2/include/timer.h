/**
 * @file    timer.h
 * @brief   Timer Module API — Delay Generation
 *
 * High-level abstraction for generating blocking delays using a
 * hardware timer. Internally uses the TIM driver.
 *
 * Target timer: TIM2 (APB1, general purpose, available on F411RE)
 *
 * @author  Carlos Cayetano
 * @version 1.0
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "tim_driver.h"

/* -----------------------------------------------------------------------
 * Hardware Mapping
 * --------------------------------------------------------------------- */
#define TIMER_DELAY_TIM     TIM2    /**< Timer used for delay generation */

/* -----------------------------------------------------------------------
 * Function Prototypes
 * --------------------------------------------------------------------- */

/**
 * @brief  Initializes the delay timer peripheral.
 *         Enables TIM2 clock and resets it to a known state.
 *         Must be called once before any timer_delay_ms() call.
 * @return TIM_OK on success.
 */
tim_status_t timer_init(void);

/**
 * @brief  Blocks execution for the specified number of milliseconds.
 *         Configures TIM2, starts it, waits for the update event,
 *         then stops the timer.
 * @param  ms  Duration in milliseconds (1 … 65535).
 * @return TIM_OK on success, TIM_INVALID if ms is out of range.
 */
tim_status_t timer_delay_ms(uint32_t ms);

#endif /* TIMER_H */
