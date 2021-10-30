#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "tusb.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "led.h"
#include "button.h"

const uint32_t BUTTONS_START = 0;
const uint32_t BUTTONS_MODE = 1;
const uint32_t BUTTONS_PLAYER_1 = 2;
const uint32_t BUTTONS_PLAYER_2 = 11;

enum game_state {
    GAME_IDLE,
    GAME_ACTIVE,
    GAME_SIMON_ACTIVE,
    GAME_OVER
};

typedef struct {
    enum game_state state;
    uint64_t start_time;
    int player1_points;
    int player2_points;
} game;

// convinience
void leds_all(bool state) {
    for(int i=0; i < 21; i++) {
        led_set_state(i, state);
    }
}

uint64_t seconds_since_boot() {
    return time_us_64() / 500000;
}

void game_idle(game *game) {
    // Button state
    button_events events;
    unsigned mode = 0;

    // Flash all LEDs
    uint64_t last_seconds = seconds_since_boot();
    while(1) {
        // Blink LEDs
        uint64_t seconds = seconds_since_boot();
        if(seconds > last_seconds) {
            printf("IDLE BLABLABLA\n");
            if(seconds % 2) {
                if(mode == 0) {
                    led_set_state(2, 1);
                    led_set_state(3, 1);
                    led_set_state(4, 1);
                    led_set_state(5, 1);
                    led_set_state(6, 1);
                    led_set_state(BUTTONS_PLAYER_2+0, 1);
                    led_set_state(BUTTONS_PLAYER_2+1, 1);
                    led_set_state(BUTTONS_PLAYER_2+2, 1);
                    led_set_state(BUTTONS_PLAYER_2+3, 1);
                    led_set_state(BUTTONS_PLAYER_2+4, 1);
                } else if(mode == 1) {
                    led_set_state(7, 1);
                    led_set_state(8, 1);
                    led_set_state(9, 1);
                    led_set_state(BUTTONS_PLAYER_2+5, 1);
                    led_set_state(BUTTONS_PLAYER_2+6, 1);
                    led_set_state(BUTTONS_PLAYER_2+7, 1);
                }
                // led_set_state(0, 1);
                // leds_all(1);
            } else {
                // led_set_state(0, 0);
                leds_all(0);
            }
            last_seconds = seconds;
        }

        button_poll(&events);
        for(int i=0; i < events.event_count; i++) {
            button_event *ev = &events.events[i];
            if(ev->type == BUTTON_RELEASED) {
                if(ev->button_number == BUTTONS_MODE) {
                    // change mode
                    mode += 1;
                    if(mode > 1) {
                        mode = 0;
                    }
                } else if (ev->button_number == BUTTONS_START) {
                    if(mode == 0) {

                        printf("GAME_START 60\n");
                        game->state = GAME_ACTIVE;
                        return;
                    } else if(mode == 1) {
                        game->state = GAME_SIMON_ACTIVE;
                        return;
                    }
                }
            }
        }
    }
}


typedef struct {
    uint32_t button_base;
    bool leds[9];
    bool buttons[9];
    int points;
} player_state;

void player_init(player_state *state, uint32_t button_base) {
    state->button_base = button_base;
    for(int i=0; i < 9; i++) {
        state->leds[i] = 0;
        state->buttons[i] = 0;
    }
    state->points = 0;
}

bool player_is_button(player_state *state, uint32_t button) {
    if((state->button_base <= button) && (button < (state->button_base + 9))) {
        return true;
    }
    return false;
}

unsigned player_enabled_leds(player_state *state) {
    unsigned enabled_leds = 0;
    for(int i=0; i < 9; i++) {
        enabled_leds++;
    }
    return enabled_leds;
}

void player_set_tile(player_state *state, uint32_t tile) {
    state->leds[tile] = 1;
    led_set_state(state->button_base + tile, 1);
}
void player_clear_tile(player_state *state, uint32_t tile) {
    state->leds[tile] = 0;
    led_set_state(state->button_base + tile, 0);
}

void player_set_random_tile(player_state *state) {
    // check if all are on
    unsigned free_buttons = 0;
    unsigned enabled_leds = 0;
    for(int i=0; i < 9; i++) {
        if(!state->leds[i] && !state->buttons[i]) {
            free_buttons++;
        }
        if(state->leds[i]) {
            enabled_leds++;
        }
    }
    if(enabled_leds >= 5) {
        // printf("Enough LEDs on\n");
        return;
    }
    if(free_buttons == 0) {
        // printf("No free buttons.\n");
        return;
    }

    while(1) {
        unsigned r = rand() % 9;
        if(!state->leds[r]) {
            player_set_tile(state, r);
            // printf("Set tile: %d\n", r);
            return;
        }
    }
}

