/**
 * @file    sensor.c
 * @brief   Analog Sensor Module Implementation
 *
 * Provides a high-level abstraction over the ADC driver to read
 * analog sensor values from a potentiometer connected to PA0
 * (ADC1 Channel 0) on the STM32F411RE Nucleo board.
 *
 * The module stores its configuration in a static local variable
 * after sensor_init() is called, so sensor_startConversion() and
 * sensor_readValue() require no parameters at the call site.
 *
 * @author  Kheara Kieley 
 * @version 1.0
 */

#include "sensor.h"

/* -----------------------------------------------------------------------
 * Private Variables
 * --------------------------------------------------------------------- */

/** Active sensor configuration — populated by sensor_init() */
static Sensor_Config_t s_sensorConfig;

/* -----------------------------------------------------------------------
 * Public Function Definitions
 * --------------------------------------------------------------------- */

Sensor_Status_t sensor_init(const Sensor_Config_t *config)
{
    if (config == NULL) { return SENSOR_INVALID; }

    /* Store configuration for use in subsequent calls */
    s_sensorConfig = *config;

    /* FR-1: Initialize ADC peripheral clock and reset registers */
    if (adc_init(config->adcInstance) != ADC_OK)
    {
        return SENSOR_ERROR;
    }

    /* FR-2: Power on the ADC (ADON = 1) */
    if (adc_enableAdc(config->adcInstance) != ADC_OK)
    {
        return SENSOR_ERROR;
    }

    /* FR-3: Configure regular channel with sampling time */
    if (adc_setChannel(config->adcInstance,
                       config->adcChannel,
                       config->sampleTime) != ADC_OK)
    {
        return SENSOR_ERROR;
    }

    return SENSOR_OK;
}

Sensor_Status_t sensor_startConversion(void)
{
    /* FR-5: Trigger a single conversion (sets SWSTART in CR2) */
    if (adc_startSingleConversion(s_sensorConfig.adcInstance) != ADC_OK)
    {
        return SENSOR_ERROR;
    }

    return SENSOR_OK;
}

Sensor_Status_t sensor_readValue(uint16_t *value)
{
    if (value == NULL) { return SENSOR_INVALID; }

    /* FR-8: Poll EOC flag and read 12-bit result from DR */
    if (adc_readData(s_sensorConfig.adcInstance, value) != ADC_OK)
    {
        return SENSOR_ERROR;
    }

    return SENSOR_OK;
}
