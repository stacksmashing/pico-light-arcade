#include "button.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "hardware/gpio.h"


const uint32_t BUTTON_Y_BASE = 10;
const uint32_t BUTTON_X_BASE = 13;

_button_state button_state;

void button_setup() {
    memset(&button_state, 0, sizeof(button_state));

    for(int y=0; y < 3; y++) {
        gpio_init(BUTTON_Y_BASE + y);
        gpio_set_dir(BUTTON_Y_BASE + y, GPIO_OUT);
        gpio_pull_down(BUTTON_Y_BASE + y);
        gpio_put(BUTTON_Y_BASE + y, 0);
    }

    for(int x = 0; x < 7; x++) {
        gpio_init(BUTTON_X_BASE + x);
        gpio_set_dir(BUTTON_X_BASE + x, GPIO_IN);
        gpio_pull_down(BUTTON_X_BASE + x);
        
    }
}

void button_poll(button_events *evs) {
    // Ensure default type none
    evs->event_count = 0;

    bool state[21] = {0};
    unsigned btn_idx = 0;
    // Enable each row, then read 5 columns at once.
    for(int y = 0; y < 3; y++) {
        gpio_put(BUTTON_Y_BASE + y, 1);
        sleep_ms(1);
        state[btn_idx++] = gpio_get(BUTTON_X_BASE + 0);
        state[btn_idx++] = gpio_get(BUTTON_X_BASE + 1);
        state[btn_idx++] = gpio_get(BUTTON_X_BASE + 2);
        state[btn_idx++] = gpio_get(BUTTON_X_BASE + 3);
        state[btn_idx++] = gpio_get(BUTTON_X_BASE + 4);
        state[btn_idx++] = gpio_get(BUTTON_X_BASE + 5);
        state[btn_idx++] = gpio_get(BUTTON_X_BASE + 6);
        gpio_put(BUTTON_Y_BASE + y, 0);
    }

    // Create events
    for(int i=0; i < 21; i++) {
        if(button_state.last_state[i] == state[i]) {
            continue;
        } else if(button_state.last_state[i] && !state[i]) {
            evs->events[evs->event_count].type = BUTTON_RELEASED;
            evs->events[evs->event_count].button_number = i;
            evs->event_count++;
            // sleep_ms(10);
        } else if(!button_state.last_state[i] && state[i]) {
            evs->events[evs->event_count].type = BUTTON_PRESSED;
            evs->events[evs->event_count].button_number = i;
            evs->event_count++;
            // sleep_ms(10);
        }
        button_state.last_state[i] = state[i];
    }
}