void player_clear_random_tile(player_state *state) {
    while(1) {
        unsigned r = rand() % 9;
        if(state->leds[r]) {
            player_clear_tile(state, r);
            return;
        }
    }
}


void player_handle_button(player_state *state, button_event *ev) {
    // Ignore button events that are not for us :)
    if(!player_is_button(state, ev->button_number)) return;

    unsigned local_button = ev->button_number - state->button_base;

    switch(ev->type) {
        case BUTTON_PRESSED:
            state->buttons[local_button] = 1;
            if(state->leds[local_button]) {
                state->points++;
                player_clear_tile(state, local_button);
            } else {
                state->points--;
            }
            printf("POINTS: %d\n", state->points);
            break;
        case BUTTON_RELEASED:
            state->buttons[local_button] = 0;
            player_set_random_tile(state);
            break;
    }
}


typedef struct {
    uint32_t button_base;
    uint32_t correct;
    bool done;
    bool mistake;
} simon_player;

void simon_player_init(simon_player *p, unsigned button_base) {
    p->button_base = button_base;
    p->correct = 0;
    p->done = false;
    p->mistake = false;
}

void simon_next_round(simon_player *p) {
    p->correct = 0;
    p->done = false;
    p->mistake = false;
}

bool simon_button_is_player(simon_player *p, unsigned button) {
    if((p->button_base <= button) && (button < p->button_base + 9)) {
        return true;
    }
    return false;
}

bool simon_check_button(simon_player *p, unsigned *leds, unsigned level, unsigned button) {
    if(p->correct == (level + 1)) {
        return true;
    }

    if(p->mistake) {
        return false;
    }

    if(leds[p->correct] == button - p->button_base) {
        p->correct++;
        if(p->correct == level+1) {
            p->done = true;
        }
        return true;
    } else {
        p->mistake = true;
    }

    return false;
}

void simon_player_blink(simon_player *p) {
    leds_all(false);
    for(int i=0; i < 6; i++) {
        for(int j=0; j < 9; j++) {
            led_set_state(p->button_base + j, 1);
        }
        sleep_ms(100);
        for(int j=0; j < 9; j++) {
            led_set_state(p->button_base + j, 0);
        }
        sleep_ms(100);
    }
}

void game_active_simon(game *game) {
    srand(time_us_32());
    unsigned level = 0;
    unsigned leds[30]= {0}; // max 30 levels

    simon_player p1, p2;
    simon_player_init(&p1, BUTTONS_PLAYER_1);
    simon_player_init(&p2, BUTTONS_PLAYER_2);
    leds_all(0);
    sleep_ms(500);
    while(1) {
        // generate new led
        leds[level] = rand() % 9; 

        // display for both players
        leds_all(0);
        for(int i = 0; i < level+1; i++) {
            unsigned led = leds[i];
            led_set_state(BUTTONS_PLAYER_1 + led, 1);
            led_set_state(BUTTONS_PLAYER_2 + led, 1);
            sleep_ms(250);
            led_set_state(BUTTONS_PLAYER_1 + led, 0);
            led_set_state(BUTTONS_PLAYER_2 + led, 0);
            sleep_ms(100);
        }

        // wait for players to enter :)
        bool round_finished = false;
        while(!round_finished) {
            button_events events;
            button_poll(&events);
            for(int i=0; i < events.event_count; i++) {
                button_event *ev = &events.events[i];
                if(ev->type == BUTTON_RELEASED) {
                    led_set_state(ev->button_number, 0);
                }
                if(ev->type == BUTTON_PRESSED) {
                    led_set_state(ev->button_number, 1);
                    if(simon_button_is_player(&p1, ev->button_number)) {
                        printf("Player 1 pressed %d\n", ev->button_number);
                        if(simon_check_button(&p1, leds, level, ev->button_number)) {
                            printf("Correct!\n");
                        } else {
                            printf("Wrong!\n");
                        }
                    }
                    if(simon_button_is_player(&p2, ev->button_number)) {
                        printf("Player 2 pressed %d\n", ev->button_number);
                        if(simon_check_button(&p2, leds, level, ev->button_number)) {
                            printf("Correct!\n");
                        } else {
                            printf("Wrong!\n");
                        }
                    }

                    if(p1.mistake) {
                        printf("Player 1 fucked up");
                        simon_player_blink(&p2);
                        game->state = GAME_IDLE;
                        return;
                    }
                    if(p2.mistake) {
                        printf("Player 2 fucked up");
                        simon_player_blink(&p1);
                        game->state = GAME_IDLE;
                        return;
                    }
                    if(p1.done) {
                        printf("Player 1 ok");
                    }
                    if(p2.done) {
                        printf("Player 2 ok");
                    }

                    if(p1.done && p2.done) {
                        printf("Both players done!");
                        leds_all(1);
                        sleep_ms(200);
                        leds_all(0);
                        sleep_ms(200);
                        simon_next_round(&p1);
                        simon_next_round(&p2);
                        level++;
                        round_finished = true;
                        break;
                    }
                    
                }
            }
        }
        

    }
}

