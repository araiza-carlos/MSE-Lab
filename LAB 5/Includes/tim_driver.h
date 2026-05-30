/******************************************************************************
 * Copyright (C) 2026 by Carlos Villarreal - CETYS Universidad
 *****************************************************************************/
/**
 * @file    tim_driver.h
 * @brief   General-purpose TIM driver for STM32F411RE
 *
 * Supports TIM2, TIM3, TIM4, TIM5 (APB1, 84 MHz timer clock).
 * Used internally by pwm.h to generate PWM signals.
 *
 * @author  Carlos Villarreal
 * @date    05/19/2026
 */

#ifndef __TIM_DRIVER_H__
#define __TIM_DRIVER_H__

#include <stdint.h>
#include "stm32f4xx.h"

/* -----------------------------------------------------------------------
 * Type Definitions
 * --------------------------------------------------------------------- */

typedef enum
{
    TIM_OK      = 0,
    TIM_ERROR,
    TIM_INVALID
} tim_status_t;

/** Capture/compare channel selector */
typedef enum
{
    TIM_CHANNEL_1 = 1U,
    TIM_CHANNEL_2 = 2U,
    TIM_CHANNEL_3 = 3U,
    TIM_CHANNEL_4 = 4U
} tim_channel_t;

/** Output compare mode */
typedef enum
{
    TIM_COMPARE_MODE_FROZEN   = 0U,  /**< No output                  */
    TIM_COMPARE_MODE_PWM1     = 6U,  /**< High while CNT < CCR       */
    TIM_COMPARE_MODE_PWM2     = 7U   /**< Low  while CNT < CCR       */
} tim_compare_mode_t;

/* -----------------------------------------------------------------------
 * Function Prototypes
 * --------------------------------------------------------------------- */

/**
 * @brief  Enables peripheral clock and resets a timer to default state.
 * @param  tim  Pointer to timer (TIM2 … TIM5).
 * @return TIM_OK or TIM_INVALID.
 */
tim_status_t tim_initTimer(TIM_TypeDef *tim);

/**
 * @brief  Configures PSC and ARR so the update event fires at freq_hz.
 *
 * Timer clock assumed = 84 MHz (APB1 × 2 on STM32F411RE).
 * ARR is set to give ~1000 counts per period for fine duty resolution.
 *
 * @param  tim      Pointer to timer.
 * @param  freq_hz  Desired frequency in Hz (1 … 1 000 000).
 * @return TIM_OK or TIM_INVALID.
 */
tim_status_t tim_setTimerFreq(TIM_TypeDef *tim, uint32_t freq_hz);

/**
 * @brief  Sets output-compare mode on a channel (PWM1, PWM2, etc.).
 * @param  tim      Pointer to timer.
 * @param  channel  TIM_CHANNEL_1 … TIM_CHANNEL_4.
 * @param  mode     tim_compare_mode_t value.
 * @return TIM_OK or TIM_INVALID.
 */
tim_status_t tim_setTimerCompareMode(TIM_TypeDef *tim,
                                     tim_channel_t channel,
                                     tim_compare_mode_t mode);

/**
 * @brief  Writes the capture/compare register (CCR) for a channel.
 * @param  tim      Pointer to timer.
 * @param  channel  TIM_CHANNEL_1 … TIM_CHANNEL_4.
 * @param  value    CCR value (0 … ARR).
 * @return TIM_OK or TIM_INVALID.
 */
tim_status_t tim_setTimerCompareChannelValue(TIM_TypeDef *tim,
                                             tim_channel_t channel,
                                             uint32_t value);

/**
 * @brief  Enables the output for a capture/compare channel (CCxE bit).
 * @param  tim      Pointer to timer.
 * @param  channel  TIM_CHANNEL_1 … TIM_CHANNEL_4.
 * @return TIM_OK or TIM_INVALID.
 */
tim_status_t tim_enableTimerCompareChannel(TIM_TypeDef *tim,
                                           tim_channel_t channel);

/**
 * @brief  Disables the output for a capture/compare channel.
 * @param  tim      Pointer to timer.
 * @param  channel  TIM_CHANNEL_1 … TIM_CHANNEL_4.
 * @return TIM_OK or TIM_INVALID.
 */
tim_status_t tim_disableTimerCompareChannel(TIM_TypeDef *tim,
                                            tim_channel_t channel);

/**
 * @brief  Starts the timer counter (sets CEN bit).
 * @param  tim  Pointer to timer.
 * @return TIM_OK or TIM_INVALID.
 */
tim_status_t tim_enableTimer(TIM_TypeDef *tim);

/**
 * @brief  Stops the timer counter (clears CEN bit).
 * @param  tim  Pointer to timer.
 * @return TIM_OK or TIM_INVALID.
 */
tim_status_t tim_disableTimer(TIM_TypeDef *tim);

#endif /* __TIM_DRIVER_H__ */