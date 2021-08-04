#pragma once

#include <stdbool.h>

void led_set_state(unsigned led_num, bool state);
void led_setup();
void led_task(void);