void game_active(game *game) {
    srand(time_us_32());
    for(int i=0; i < 21; i++) {
        led_set_state(i, 0);
    }
    player_state player1, player2;
    player_init(&player1, BUTTONS_PLAYER_1);
    player_init(&player2, BUTTONS_PLAYER_2);

    // enable 3 tiles for each player

    player_set_random_tile(&player1);
    player_set_random_tile(&player1);
    player_set_random_tile(&player1);

    player_set_random_tile(&player2);
    player_set_random_tile(&player2);
    player_set_random_tile(&player2);

    game->start_time = time_us_64();

    uint64_t last_seconds = 0;
    uint64_t game_time = 30*1000000;

    uint64_t previous_time = game->start_time;
    while(1) {
        // check game time
        uint64_t current_time = time_us_64();
        uint64_t elapsed_time = current_time - game->start_time;
        

        if(elapsed_time > game_time) {
            printf("GAME_END\n");
            game->player1_points = player1.points;
            game->player2_points = player2.points;
            game->state = GAME_OVER;
            return;
        }

        // send updates every 100ms
        else if((elapsed_time - previous_time)/100000 > 0) {
            previous_time = elapsed_time;
            uint64_t remaining_time = (game_time - elapsed_time) / 1000000;
            printf("STATUS %llu %d %d\n", remaining_time, player1.points, player2.points);
        }

        uint64_t seconds = elapsed_time/1000000;
        if(seconds > last_seconds + 2) {
            if(player_enabled_leds(&player1) > 3) {
                player_clear_random_tile(&player1);
            }
            if(player_enabled_leds(&player2) > 3) {
                player_clear_random_tile(&player2);
            }
            
            player_set_random_tile(&player1);
            player_set_random_tile(&player2);
            
            last_seconds = seconds;
        }

        button_events events;

        button_poll(&events);
        for(int i=0; i < events.event_count; i++) {
            button_event *ev = &events.events[i];
            printf("Button event: %d - %d\n", ev->type, ev->button_number);
            if(player_is_button(&player1, ev->button_number)) {
                player_handle_button(&player1, ev);
            } else if(player_is_button(&player2, ev->button_number)) {
                player_handle_button(&player2, ev);
            }
        }
        sleep_ms(20);
    }
}

void game_over(game *game) {
    printf("GAME_OVER %d %d\n", game->player1_points, game->player2_points);
    uint32_t led_base = BUTTONS_PLAYER_1;
    if(game->player2_points > game->player1_points) {
        led_base = BUTTONS_PLAYER_2;
    }
    for(int i=0; i < 10; i++) {
        for(int j=0; j < 9; j++) {
            led_set_state(led_base + j, 1);
        }
        sleep_ms(300);
        for(int j=0; j < 9; j++) {
            led_set_state(led_base + j, 0);
        }
        sleep_ms(300);
    }
    game->state = GAME_IDLE;
    return;
}




int main() {
    stdio_init_all();
    led_setup();
    button_setup();
    multicore_reset_core1();
    multicore_launch_core1(led_task);

    printf("Arcade time!\n");

    game game;
    game.state = GAME_IDLE;

    button_events events;
    while(1) {
        switch(game.state) {
            case GAME_IDLE:
                game_idle(&game);
                break;
            case GAME_ACTIVE:
                game_active(&game);
                break;
            case GAME_SIMON_ACTIVE:
                game_active_simon(&game);
                break;
            case GAME_OVER:
                game_over(&game);
                break;
            default:
                printf("UNKNOWN STATE!\n");
                break;
        }

        sleep_ms(30);
    }
    
    return 0;
}

