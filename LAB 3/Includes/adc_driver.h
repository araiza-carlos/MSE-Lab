/**
 * @file    adc_driver.h
 * @brief   ADC Driver API for STM32F4xx
 *
 * Provides functions to initialize the ADC subsystem, configure
 * regular and injected channels, trigger conversions, and read
 * digital results from analog inputs on the STM32F411RE.
 *
 * Target: ADC1 on STM32F411RE — Nucleo-F411RE
 *
 * Pin mapping (from UM1724 user manual):
 *   PA0 → ADC1_IN0 → potentiometer wiper output
 *
 * @author  Carlos Humberto Araiza Quintana
 * @version 1.0
 */

#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include "stm32f4xx.h"

/* -----------------------------------------------------------------------
 * Type Definitions
 * --------------------------------------------------------------------- */

/**
 * @brief Return status codes for ADC driver functions.
 */
typedef enum {
    ADC_OK      = 0,   /**< Operation successful              */
    ADC_ERROR   = 1,   /**< Generic error                     */
    ADC_INVALID = 2    /**< Invalid parameter (instance/channel) */
} ADC_Status_t;

/**
 * @brief ADC peripheral instance identifiers.
 *
 * STM32F411RE has only one ADC peripheral (ADC1).
 */
typedef enum {
    ADC_INSTANCE_1 = 0   /**< ADC1 — only instance on STM32F411RE */
} ADC_Instance_t;

/**
 * @brief ADC sampling time options (cycles).
 *
 * Values map directly to SMPR register bit fields.
 * Higher cycle counts improve accuracy for high-impedance sources.
 */
typedef enum {
    ADC_SAMPLETIME_3CYCLES   = 0,   /**< 3 cycles   — fastest               */
    ADC_SAMPLETIME_15CYCLES  = 1,   /**< 15 cycles                           */
    ADC_SAMPLETIME_28CYCLES  = 2,   /**< 28 cycles                           */
    ADC_SAMPLETIME_56CYCLES  = 3,   /**< 56 cycles                           */
    ADC_SAMPLETIME_84CYCLES  = 4,   /**< 84 cycles  — recommended for pot.   */
    ADC_SAMPLETIME_112CYCLES = 5,   /**< 112 cycles                          */
    ADC_SAMPLETIME_144CYCLES = 6,   /**< 144 cycles                          */
    ADC_SAMPLETIME_480CYCLES = 7    /**< 480 cycles — slowest, most accurate */
} ADC_SampleTime_t;

/* -----------------------------------------------------------------------
 * Function Prototypes
 * --------------------------------------------------------------------- */

/**
 * @brief  Initializes the ADC subsystem (FR-1).
 *         Enables the ADC1 peripheral clock via RCC_APB2ENR,
 *         configures PA0 as analog input, and resets CR1/CR2
 *         to their default state.
 * @param  instance  ADC instance to initialize (ADC_INSTANCE_1).
 * @return ADC_OK on success, ADC_INVALID if instance is unknown.
 */
ADC_Status_t adc_init(ADC_Instance_t instance);

/**
 * @brief  Enables the ADC peripheral (FR-2).
 *         Sets the ADON bit in CR2, powering on the ADC hardware.
 * @param  instance  ADC instance to enable.
 * @return ADC_OK on success, ADC_INVALID if instance is unknown.
 */
ADC_Status_t adc_enableAdc(ADC_Instance_t instance);

/**
 * @brief  Configures a regular ADC channel (FR-3).
 *         Writes the channel to SQR3 (first in sequence) and sets
 *         the sampling time in SMPR1 or SMPR2 depending on channel number.
 * @param  instance    ADC instance.
 * @param  channel     Channel number (0–18).
 * @param  sampleTime  Sampling time (ADC_SampleTime_t).
 * @return ADC_OK on success, ADC_INVALID on bad parameters.
 */
ADC_Status_t adc_setChannel(ADC_Instance_t instance,
                             uint8_t channel,
                             ADC_SampleTime_t sampleTime);

/**
 * @brief  Configures an injected ADC channel (FR-4).
 *         Writes the channel to JSQR and sets the sampling time.
 * @param  instance    ADC instance.
 * @param  channel     Channel number (0–18).
 * @param  sampleTime  Sampling time (ADC_SampleTime_t).
 * @return ADC_OK on success, ADC_INVALID on bad parameters.
 */
ADC_Status_t adc_setInjectedChannel(ADC_Instance_t instance,
                                     uint8_t channel,
                                     ADC_SampleTime_t sampleTime);

/**
 * @brief  Triggers a single regular conversion (FR-5).
 *         Clears CONT and sets SWSTART in CR2.
 *         The ADC converts once and stops.
 * @param  instance  ADC instance.
 * @return ADC_OK on success, ADC_INVALID if instance is unknown.
 */
ADC_Status_t adc_startSingleConversion(ADC_Instance_t instance);

/**
 * @brief  Starts continuous regular conversions (FR-6).
 *         Sets CONT and SWSTART in CR2. The ADC converts
 *         repeatedly until stopped.
 * @param  instance  ADC instance.
 * @return ADC_OK on success, ADC_INVALID if instance is unknown.
 */
ADC_Status_t adc_startContinuousConversion(ADC_Instance_t instance);

/**
 * @brief  Triggers an injected channel conversion (FR-7).
 *         Sets JSWSTART in CR2.
 * @param  instance  ADC instance.
 * @return ADC_OK on success, ADC_INVALID if instance is unknown.
 */
ADC_Status_t adc_startInjectedConversion(ADC_Instance_t instance);

/**
 * @brief  Reads the result of the last regular conversion (FR-8).
 *         Polls the EOC flag in SR and reads the 12-bit result from DR.
 * @param  instance  ADC instance.
 * @param  data      Output pointer to store the result (0–4095).
 * @return ADC_OK on success, ADC_INVALID if instance unknown, ADC_ERROR if data is NULL.
 */
ADC_Status_t adc_readData(ADC_Instance_t instance,
                           uint16_t *data);

/**
 * @brief  Reads the result of the last injected channel conversion (FR-9).
 *         Polls the JEOC flag and reads from JDR1–JDR4 based on channel.
 * @param  instance  ADC instance.
 * @param  channel   Injected channel number (1–4).
 * @param  data      Output pointer to store the result (0–4095).
 * @return ADC_OK on success, ADC_INVALID on bad parameters, ADC_ERROR if data is NULL.
 */
ADC_Status_t adc_readInjectedChannelData(ADC_Instance_t instance,
                                          uint8_t channel,
                                          uint16_t *data);

#endif /* ADC_DRIVER_H */
