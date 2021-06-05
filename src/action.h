
#include "settings.h"

void init_vars(void);

void handle_input(void);
void update_window_animation(void);

void sprite_update_stars(void);

void sprite_update_gameboy(void);
void sprite_select_gameboy(uint8_t);

void sprite_handle_state_gameboy(void);

uint8_t rand_clamp(uint8_t min, uint8_t max);

uint8_t collision_test_win_gameboy(void);

extern uint8_t win_x;
extern uint8_t win_y;

extern uint8_t frame;

extern uint8_t stars_x[NUM_STARS];
extern uint8_t stars_y[NUM_STARS];
extern uint8_t stars_speed[NUM_STARS];