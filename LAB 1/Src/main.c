#include "led.h"
#include "button.h"

#define DEBOUNCE 3
int main(void)
{
    gpio_init();
    led_init();
    button_init();
    uint32_t actual =0;
    button_state_t btn;

    for( ;;)
    {
        button_get_state(&btn);
        if (btn == BUTTON_PRESSED)
        {
            if (actual++ == DEBOUNCE)
            {
                led_toggle();
            }
        }
        else
        {
            actual = 0;
        }
    }
}
