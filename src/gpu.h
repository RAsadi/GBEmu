//
// Created by Rayan Asadi on 2020-06-28.
//

#ifndef GBEMU_GPU_H
#define GBEMU_GPU_H

#include <cstdint>
#include "utils/enums.h"

class MMU;
class InterruptHandler;
class Joypad;
class SDL_Renderer;
class SDL_Texture;

// Physical size of gameboy screen
inline const uint8_t VIEWPORT_WIDTH = 160;
inline const uint8_t VIEWPORT_HEIGHT = 144;
inline const uint16_t REAL_HEIGHT = 256;
inline const uint16_t REAL_WIDTH = 256;

class GPU {
    MMU *mmu;
    InterruptHandler *interrupt_handler;
    Joypad *joypad;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    uint32_t cycle_counter = 0;
    uint8_t curr_line = 0;
    LCD_MODE curr_mode = ACCESS_OAM;
    uint32_t pixels[VIEWPORT_WIDTH * VIEWPORT_HEIGHT];


    // Helpers to read and write from LCD control register
    bool is_lcd_enabled();
    bool should_use_window_tile_map();
    bool is_window_enabled();
    bool should_use_tile_map_zero();
    bool should_use_tile_set_zero();
    bool sprite_size();
    bool are_sprites_enabled();
    bool is_bg_enabled();

    // LCD status had some weird management to be done
    void set_lcd_status(LCD_MODE mode);

    // Helpers to fetch/set different graphics data from mmu
    uint8_t get_status();
    uint8_t get_control();
    uint8_t get_scroll_y();
    uint8_t get_scroll_x();
    uint8_t get_scanline_compare();
    uint8_t get_window_y();
    uint8_t get_window_x();
    void set_line(uint8_t line_value);

    // Helpers to deal with pixel array and colors
    static uint8_t get_color(uint8_t pixel1, uint8_t pixel2, uint8_t pixel_index);
    void set_pixels(uint32_t screen_x, uint32_t screen_y, uint32_t color);
    uint32_t get_color_from_addr(uint16_t addr, uint16_t offset_x, uint16_t offset_y);
    uint32_t get_palette_color(uint32_t color, uint16_t palette_addr);
    static uint32_t get_real_color(uint8_t color);

    // Helpers to write to pixel array
    void draw_scanline(uint8_t screen_y);
    void draw_sprites();
    void draw_sprite(uint32_t sprite_num);
    void draw_background(uint32_t screen_y);
    void draw_window(uint32_t screen_y);

    // Actual rendering
    void process_events();
    void render();

public:
    GPU(MMU *mmu, InterruptHandler *interrupt_handler, Joypad *joypad);
    void clock_step(uint8_t cycles);
};


#endif //GBEMU_GPU_H
