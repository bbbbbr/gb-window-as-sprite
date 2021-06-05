#include <gb/gb.h>
#include <stdint.h>
#include "asm/types.h"
#include <rand.h>

#include "../res/bg_map.h"

#include "../res/window_map.h"
#include "../res/window_map_legs.h"

#include "../res/star_tiles.h"
#include "../res/gameboy_tiles.h"
#include "../res/window_tiles.h"

#include "action.h"
#include "settings.h"



void init_gfx(void);
void init_sprites(void);


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

    WX_REG = win_x;
    WY_REG = win_y;
    SHOW_WIN;
}


void init_sprites() {

    uint8_t c;

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

    sprite_handle_state_gameboy();

    SHOW_SPRITES;
}



void main() {

    init_vars();
    init_gfx();
    init_sprites();


    while(1) {

        wait_vbl_done();
        frame++;

        handle_input();

        sprite_update_stars();
        sprite_update_gameboy();

        sprite_handle_state_gameboy();

        // update_window_animation();
    }

}
