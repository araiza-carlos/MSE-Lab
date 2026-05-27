/**
 * @file    main.c
 * @brief   Lab Assignment 03 — ADC Driver and Sensor-Controlled PWM
 *
 * Reads a 10 kΩ potentiometer connected to PA0 (ADC1 Channel 0)
 * and maps the 12-bit ADC result to a PWM duty cycle on PA6
 * (TIM3 Channel 1, AF2) to control the brightness of an external LED.
 *
 * Initialization order matters: pwm_init() must be called before
 * sensor_init() so that the alternate function configuration for PA6
 * is not overwritten by the ADC driver's GPIOA clock enable.
 *
 * The compare register (CCR1) is updated directly via
 * tim_setTimerCompareChannelValue() rather than pwm_setSignal()
 * to avoid resetting ARR and CNT on every iteration.
 *
 * Hardware connections:
 *   PA0 (CN8 Pin 1 / A0)  ← Potentiometer wiper output (0–3.3 V)
 *   PA6 (CN5 Pin 5 / D12) → 220 Ω → External LED (+)
 *                           External LED (–) → GND
 *
 * @author  Carlos Humberto Araiza Quintana y Kheara Kieley
 * @version 1.0
 */

#include "sensor.h"
#include "pwm.h"
#include "tim_driver.h"

/* -----------------------------------------------------------------------
 * Private Definitions
 * --------------------------------------------------------------------- */

#define PWM_FREQUENCY_HZ   1000U   /**< PWM frequency in Hz (1 kHz)    */
#define ADC_MAX_VALUE      4095U   /**< 12-bit ADC full-scale value     */

/* -----------------------------------------------------------------------
 * Main Entry Point
 * --------------------------------------------------------------------- */

int main(void)
{
    uint16_t adcValue = 0U;
    uint32_t ccr      = 0U;

    /* --- 1. Initialize PWM on PA6 (TIM3_CH1, AF2) --- */
    pwm_init(PWM_FREQUENCY_HZ);
    pwm_start();

    /* --- 2. Initialize sensor on PA0 (ADC1 Channel 0) --- */
    Sensor_Config_t sensorConfig = {
        .adcInstance = SENSOR_ADC_INSTANCE,   /* ADC_INSTANCE_1       */
        .adcChannel  = SENSOR_ADC_CHANNEL,    /* 0 = PA0              */
        .sampleTime  = SENSOR_SAMPLE_TIME     /* 84 cycles            */
    };
    sensor_init(&sensorConfig);

    /* Cache ARR once — avoids reading the register on every iteration */
    uint32_t arr = PWM_TIM->ARR;

    /* --- Main loop: read sensor → update PWM duty cycle --- */
    while (1)
    {
        sensor_startConversion();

        if (sensor_readValue(&adcValue) == SENSOR_OK)
        {
            /* Map 12-bit ADC result to compare register value.
             * CCR = (ARR + 1) * adcValue / 4095
             * This updates duty cycle without resetting the timer. */
            ccr = ((arr + 1U) * adcValue) / ADC_MAX_VALUE;
            tim_setTimerCompareChannelValue(PWM_TIM, PWM_CHANNEL, ccr);
        }
    }
}
