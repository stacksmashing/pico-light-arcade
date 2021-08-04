#pragma once

#include <stdint.h>
#include <stdbool.h>

enum button_event_type {
    BUTTON_NONE,
    BUTTON_RELEASED,
    BUTTON_PRESSED
};

typedef struct {
    enum button_event_type type;
    uint32_t button_number;
} button_event;

typedef struct {
    button_event events[21];
    uint32_t event_count;
} button_events;

typedef struct {
    bool last_state[21];
} _button_state;

void button_setup();
void button_poll(button_events *evs);
