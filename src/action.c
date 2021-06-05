#include <gb/gb.h>
#include <rand.h>
#include <stdint.h>

#include "../res/window_map.h"
#include "../res/window_map_legs.h"

#include "../res/star_tiles.h"
#include "../res/gameboy_tiles.h"
#include "../res/window_tiles.h"

#include "settings.h"
#include "input.h"

#include "action.h"



// Array of tile offsets
const uint8_t gb_tile_id[] = {7 * TILES_PER_FRAME_GB * META_TILE_SIZE, // Rotation frames
                            6 * TILES_PER_FRAME_GB * META_TILE_SIZE,
                            5 * TILES_PER_FRAME_GB * META_TILE_SIZE,
                            4 * TILES_PER_FRAME_GB * META_TILE_SIZE,
                            3 * TILES_PER_FRAME_GB * META_TILE_SIZE,
                            2 * TILES_PER_FRAME_GB * META_TILE_SIZE,
                            1 * TILES_PER_FRAME_GB * META_TILE_SIZE,
                            0 * TILES_PER_FRAME_GB * META_TILE_SIZE,

                            8  * TILES_PER_FRAME_GB * META_TILE_SIZE, // Exploding frames
                            9  * TILES_PER_FRAME_GB * META_TILE_SIZE,
                            10 * TILES_PER_FRAME_GB * META_TILE_SIZE,
                            11 * TILES_PER_FRAME_GB * META_TILE_SIZE};
// x,y offsets for 8x16 sub-tiles within the 32x32 sprite
const uint8_t gb_x_off[] = {0,8,0,8,16,24,16,24};
const uint8_t gb_y_off[] = {0,0,16,16,0,0,16,16};

uint8_t gb_x = 0, gb_y = 0;
 int8_t gb_speed_y = 0;
uint8_t gb_frame = 0, gb_frame_min = 0, gb_frame_max = 0;
uint8_t gb_state;


uint8_t frame;

uint8_t keys = 0;
uint8_t previous_keys = 0;

uint8_t win_x;
uint8_t win_y;
 int8_t win_speed_y;
uint8_t win_anim_count;

uint8_t stars_x[NUM_STARS];
uint8_t stars_y[NUM_STARS];
uint8_t stars_speed[NUM_STARS];


uint8_t rand_clamp(uint8_t min, uint8_t max) {

    uint8_t r;
    r = ((uint8_t)rand() % (max - min + 1)) + min;

    return (r);
}


void init_vars() {

    // This could be moved to after the first user input
    // to make the seed actually random
    initarand(DIV_REG);

    uint8_t c;

    frame = 0;

    win_speed_y = 0;
    win_anim_count = 0;
    //win_x = X_MIN + 10;
    win_x = X_MIN;
    win_y = Y_MAX;

    gb_state = GB_STATE_RESPAWN;

    // Stars init
    for (c=0; c<NUM_STARS; c++) {
        // Alternate speeds
        stars_speed[c] = (c % 3) + STAR_SPEED_MIN;

        // Set star sprites to random X location
        stars_x[c] = rand_clamp(0U, STARS_Y_SCREEN_MAX + STARS_Y_RAND_MASK); // (0,STARS_Y_SCREEN_MAX) // (0, 255);

        // Evenly distribute stars along Y axis to avoid per-line sprite limit flickering
        stars_y[c] = c * ((165U) / NUM_STARS);
        // stars_y[c] = rand_clamp(10,150);
    }

}


void sprite_select_gameboy(uint8_t new_sprite) {

    // Select tile range
    if (new_sprite == GB_SPR_GAMEBOY) {
        gb_frame_min = 0;
        gb_frame_max = TILE_FRAMES_GB_ROT - 1;
    }
    else if (new_sprite == GB_SPR_EXPLODE) {
        gb_frame_min = TILE_FRAMES_GB_ROT;
        gb_frame_max = TILE_FRAMES_GB_ROT + TILE_FRAMES_GB_EXP - 1;
    }

    gb_frame = gb_frame_min;

    // Refresh the sprite
    sprite_update_gameboy();
}


void sprite_update_gameboy() {

    uint8_t c;

    // Handle movement
    gb_x--;
    // gb_y++;
    gb_y += gb_speed_y;
    /*
    gb_speed_y += GB_GRAVITY;
    if (gb_speed_y > GB_TERM_VEL) {
        gb_speed_y = GB_TERM_VEL;
    }
    gb_y = (uint8_t)((int8_t)gb_y + gb_speed_y);
    */

    // Handle animation (every N frames)
    //if ((frame & 0x7) == 0x04) {
    if ((frame & 0x7) == 0x02) {
        gb_frame++;
        if (gb_frame > gb_frame_max)
            gb_frame = gb_frame_min;

        for(c=0;c < GB_SPRITES; c++) {
            // Set sprite location
            move_sprite(NUM_STARS + c,
                        gb_x + gb_x_off[c],
                        gb_y + gb_y_off[c] + SPR_8x16_Y_OFFSET); // 8x16 sprite mode has Y offset by -16 pixels

            // Set sprite tile (GB tiles start after star tiles)
            set_sprite_tile(NUM_STARS + c, TILE_COUNT_8x8_STARS + gb_tile_id[gb_frame] + (c * 2));
        }
    }
}


