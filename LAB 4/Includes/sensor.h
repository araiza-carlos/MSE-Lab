/******************************************************************************
* Copyright (C) 2026 by Carlos Villarreal - CETYS Universidad
*
* Redistribution, modification or use of this software in source or binary
* forms is permitted as long as the files maintain this copyright. Users are
* permitted to modify this and use it to learn about the field of embedded
* software. Carlos Villarreal and CETYS Universidad are not liable for any
* misuse of this material.
*
*****************************************************************************/
/**
* @file    sensor.h
* @brief   Analog Sensor Module - Header
*
* High-level abstraction for reading an analog sensor (potentiometer)
* using the ADC driver. Internally uses ADC1 Channel 0 on pin PA0.
*
* Pin mapping (Nucleo-F411RE):
*   PA0 → ADC1_IN0 → potentiometer wiper output
*
* @author  Carlos Humberto Araiza Quintana
* @date    04/29/2025
*/

#ifndef SENSOR_H
#define SENSOR_H

#include "adc_driver.h"

/* -----------------------------------------------------------------------
 * Hardware Mapping
 * --------------------------------------------------------------------- */
#define SENSOR_ADC_INSTANCE   ADC_INSTANCE_1          /**< STM32F411 has only ADC1    */
#define SENSOR_ADC_CHANNEL    0U                       /**< PA0 = ADC1 Channel 0       */
#define SENSOR_SAMPLE_TIME    ADC_SAMPLETIME_84CYCLES  /**< Stable for potentiometer   */
#define SENSOR_ADC_MAX_VALUE  4095U                    /**< 12-bit resolution max      */

/* -----------------------------------------------------------------------
 * Type Definitions
 * --------------------------------------------------------------------- */

/**
 * @brief Return status for sensor module functions.
 */
typedef enum {
    SENSOR_OK      = 0,   /**< Operation successful       */
    SENSOR_ERROR   = 1,   /**< General error              */
    SENSOR_INVALID = 2    /**< Invalid parameter or NULL  */
} Sensor_Status_t;

/**
 * @brief Configuration structure for the sensor module.
 */
typedef struct {
    ADC_Instance_t   adcInstance;   /**< ADC instance (ADC_INSTANCE_1)      */
    uint8_t          adcChannel;    /**< ADC channel (0 = PA0)              */
    ADC_SampleTime_t sampleTime;    /**< Sampling time for ADC conversion   */
} Sensor_Config_t;

/* -----------------------------------------------------------------------
 * Function Prototypes
 * --------------------------------------------------------------------- */

/**
 * @brief  Initializes the sensor module.
 *         Calls adc_init(), adc_enableAdc(), and adc_setChannel()
 *         internally using the provided configuration.
 * @param  config   Pointer to sensor configuration struct.
 * @return SENSOR_OK on success, SENSOR_INVALID if config is NULL.
 */
Sensor_Status_t sensor_init(const Sensor_Config_t *config);

/**
 * @brief  Triggers a single ADC conversion on the configured channel.
 *         Calls adc_startSingleConversion() internally.
 * @return SENSOR_OK on success.
 */
Sensor_Status_t sensor_startConversion(void);

/**
 * @brief  Reads the result of the last ADC conversion.
 *         Waits for EOC flag and returns the 12-bit digital value.
 *         Calls adc_readData() internally.
 * @param  value   Pointer to store the result (0 – 4095).
 * @return SENSOR_OK on success, SENSOR_INVALID if value is NULL.
 */
Sensor_Status_t sensor_readValue(uint16_t *value);

#endif /* SENSOR_H */