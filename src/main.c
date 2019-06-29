#include <gb/gb.h>
#include "asm/types.h"
#include <rand.h>

#include "window_tiles.c"
// The following all use window_tiles:
#include "window_map.c"
#include "window_map_legs_1.c"
#include "window_map_legs_2.c"
#include "window_map_legs_3.c"
#include "bg_map.c"

#include "star_tiles.c"

#include "gameboy_tiles.c"

#define SPR_8x16_Y_OFFSET 16     // 8x16 sprite mode has Y offset by -16 pixels

// 8x8 Tiles per-(meta)-frame
#define META_TILE_SIZE           2 // 8x16 tile is 2 8x8 tiles
#define TILES_PER_FRAME_STARS    1
#define TILES_PER_FRAME_GB       8 // 16 (8x8)

// Tile frame count per sprite
#define TILE_FRAMES_STARS    4
#define TILE_FRAMES_GB_ROT   8
#define TILE_FRAMES_GB_EXP   4
#define TILE_FRAMES_GB_TOTAL (TILE_FRAMES_GB_ROT + TILE_FRAMES_GB_EXP)

#define TILE_COUNT_8x8_STARS  (TILE_FRAMES_STARS * TILES_PER_FRAME_STARS * META_TILE_SIZE)
#define TILE_COUNT_8x8_GB     (TILE_FRAMES_GB_TOTAL * TILES_PER_FRAME_GB * META_TILE_SIZE)

#define GB_SPRITES    8
#define GB_SPRITE_WIDTH  32
#define GB_SPRITE_HEIGHT 32
#define GB_Y_MAX         SCREEN_HEIGHT
#define GB_X_MIN         3
#define GB_GRAVITY       1
#define GB_TERM_VEL      1
typedef enum {
    GB_SPR_GAMEBOY,
    GB_SPR_EXPLODE
};