void sprite_update_stars() {

    uint8_t c;

    for (c=0; c<NUM_STARS; c++) {
        // Small stars(1) move faster than big stars (2)
        // stars_x[c] -= (c & 0x01) + STAR_SPEED_MIN;
        stars_x[c] -= stars_speed[c]; //(c % 3) + STAR_SPEED_MIN;

        // Wrap around on X axis (with a small random component)
        if (stars_x[c] > (STARS_Y_SCREEN_MAX + STARS_Y_RAND_MASK))
            stars_x[c] = STARS_Y_SCREEN_MAX + (sys_time & STARS_Y_RAND_MASK);

        move_sprite(c,
                    stars_x[c],
                    stars_y[c]);
    }
}


void update_window_animation() {

    win_anim_count++;

    switch (win_anim_count) {
        // Animate the feet tiles in the window
        case WIN_ANIM_1: set_win_tiles( 0, WIN_TILE_HEIGHT_MAIN,
                                       20, WIN_TILE_HEIGHT_LEGS, window_map_legs_3);
                         break;

        case WIN_ANIM_2: set_win_tiles( 0, WIN_TILE_HEIGHT_MAIN,
                                       20, WIN_TILE_HEIGHT_LEGS, window_map_legs_2);
                         break;

        case WIN_ANIM_3: set_win_tiles( 0, WIN_TILE_HEIGHT_MAIN,
                                       20, WIN_TILE_HEIGHT_LEGS, window_map_legs_1);
                         // reset counter on last frame
                         win_anim_count = 0;
                         break;
    }
}


void handle_input() {

    UPDATE_KEYS();

    // Handle vertical (jump)
    // Only jump on ground
    if (KEY_TICKED(J_A) && (win_y == Y_MAX)) {
        win_speed_y = WIN_SPEED_Y_MAX;

        // Set legs to jumping animation frame
        set_win_tiles(0, 6, 20, 3, window_map_legs_2);
    }

    // If jumped, move window and process gravity
    if ((win_speed_y != 0) || (win_y != Y_MAX)) {

        win_speed_y += GRAVITY;
        if (win_speed_y > TERM_VEL) {
            win_speed_y = TERM_VEL;
        }

        win_y = (uint8_t)((int8_t)win_y + win_speed_y);
        if (win_y >= Y_MAX) {
            win_y = Y_MAX;
            win_speed_y = 0;

            // Set legs to landed animation frame
            set_win_tiles(0, 6, 20, 3, window_map_legs_1);
            //win_anim_count = 5;
        }
    }
    else
        // Only update leg animation when not jumping
        update_window_animation();


    // Handle horizontal left/right movement
    if (KEY_PRESSED(J_LEFT)) {
        if (win_x > X_MIN)
            win_x-=2;
    }
    else if (KEY_PRESSED(J_RIGHT)) {
        if (win_x < X_MAX)
            win_x+=2;
    }

    // Update window position
    WX_REG = win_x;
    WY_REG = win_y;
}


uint8_t collision_test_win_gameboy() {
    // Might have some 8-bit wraparound trouble here

    // Check for non-overlap
    if ((gb_x  > (win_x + WIN_SPRITE_WIDTH_COLLIDE))  || // GB > Right Edge
        (win_x > (gb_x + GB_SPRITE_WIDTH))       ||      // GB < Left  Edge
        (gb_y  > (win_y + WIN_SPRITE_HEIGHT_COLLIDE)) || // GB > Bottom Edge
        (win_y > (gb_y + GB_SPRITE_HEIGHT)))             // GB > Top Edge
        return FALSE;

    // Otherwise return true
    return TRUE;
}


void sprite_handle_state_gameboy() {

    switch (gb_state) {
        case GB_STATE_RESPAWN:
            gb_x = rand_clamp(160,170);
            gb_y = rand_clamp(0,70);
            // gb_y = rand_clamp(0,110);
            gb_speed_y = (DIV_REG & 0x02) >> 1; // either some or no gravity

            sprite_select_gameboy(GB_SPR_GAMEBOY);
            gb_state = GB_STATE_READY;
            break;

        case GB_STATE_READY:
            if (collision_test_win_gameboy()) {
                sprite_select_gameboy(GB_SPR_EXPLODE);
                gb_state = GB_STATE_EXPLODING;
            }
            else if ((gb_y > GB_Y_MAX) || (gb_x < GB_X_MIN)) {
                // Respawn if fell off screen
                gb_state = GB_STATE_RESPAWN;
            }

            break;

        case GB_STATE_EXPLODING:
                if (gb_frame == gb_frame_max)
                    gb_state = GB_STATE_RESPAWN;
            break;
    }
}
