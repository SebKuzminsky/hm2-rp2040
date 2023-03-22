#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "hm2-fw.h"


static uint const led_pin = PICO_DEFAULT_LED_PIN;
static uint32_t const * reg;


static void led_update(void) {
    gpio_put(led_pin, (*reg >> 31) & 0x1);
}


static void led_setup_once(void) {
    static bool initialized = false;
    if (!initialized) {
        gpio_init(led_pin);
        gpio_set_dir(led_pin, GPIO_OUT);
        initialized = true;
    }
}


// This is a helper function to blink the LED, to show that the hostmot2
// firmware is booting.
void led_blink(uint8_t const num_blinks, uint16_t const ms_delay) {
    led_setup_once();

    for (uint8_t i = 0; i < num_blinks; ++i) {
        gpio_put(led_pin, 1);
        sleep_ms(ms_delay);
        gpio_put(led_pin, 0);
        sleep_ms(ms_delay);
    }
}


int led_init(void) {
    led_setup_once();

    led_blink(2, 100);

    reg = (uint32_t const *)hm2_fw_register("led", 0x0200, 4, led_update);
    if (reg == NULL) {
        return -1;
    }
    return 0;
}
