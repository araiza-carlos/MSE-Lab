
#include "led.h"
#include "button.h"
#include "timer.h"
#include "pwm.h"
#include "tim_driver.h"

int main(void)
{
  /* Tim 2 */
    tim_init();
    led_init();

    /* Tim 3 */
    timer_init();

    /* PWM , 50% duty cycle */
    pwm_init(1000);
    pwm_setSignal(1000, 50);
    pwm_start();

    /* Main loop */
    while (1)
    {
        led_toggle();
        timer_delay_ms(500);
    }
}