// Array of tile offsets that make up
const UINT8 gb_tile_id[] = {7 * TILES_PER_FRAME_GB * META_TILE_SIZE, // Rotation frames
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
const UINT8 gb_x_off[] = {0,8,0,8,16,24,16,24};
const UINT8 gb_y_off[] = {0,0,16,16,0,0,16,16};

UINT8 gb_x, gb_y;
 INT8 gb_speed_y;
UINT8 gb_frame, gb_frame_min, gb_frame_max;
UINT8 gb_state;
typedef enum {
    GB_STATE_RESPAWN,
    GB_STATE_READY,
    GB_STATE_EXPLODING,
};


UINT8 frame;
UINT8 vbl_count;
UINT8 c;

UINT8 keys;
UINT8 previous_keys;


UINT8 win_x;
UINT8 win_y;
 INT8 win_speed_y;
UINT8 win_anim_count;



#define NUM_STARS           20
#define STAR_SPEED_MIN       2
UINT8 stars_x[NUM_STARS];
UINT8 stars_y[NUM_STARS];
UINT8 stars_speed[NUM_STARS];

#define WIN_ANIM_SPEED    9 // Frames between updates
#define WIN_ANIM_1        WIN_ANIM_SPEED
#define WIN_ANIM_2        WIN_ANIM_SPEED * 2
#define WIN_ANIM_3        WIN_ANIM_SPEED * 3
#define WIN_TILE_HEIGHT_MAIN  6 // in 8x8 tiles
#define WIN_TILE_HEIGHT_LEGS  3 // in 8x8 tiles

#define WIN_SPRITE_WIDTH      128 //  15 * 8
#define WIN_SPRITE_HEIGHT      68 // 8.5 * 8
#define WIN_SPRITE_WIDTH_COLLIDE   112 //  128 - 16
#define WIN_SPRITE_HEIGHT_COLLIDE   48 // 8.5 * 8

#define SCREEN_WIDTH          160 //  20 * 8
#define SCREEN_HEIGHT         144 //  18 * 8

#define X_MIN 0 //  + 7
#define X_MAX SCREEN_WIDTH - WIN_SPRITE_WIDTH + 7
#define Y_MIN 0
#define Y_MAX SCREEN_HEIGHT - WIN_SPRITE_HEIGHT - 20 // Offset 20 px from bottom


#define WIN_SPEED_Y_MAX  -11
#define GRAVITY           1
#define TERM_VEL          8


#define UPDATE_KEYS() previous_keys = keys; keys = joypad()

#define KEY_PRESSED(K) (keys & (K))
#define KEY_TICKED(K)   ((keys & (K)) && !(previous_keys & (K)))
#define KEY_RELEASED(K) ((previous_keys & (K)) && !(keys & (K)))

#define ANY_KEY_PRESSED (keys)


void vbl_update(void);

void init_interrupts(void);
void init_gfx(void);
void init_sprites(void);
void init_vars(void);

void handle_input(void);
void update_window_animation(void);

void sprite_update_stars(void);

void sprite_update_gameboy(void);
void sprite_select_gameboy(UINT8);

void sprite_handle_state_gameboy(void);

UINT8 rand_clamp(UINT8 min, UINT8 max);

UINT8 collision_test_win_gameboy(void);


void vbl_update() {
    vbl_count ++;
}


void init_interrupts() {
    disable_interrupts();
    add_VBL(vbl_update);
    set_interrupts(VBL_IFLAG);
    enable_interrupts();
}


void init_gfx() {

    // Set Background / Window palette to
    // 3= 3(black),2= 1 (l.gray), 1= 0 (white),  0= 2 (d.gray) TRANSP w/ PRIOR
    // This is intended to mimic transparency via sprite priority under/overlaying window color Zero/0
    BGP_REG = 0xD2U; //0xC0 | 0x10 | 0x00 | 0x02;

    // Load tiles (background + window)
    set_bkg_data(0, 51, window_tiles);

    // Load background map
    set_bkg_tiles(0, 0, 20, 18, bg_map);
    SHOW_BKG;

    // Set window tiles
    set_win_tiles(0, 0, 20, 6, window_map);
    set_win_tiles(0, 6, 20, 3, window_map_legs_1);
    //win_x = X_MIN + 10;
    win_x = X_MIN;
    win_y = Y_MAX;
    WX_REG = win_x;
    WY_REG = win_y;
    SHOW_WIN;
}


void init_vars() {
    win_speed_y = 0;
    win_anim_count = 0;
}


UINT8 rand_clamp(UINT8 min, UINT8 max) {

    UINT8 r;

    r = rand();

    // This isn't the best way to do this (potentially slow, unpredictable)
    while ((r > max) || (r < min))
        r = rand();

    return (r);
}


void init_sprites() {

    // Mediocre-Initialize the random number generator
    initarand(DIV_REG);

    // Set Object palette to
    // 3= 0(white), 2= 3 (black), 1= 2 (d.gray), 0= 1 (l.gray) TRANSP
    OBP0_REG = 0x36U; //0x00 | 0x30 | 0x04 | 0x02;

    // palette for game boy
    //OBP1_REG = 0xE4U; //0xC0 | 0x20 | 0x04 | 0x00;
    OBP1_REG = 0xE0U; //0xC0 | 0x20 | 0x00 | 0x00;

    SPRITES_8x16;
    set_sprite_data(0, TILE_COUNT_8x8_STARS, star_tiles);

    for (c=0; c<NUM_STARS; c++) {
        // Use sprite PRIORITY flag to draw stars Behind WIN/BG
        // except when WIN/BG color=0
        set_sprite_prop(c, get_sprite_prop(c) | S_PRIORITY);

        // Alternate between small (1)/big star (0)
        set_sprite_tile(c, (c % 3) * META_TILE_SIZE);
        stars_speed[c] = (c % 3) + STAR_SPEED_MIN;

        // Set sprite to random location
        stars_x[c] = rand_clamp(0,255);
        stars_y[c] = rand_clamp(10,150);

        move_sprite(c,
                    stars_x[c],
                    stars_y[c]);
    }

    // Load Gameboy Sprite tiles
    // Load: Starting tile index, Num tiles, Var for tiles
    set_sprite_data(TILE_COUNT_8x8_STARS,
                    TILE_COUNT_8x8_GB,
                    gameboy_tiles);

    // Use palette #2 for all gameboy sprites
    for(c=0;c < GB_SPRITES; c++)
        set_sprite_prop(NUM_STARS + c, get_sprite_prop(NUM_STARS + c) | S_PALETTE);

    gb_state = GB_STATE_RESPAWN;
    sprite_handle_state_gameboy();

    SHOW_SPRITES;
}


void sprite_select_gameboy(UINT8 new_sprite) {

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


    // Handle movement
    gb_x--;
    // gb_y++;
    gb_y += gb_speed_y;
    /*
    gb_speed_y += GB_GRAVITY;
    if (gb_speed_y > GB_TERM_VEL) {
        gb_speed_y = GB_TERM_VEL;
    }
    gb_y = (UINT8)((INT8)gb_y + gb_speed_y);
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

    for (c=0; c<NUM_STARS; c++) {
        // Small stars(1) move faster than big stars (2)
        // stars_x[c] -= (c & 0x01) + STAR_SPEED_MIN;
        stars_x[c] -= stars_speed[c]; //(c % 3) + STAR_SPEED_MIN;
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

        win_y = (UINT8)((INT8)win_y + win_speed_y);
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


UINT8 collision_test_win_gameboy() {
    // Might have some 8-bit wraparound trouble here

    // Check for non-overlap
    if ((gb_x  > (win_x + WIN_SPRITE_WIDTH_COLLIDE))  || // GB > Right Edge
        (win_x > (gb_x + GB_SPRITE_WIDTH))       || // GB < Left  Edge
        (gb_y  > (win_y + WIN_SPRITE_HEIGHT_COLLIDE)) || // GB > Bottom Edge
        (win_y > (gb_y + GB_SPRITE_HEIGHT)))        // GB > Top Edge
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



void main() {

    frame = 0;

    init_gfx();
    init_sprites();
    init_vars();
    init_interrupts();

    while(1) {
        // Wait for vertical blank
        // (skip if already happened)
        if(!vbl_count)
            wait_vbl_done();
        vbl_count = 0;
        frame++;

        UPDATE_KEYS();
        handle_input();

        sprite_update_stars();
        sprite_update_gameboy();

        sprite_handle_state_gameboy();

        // update_window_animation();
    }

}
