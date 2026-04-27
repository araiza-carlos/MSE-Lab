/**
 * @file    main.c
 * @brief   Lab Assignment 02 - Application Integration
 *
 * Demonstrates both modules working simultaneously:
 *   - TIM2: blinks the LED every 500 ms using timer_delay_ms()
 *   - TIM3: generates PWM on PA6 at 1 kHz with 50% duty cycle
 *
 *  @author  Carlos Humberto Araiza Quintana
 * @version 1.0
 */

#include "led.h"
#include "button.h"
#include "timer.h"
#include "pwm.h"
#include "tim_driver.h"

int main(void)
{
    /* Initialize TIM subsystem */
    tim_init();

    /* Initialize LED on PA5 */
    led_init();

    /* Initialize delay timer (TIM2) */
    timer_init();

    /* Initialize PWM on PA6 at 1 kHz, 50% duty cycle */
    pwm_init(1000);
    pwm_setSignal(1000, 50);
    pwm_start();

    /* Main loop: blink LED every 500 ms */
    while (1)
    {
        led_toggle();
        timer_delay_ms(500);
    }
}