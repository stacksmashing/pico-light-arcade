#include "led.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "hardware/gpio.h"

const uint32_t LED_Y_BASE = 0;
const uint32_t LED_X_BASE = 3;

bool led_state[21] = {0};
const uint32_t led_count = 20;

void led_set_state(unsigned led_num, bool state) {
    if(led_num > led_count) {
        return;
    }

    led_state[led_num] = state;
}

void led_setup() {
    for(int i=0; i < 7; i++) {
        gpio_init(LED_X_BASE + i);
        gpio_set_dir(LED_X_BASE + i, GPIO_OUT);
        gpio_put(LED_X_BASE + i, 1);
    }
    for(int i=0; i < 3; i++) {
        gpio_init(LED_Y_BASE + i);
        gpio_set_dir(LED_Y_BASE + i, GPIO_OUT);
        gpio_put(LED_Y_BASE + i, 0);
    }


}

void led_task(void) {
    while(1) {
    for(uint32_t y = 0; y < 3; y++) {
        // Continue if no LED is on here.
        uint32_t led_idx = y * 7;
        if(!led_state[led_idx++] && 
            !led_state[led_idx++] &&
            !led_state[led_idx++] &&
            !led_state[led_idx++] &&
            !led_state[led_idx++] &&
            !led_state[led_idx++] &&
            !led_state[led_idx++]) {
            continue;
        }

        // output state of each LED in row
        uint32_t x_idx = 0;

        led_idx = y * 7;

        // printf("LED IDX: %d %d\n", led_idx, led_state[led_idx]);
        // printf("IO PUT %d %d\n", LED_X_BASE + x_idx, !led_state[led_idx]);
        gpio_put(LED_X_BASE + x_idx++, !led_state[led_idx++]);
        gpio_put(LED_X_BASE + x_idx++, !led_state[led_idx++]);
        gpio_put(LED_X_BASE + x_idx++, !led_state[led_idx++]);
        gpio_put(LED_X_BASE + x_idx++, !led_state[led_idx++]);
        gpio_put(LED_X_BASE + x_idx++, !led_state[led_idx++]);
        gpio_put(LED_X_BASE + x_idx++, !led_state[led_idx++]);
        gpio_put(LED_X_BASE + x_idx++, !led_state[led_idx++]);

        // enable output row
        gpio_put(LED_Y_BASE + y, 1);
        sleep_ms(3);

        // disable output row
        gpio_put(LED_Y_BASE + y, 0);

        // disable columns
        x_idx = 0;
    }
        
    }
}

