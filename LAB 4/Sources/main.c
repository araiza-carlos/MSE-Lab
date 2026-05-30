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
 * @file main.c
 * @brief Lab Assignment 04 - UART Driver and Serial Communication
 *
 * Application entry point. Reads a potentiometer value from ADC1 CH0 (PA0)
 * every 500 ms and sends telemetry over UART2 (115200 baud) using the
 * Serial module.
 *
 * Output format (VS Code Serial Monitor at 115200):
 *   ADC Value: 2048 | Voltage: 1650 mV
 *
 * Hardware (Nucleo-F411RE):
 *   PA0  -> ADC1_IN0   (potentiometer wiper)
 *   PA2  -> USART2_TX   (AF7, ST-Link VCP)
 *
 * @author  Kheara Kieley y Vania Leal
 * @date    05/06/2026
 */

/*** Includes ***/
#include "gpio_driver.h"
#include "adc_driver.h"
#include "tim_driver.h"
#include "sensor.h"
#include "serial.h"
#include "utils.h"

/*** Preprocessor Definitions ***/

/** Full-scale ADC reference voltage in mV (3.3 V) */
#define VREF_MV         3300U

/** 12-bit ADC full-scale value */
#define ADC_FULL_SCALE  4095U

/** Telemetry period in milliseconds */
#define PERIOD_MS       500U

/*** Function Definitions ***/

int main(void)
{
    /* ------------------------------------------------------------------ */
    /* 1. GPIO Initialization                                              */
    /* ------------------------------------------------------------------ */
    gpio_init();

    /* ------------------------------------------------------------------ */
    /* 2. Serial (UART) Initialization                                     */
    /* ------------------------------------------------------------------ */
    serial_init();

    /* ------------------------------------------------------------------ */
    /* 3. Sensor (ADC) Initialization                                      */
    /* ------------------------------------------------------------------ */
    Sensor_Config_t sensor_cfg = {
        .adcInstance = SENSOR_ADC_INSTANCE,
        .adcChannel  = SENSOR_ADC_CHANNEL,
        .sampleTime  = SENSOR_SAMPLE_TIME
    };
    sensor_init(&sensor_cfg);

    /* ------------------------------------------------------------------ */
    /* 4. Timer Initialization (TIM2 for 500 ms delay)                    */
    /* ------------------------------------------------------------------ */
    tim_init();
    tim_initTimer(TIM2);
    tim_setTimerMs(TIM2, PERIOD_MS);
    tim_enableTimer(TIM2);

    /* ------------------------------------------------------------------ */
    /* 5. Main Loop                                                        */
    /* ------------------------------------------------------------------ */
    while (1)
    {

        tim_waitTimer(TIM2);

        uint16_t adc_raw = 0U;
        sensor_startConversion();
        sensor_readValue(&adc_raw);

        
        uint32_t voltage_mV = ((uint32_t)adc_raw * VREF_MV) / ADC_FULL_SCALE;

       
        serial_printf("ADC Value: %d | Voltage: %d mV\n",
                      (int32_t)adc_raw,
                      (int32_t)voltage_mV);
    }


    return 0;
}
