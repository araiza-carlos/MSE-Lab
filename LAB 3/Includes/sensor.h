/**
 * @file    sensor.h
 * @brief   Analog Sensor Module API
 *
 * High-level abstraction for reading an analog sensor (potentiometer)
 * using the ADC driver. Internally uses ADC1 Channel 0 on pin PA0.
 * Provides a simplified three-function interface for application code.
 *
 * Target: ADC1_IN0 on PA0 (CN8 Pin 1 / Arduino A0) — Nucleo-F411RE
 *
 * Pin mapping (from UM1724 user manual):
 *   PA0 → ADC1_IN0 → potentiometer wiper output
 *
 * @author  Kheara Kieley
 * @version 1.0
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
 * @brief Return status codes for sensor module functions.
 */
typedef enum {
    SENSOR_OK      = 0,   /**< Operation successful              */
    SENSOR_ERROR   = 1,   /**< Generic error                     */
    SENSOR_INVALID = 2    /**< Invalid parameter or NULL pointer */
} Sensor_Status_t;

/**
 * @brief Configuration structure for the sensor module.
 */
typedef struct {
    ADC_Instance_t   adcInstance;   /**< ADC instance (ADC_INSTANCE_1)    */
    uint8_t          adcChannel;    /**< ADC channel number (0 = PA0)     */
    ADC_SampleTime_t sampleTime;    /**< Sampling time for ADC conversion */
} Sensor_Config_t;

/* -----------------------------------------------------------------------
 * Function Prototypes
 * --------------------------------------------------------------------- */

/**
 * @brief  Initializes the sensor module.
 *         Calls adc_init(), adc_enableAdc(), and adc_setChannel()
 *         internally using the provided configuration. Stores the
 *         configuration in a static local variable for later use.
 * @param  config  Pointer to sensor configuration struct.
 * @return SENSOR_OK on success, SENSOR_INVALID if config is NULL.
 */
Sensor_Status_t sensor_init(const Sensor_Config_t *config);

/**
 * @brief  Triggers a single ADC conversion on the configured channel.
 *         Calls adc_startSingleConversion() internally (sets SWSTART).
 *         Call sensor_readValue() after this to obtain the result.
 * @return SENSOR_OK on success, SENSOR_ERROR if the ADC call fails.
 */
Sensor_Status_t sensor_startConversion(void);

/**
 * @brief  Reads the result of the last ADC conversion.
 *         Polls the EOC flag and returns the 12-bit digital value
 *         by calling adc_readData() internally.
 * @param  value  Output pointer to store the result (0–4095).
 * @return SENSOR_OK on success, SENSOR_INVALID if value is NULL,
 *         SENSOR_ERROR if the ADC read fails.
 */
Sensor_Status_t sensor_readValue(uint16_t *value);

#endif /* SENSOR_H */
