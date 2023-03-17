#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "hm2-fw.h"


static uint const led_pin = PICO_DEFAULT_LED_PIN;
static uint32_t const * reg;


static void led_update(void) {
    gpio_put(led_pin, (*reg >> 31) & 0x1);
}


int led_init(void) {
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);

    gpio_put(led_pin, 1);
    sleep_ms(100);
    gpio_put(led_pin, 0);
    sleep_ms(100);
    gpio_put(led_pin, 1);
    sleep_ms(100);
    gpio_put(led_pin, 0);
    // sleep_ms(100);

    reg = (uint32_t const *)hm2_fw_register("led", 0x0200, 4, led_update);
    if (reg == NULL) {
        return -1;
    }
    return 0;
}
