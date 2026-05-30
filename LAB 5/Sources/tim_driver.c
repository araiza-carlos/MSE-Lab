/******************************************************************************
 * Copyright (C) 2026 by Carlos Villarreal - CETYS Universidad
 *****************************************************************************/
/**
 * @file    tim_driver.c
 * @brief   General-purpose TIM driver for STM32F411RE
 *
 * Supports TIM2–TIM5 on APB1 (timer clock = 84 MHz on STM32F411RE).
 *
 * @author  Carlos Villarreal
 * @date    05/19/2026
 */

#include "tim_driver.h"
#include <stddef.h>

/* Timer clock frequency (APB1 × 2 = 84 MHz on STM32F411RE) */
#define TIM_CLOCK_HZ    84000000UL

/* -----------------------------------------------------------------------
 * Private helper — enable APB1 clock for a given timer
 * --------------------------------------------------------------------- */
static tim_status_t prv_enable_clock(TIM_TypeDef *tim)
{
    if      (tim == TIM2) RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    else if (tim == TIM3) RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    else if (tim == TIM4) RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    else if (tim == TIM5) RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
    else return TIM_INVALID;
    return TIM_OK;
}

/* -----------------------------------------------------------------------
 * Public Functions
 * --------------------------------------------------------------------- */

tim_status_t tim_initTimer(TIM_TypeDef *tim)
{
    if (tim == NULL) return TIM_INVALID;

    if (prv_enable_clock(tim) != TIM_OK) return TIM_INVALID;

    /* Reset registers to a known state */
    tim->CR1   = 0U;
    tim->CR2   = 0U;
    tim->SMCR  = 0U;
    tim->DIER  = 0U;
    tim->CCMR1 = 0U;
    tim->CCMR2 = 0U;
    tim->CCER  = 0U;
    tim->PSC   = 0U;
    tim->ARR   = 0xFFFFU;
    tim->CCR1  = 0U;
    tim->CCR2  = 0U;
    tim->CCR3  = 0U;
    tim->CCR4  = 0U;

    /* Enable auto-reload preload */
    tim->CR1 |= TIM_CR1_ARPE;

    /* Generate update to load preload registers */
    tim->EGR = TIM_EGR_UG;
    tim->SR  = 0U;

    return TIM_OK;
}

tim_status_t tim_setTimerFreq(TIM_TypeDef *tim, uint32_t freq_hz)
{
    if (tim == NULL || freq_hz == 0U) return TIM_INVALID;

    /*
     * Strategy: keep ARR = 999 (1000 counts) for fine duty resolution.
     * PSC is chosen so that:
     *   timer_tick = TIM_CLOCK / (PSC + 1)
     *   period     = (ARR + 1) / timer_tick = (ARR+1)*(PSC+1) / TIM_CLOCK
     *   freq_hz    = TIM_CLOCK / ((PSC+1) * (ARR+1))
     *   PSC+1      = TIM_CLOCK / (freq_hz * (ARR+1))
     */
    const uint32_t ARR_VALUE = 999U;
    uint32_t psc = (TIM_CLOCK_HZ / (freq_hz * (ARR_VALUE + 1U)));

    if (psc == 0U) psc = 1U;   /* floor to 1 so PSC reg = psc-1 >= 0 */

    tim->PSC = psc - 1U;
    tim->ARR = ARR_VALUE;

    /* Force update to load new PSC/ARR immediately */
    tim->EGR = TIM_EGR_UG;
    tim->SR  = 0U;

    return TIM_OK;
}

tim_status_t tim_setTimerCompareMode(TIM_TypeDef *tim,
                                     tim_channel_t channel,
                                     tim_compare_mode_t mode)
{
    if (tim == NULL) return TIM_INVALID;

    uint32_t oc_mode = (uint32_t)mode;

    switch (channel)
    {
        case TIM_CHANNEL_1:
            tim->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_CC1S);
            tim->CCMR1 |=  (oc_mode << 4U);   /* OC1M bits [6:4] */
            tim->CCMR1 |=  TIM_CCMR1_OC1PE;   /* preload enable  */
            break;

        case TIM_CHANNEL_2:
            tim->CCMR1 &= ~(TIM_CCMR1_OC2M | TIM_CCMR1_CC2S);
            tim->CCMR1 |=  (oc_mode << 12U);
            tim->CCMR1 |=  TIM_CCMR1_OC2PE;
            break;

        case TIM_CHANNEL_3:
            tim->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_CC3S);
            tim->CCMR2 |=  (oc_mode << 4U);
            tim->CCMR2 |=  TIM_CCMR2_OC3PE;
            break;

        case TIM_CHANNEL_4:
            tim->CCMR2 &= ~(TIM_CCMR2_OC4M | TIM_CCMR2_CC4S);
            tim->CCMR2 |=  (oc_mode << 12U);
            tim->CCMR2 |=  TIM_CCMR2_OC4PE;
            break;

        default:
            return TIM_INVALID;
    }

    return TIM_OK;
}

tim_status_t tim_setTimerCompareChannelValue(TIM_TypeDef *tim,
                                             tim_channel_t channel,
                                             uint32_t value)
{
    if (tim == NULL) return TIM_INVALID;

    switch (channel)
    {
        case TIM_CHANNEL_1: tim->CCR1 = value; break;
        case TIM_CHANNEL_2: tim->CCR2 = value; break;
        case TIM_CHANNEL_3: tim->CCR3 = value; break;
        case TIM_CHANNEL_4: tim->CCR4 = value; break;
        default: return TIM_INVALID;
    }

    return TIM_OK;
}

tim_status_t tim_enableTimerCompareChannel(TIM_TypeDef *tim,
                                           tim_channel_t channel)
{
    if (tim == NULL) return TIM_INVALID;

    switch (channel)
    {
        case TIM_CHANNEL_1: tim->CCER |= TIM_CCER_CC1E; break;
        case TIM_CHANNEL_2: tim->CCER |= TIM_CCER_CC2E; break;
        case TIM_CHANNEL_3: tim->CCER |= TIM_CCER_CC3E; break;
        case TIM_CHANNEL_4: tim->CCER |= TIM_CCER_CC4E; break;
        default: return TIM_INVALID;
    }

    return TIM_OK;
}

tim_status_t tim_disableTimerCompareChannel(TIM_TypeDef *tim,
                                            tim_channel_t channel)
{
    if (tim == NULL) return TIM_INVALID;

    switch (channel)
    {
        case TIM_CHANNEL_1: tim->CCER &= ~TIM_CCER_CC1E; break;
        case TIM_CHANNEL_2: tim->CCER &= ~TIM_CCER_CC2E; break;
        case TIM_CHANNEL_3: tim->CCER &= ~TIM_CCER_CC3E; break;
        case TIM_CHANNEL_4: tim->CCER &= ~TIM_CCER_CC4E; break;
        default: return TIM_INVALID;
    }

    return TIM_OK;
}

tim_status_t tim_enableTimer(TIM_TypeDef *tim)
{
    if (tim == NULL) return TIM_INVALID;
    tim->CR1 |= TIM_CR1_CEN;
    return TIM_OK;
}

tim_status_t tim_disableTimer(TIM_TypeDef *tim)
{
    if (tim == NULL) return TIM_INVALID;
    tim->CR1 &= ~TIM_CR1_CEN;
    return TIM_OK;
}