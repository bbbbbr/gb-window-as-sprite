
#ifndef _SETTINGS_H
#define _SETTINGS_H


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

typedef enum {
    GB_STATE_RESPAWN,
    GB_STATE_READY,
    GB_STATE_EXPLODING,
};

#define NUM_STARS           16U // 20U
#define STAR_SPEED_MIN       2U
#define STARS_Y_SCREEN_MAX   160U
#define STARS_Y_RAND_MASK    0x1FU

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

#endif // _SETTINGS_H