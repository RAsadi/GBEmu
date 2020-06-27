//
// Created by Rayan Asadi on 2020-06-28.
//

#include "gpu.h"
#include "joypad.h"
#include "memory/mmu.h"
#include "interrupt_handler.h"
#include "utils/bit_utils.h"
#include "utils/constants.h"
#include "spdlog/spdlog.h"
#include <SDL.h>

// Addresses
const uint16_t TILE_SET_ZERO_ADDR = 0x8000;
const uint16_t TILE_SET_ONE_ADDR = 0x8800;
const uint16_t TILE_MAP_ZERO_ADDR = 0x9800;
const uint16_t TILE_MAP_ONE_ADDR = 0x9C00;
const uint16_t SPRITE_DATA_START = TILE_SET_ZERO_ADDR; // sprites are pulled from tile set zero
const uint16_t PALETTE_ZERO_ADDR = 0xFF47;
const uint16_t SPRITE_PALETTE_ZERO_ADDR = 0xFF48;
const uint16_t SPRITE_PALETTE_ONE_ADDR = 0xFF49;
const uint16_t OAM_ADDR = 0xFE00;
const uint16_t SCROLL_Y_REGISTER = 0xFF42;
const uint16_t SCROLL_X_REGISTER = 0xFF43;
const uint16_t WINDOW_Y_REGISTER = 0xFF4A;
const uint16_t WINDOW_X_REGISTER = 0xFF4B;

// Tile and sprite size information
const uint16_t BACKGROUND_PICTURE_SIZE = 256;
const uint8_t BACKGROUND_TILE_SIZE = 8;
const uint16_t TILE_MAP_SIZE = 32;
const uint8_t TILE_SIDE_LENGTH = 8;
const uint8_t TILE_SIZE = 16;
const uint8_t SPRITE_SIZE = 4;

// Timing information
const uint32_t CLOCKS_PER_SCANLINE = 456;
const uint32_t CLOCKS_PER_HBLANK = 204;
const uint32_t CLOCKS_PER_SCANLINE_OAM = 80;
const uint32_t CLOCKS_PER_SCANLINE_VRAM = 172;

GPU::GPU(MMU *mmu,
         InterruptHandler *interrupt_handler,
         Joypad *joypad) : pixels() {
    this->mmu = mmu;
    this->interrupt_handler = interrupt_handler;
    this->joypad = joypad;

    // Setup graphics using SDL
    const int scale = 4;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow(
            "Gameboy Emulator",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            VIEWPORT_WIDTH * scale,
            VIEWPORT_HEIGHT * scale,
            SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            VIEWPORT_WIDTH,
            VIEWPORT_HEIGHT
    );
}

void GPU::clock_step(uint8_t cycles) {
    spdlog::debug("gpu: {:d}, {:d}, {:d}", cycle_counter, curr_mode, curr_line);
    cycle_counter += cycles;
    /*
     * GPU cycles through a few modes over a full cycle to write a screen
     * First it enters the ACCESS_OAM mode, where it looks up the sprite table.
     * Next, it takes a bit of time in the ACCESS_VRAM mode, where it reads from VRAM
     * Then, it enters H_BLANK mode, where it writes one line onto the LCD
     * (similar to a cathode ray monitor, writing one line at a time, and jumping down to the start of the next line).
     * Once it reaches the final line in the list, the LCD needs to jump back to the top of list
     * As well, for our emulator, we are going to write the pixels to the screen at that point
     * As well as all the sprites on the screen. That order would be handled differently on a physical display,
     * but it should be good?
     */
    switch (curr_mode) {
        case ACCESS_OAM:
            if (cycle_counter >= CLOCKS_PER_SCANLINE_OAM) {
                cycle_counter = cycle_counter % CLOCKS_PER_SCANLINE_OAM;
                set_lcd_status(ACCESS_VRAM);
            }
            break;
        case ACCESS_VRAM:
            if (cycle_counter >= CLOCKS_PER_SCANLINE_VRAM) {
                cycle_counter = cycle_counter % CLOCKS_PER_SCANLINE_VRAM;
                set_lcd_status(H_BLANK);
            }
            break;
        case H_BLANK:
            if (cycle_counter >= CLOCKS_PER_HBLANK) {
                draw_scanline(curr_line);
                curr_line++;
                set_line(curr_line);
                cycle_counter = cycle_counter % CLOCKS_PER_HBLANK;

                if (curr_line == 144) {
                    set_lcd_status(V_BLANK);
                } else {
                    set_lcd_status(ACCESS_OAM);
                }
            }
            break;
        case V_BLANK:
            if (cycle_counter >= CLOCKS_PER_SCANLINE) {
                curr_line++;
                set_line(curr_line);
                cycle_counter = cycle_counter % CLOCKS_PER_SCANLINE;

                if (curr_line == 154) {
                    draw_sprites();
                    render();
                    set_line(0);
                    set_lcd_status(ACCESS_OAM);
                }
            }
            break;
    }
}

void GPU::set_line(uint8_t line_value) {
    mmu->unsafe_write_byte(SCANLINE_REGISTER, line_value);
    curr_line = line_value;
}

void GPU::draw_sprites() {
    if (!are_sprites_enabled())
        return;
    for (int i = 0; i < 40; i++) { // 40 sprites are stored in the gameboy memory
        draw_sprite(i);
    }
}

/*
 * draw_background takes in a line number, which is the horizontal line to draw
 *
 * In order to be able to draw a line, we need to go over each pixel of the line. For each pixel,
 * we need to find which tile it is part of in the 256x256 screen, find the ID of that tile,
 * fetch the tile, and get the color of specific pixel we are pointed at. Rince and repeat for
 * all of the pixels in the row
 */
void GPU::draw_background(uint32_t screen_y) {
    /*
     * For the background, there are two overlapping tile sets and 2 different tile map addresses to choose
     */
    uint16_t tile_set_addr = should_use_tile_set_zero() ? TILE_SET_ZERO_ADDR : TILE_SET_ONE_ADDR;
    uint16_t tile_map_addr = should_use_tile_map_zero() ? TILE_MAP_ZERO_ADDR : TILE_MAP_ONE_ADDR;

    for (uint32_t screen_x = 0; screen_x < VIEWPORT_WIDTH; screen_x++) {

        /*
         * Gameboy positions things on a larger screen, and scroll_x and scroll_y represent
         * the top left corner of the viewport to display
         */
        uint16_t scrolled_x = screen_x + get_scroll_x();
        uint16_t scrolled_y = screen_y + get_scroll_y();

        /*
         * Sometimes games roll of the screen, at that point we just map it to the opposite side of the
         * screen, and overall our background map will hold a total of 256 pixels
         */
        uint8_t background_map_x = scrolled_x % BACKGROUND_PICTURE_SIZE;
        uint8_t background_map_y = scrolled_y % BACKGROUND_PICTURE_SIZE;

        /* Each background tile is 8x8 in size, so to get the number of the tile to load, we
         * will int divide which will bring us the tile that the pixel is in.
         *
         * Basically, if we are anywhere between (0,0) and (7,7), all of those pixels are part
         * of the top right tile, or the tile with index (0,0)
         *
         * Also, in order get the color of that pixel, we need to find where in the tile we are,
         * which is simply handled with modulo
         */
        uint32_t tile_x = background_map_x / BACKGROUND_TILE_SIZE;
        uint32_t tile_y = background_map_y / BACKGROUND_TILE_SIZE;
        uint8_t tile_pixel_x = background_map_x % BACKGROUND_TILE_SIZE;
        uint8_t tile_pixel_y = background_map_y % BACKGROUND_TILE_SIZE;

        // Our index comes from the fact that background tiles are stored in rows of 32 bytes
        uint32_t background_tile_index = tile_y * TILE_MAP_SIZE + tile_x;
        uint16_t background_tile_addr = tile_map_addr + background_tile_index;
        uint8_t tile_id = mmu->read_byte(background_tile_addr);

        // Now that we have our tile_id, we can fetch it from memory and get the color we need

        // May need to treat as unsigned byte if using tile set one. This is because there is a section of
        // overlapped memory, which is indexed into from tile set one register using a signed int.
        // The magic number 128 comes from memory being stored in bytes, which have range -128 -> 127 for signed values
        uint32_t tile_offset = should_use_tile_set_zero() ? tile_id * TILE_SIZE :
                               ((static_cast<int8_t>(tile_id)) + 128) * TILE_SIZE;

        // 2x since each line requires 16 bits, not 8 like would be implied by the background tile size
        uint32_t line_offset = tile_pixel_y * 2;

        uint16_t tile_addr = tile_set_addr + tile_offset + line_offset;

        /* Tiles are stored in 2 bytes, which gives us 8 2-bit numbers for each pixel
         * In memory, these 2-bit numbers are stored with all 8 upper bits followed by 8 lower bits
         */
        uint8_t pixel1 = mmu->read_byte(tile_addr);
        uint8_t pixel2 = mmu->read_byte(tile_addr + 1);
        uint32_t color = get_real_color(get_palette_color(get_color(pixel1, pixel2, tile_pixel_x), PALETTE_ZERO_ADDR));
        set_pixels(screen_x, screen_y, color);
    }
}

/*
 * Principals for drawing the window are basically identical to the background drawing
 */
void GPU::draw_window(uint32_t screen_y) {
    uint16_t tile_set_addr = should_use_tile_set_zero() ? TILE_SET_ZERO_ADDR : TILE_SET_ONE_ADDR;
    uint16_t tile_map_addr = should_use_window_tile_map() ? TILE_MAP_ZERO_ADDR : TILE_MAP_ONE_ADDR;

    const uint32_t scrolled_y = screen_y - get_window_y();

    if (scrolled_y >= VIEWPORT_HEIGHT) {
        return;
    }

    // rendering method is similar to the BG, just with window x and y instead
    // To see how it works, see above
    for (uint32_t screen_x = 0; screen_x < VIEWPORT_WIDTH; screen_x++) {
        uint32_t scrolled_x = screen_x + get_window_x() - 7; // window x is always off by 7

        uint32_t tile_x = scrolled_x / BACKGROUND_TILE_SIZE;
        uint32_t tile_y = scrolled_y / BACKGROUND_TILE_SIZE;
        uint8_t tile_pixel_x = scrolled_x % BACKGROUND_TILE_SIZE;
        uint8_t tile_pixel_y = scrolled_y % BACKGROUND_TILE_SIZE;

        uint32_t background_tile_index = tile_y * TILE_MAP_SIZE + tile_x;
        uint16_t background_tile_addr = tile_map_addr + background_tile_index;

        uint8_t tile_id = mmu->read_byte(background_tile_addr);

        uint32_t tile_offset = should_use_tile_set_zero() ? tile_id * TILE_SIZE :
                               ((static_cast<int8_t>(tile_id)) + 128) * TILE_SIZE;
        uint32_t line_offset = tile_pixel_y * 2;

        uint16_t tile_addr = tile_set_addr + tile_offset + line_offset;
        uint8_t pixel1 = mmu->read_byte(tile_addr);
        uint8_t pixel2 = mmu->read_byte(tile_addr + 1);
        uint32_t color = get_real_color(get_palette_color(get_color(pixel1, pixel2, tile_pixel_x), PALETTE_ZERO_ADDR));
        set_pixels(screen_x, screen_y, color);
    }
}

void GPU::draw_sprite(uint32_t sprite_num) {
    // Each sprite takes up 4 bytes in the sprite attribute table (OAM)
    uint16_t oam_sprite_addr = OAM_ADDR + sprite_num * SPRITE_SIZE;
    uint8_t sprite_y = mmu->read_byte(oam_sprite_addr);
    uint8_t sprite_x = mmu->read_byte(oam_sprite_addr + 1);

    // TODO, for x values in this range, we should still change priority of drawing, see pan docs
    if (sprite_y == 0 || sprite_y >= 160 || sprite_x == 0 || sprite_x >= 168)
        return;

    uint8_t tile_number = mmu->read_byte(oam_sprite_addr + 2);
    uint8_t attributes = mmu->read_byte(oam_sprite_addr + 3);
    uint32_t sprite_size_multiplier = sprite_size() ? 2 : 1;
    // Control bits used for various ops on the write
    bool should_use_palette_one = get_bit(attributes, 4);
    bool flip_x = get_bit(attributes, 5);
    bool flip_y = get_bit(attributes, 6);
    bool obj_behind_bg = get_bit(attributes, 7);

    uint16_t tile_offset = tile_number * TILE_SIZE;
    uint16_t pattern_addr = SPRITE_DATA_START + tile_offset;
    uint16_t palette_addr = should_use_palette_one ? SPRITE_PALETTE_ONE_ADDR : SPRITE_PALETTE_ZERO_ADDR;

    // These magic numbers come from sprites being draw from the top left
    int start_y = sprite_y - 16;
    int start_x = sprite_x - 8;

    for (uint32_t y = 0; y < TILE_SIDE_LENGTH * sprite_size_multiplier; y++) {
        for (uint32_t x = 0; x < TILE_SIDE_LENGTH; x++) {
            uint32_t offset_y = !flip_y ? y : TILE_SIDE_LENGTH * sprite_size_multiplier - y - 1;
            uint32_t offset_x = !flip_x ? x : TILE_SIDE_LENGTH - x - 1;
            uint8_t color = get_color_from_addr(pattern_addr, offset_x, offset_y);
            if (color == 0) { // color 0 is transparent
                continue;
            }
            uint32_t real_color = get_real_color(get_palette_color(color, palette_addr));
            int screen_x = start_x + x;
            int screen_y = start_y + y;
            // sometimes sprites are drawn bit off screen, just ignore them for now to avoid
            // bad access exceptions.
            if (static_cast<uint8_t>(screen_x) >= VIEWPORT_WIDTH || static_cast<uint8_t>(screen_y) >= VIEWPORT_HEIGHT)
                continue;
            uint32_t existing_pixel = pixels[screen_x + screen_y * VIEWPORT_WIDTH];
            if (obj_behind_bg && existing_pixel != 0xFFFFFFFF) { // TODO change this from white to use palette
                continue;
            }
            set_pixels(screen_x, screen_y, real_color);
        }
    }
}

uint32_t GPU::get_color_from_addr(uint16_t addr, uint16_t offset_x, uint16_t offset_y) {
    uint8_t pixel1 = mmu->read_byte(addr + offset_y * 2);
    uint8_t pixel2 = mmu->read_byte(addr + offset_y * 2 + 1);
    return get_color(pixel1, pixel2, offset_x);
}

void GPU::set_pixels(uint32_t screen_x, uint32_t screen_y, uint32_t color) {
    pixels[screen_x + screen_y * VIEWPORT_WIDTH] = color;
}

uint32_t GPU::get_palette_color(uint32_t color, uint16_t palette_addr) {
    uint8_t hi, lo;
    uint8_t palette = mmu->read_byte(palette_addr);
    switch (color) {
        case 0:
            hi = 1;
            lo = 0;
            break;
        case 1:
            hi = 3;
            lo = 2;
            break;
        case 2:
            hi = 5;
            lo = 4;
            break;
        case 3:
            hi = 7;
            lo = 6;
            break;
        default:
            assert(false);
    }

    // use the palette to get the colour
    return get_bit(palette, hi) << 1 | get_bit(palette, lo);
}

uint8_t GPU::get_color(uint8_t pixel1, uint8_t pixel2, uint8_t pixel_index) {
    return (get_bit(pixel2, 7 - pixel_index) << 1) | get_bit(pixel1, 7 - pixel_index);
}

uint32_t GPU::get_real_color(uint8_t color) {
    // format is 0xFFRRGGBB
    switch (color) {
        case 0:
            return 0xFFFFFFFF;
        case 1:
            return 0xFFCCCCCC;
        case 2:
            return 0xFF777777;
        case 3:
            return 0xFF000000;
        default:
            assert(false);
    }
}

void GPU::set_lcd_status(LCD_MODE mode) {
    uint8_t curr_status = get_status();
    curr_mode = mode;
    // check for 6th bit coincidence interrupt
    bool scanline_coincidence_interrupt = get_bit(curr_status, 6);
    bool scanline_coincidence = get_scanline_compare() == curr_line;
    switch (mode) {
        case ACCESS_OAM:
            set_bit(curr_status, 1);
            unset_bit(curr_status, 0);
            break;
        case ACCESS_VRAM:
            set_bit(curr_status, 1);
            set_bit(curr_status, 0);
            break;
        case H_BLANK:
            if (get_bit(curr_status, 3)) { // should request lcd interrupt
                interrupt_handler->request_interrupt(LCD_INTERRUPT);
            }
            if (scanline_coincidence_interrupt && scanline_coincidence) {
                interrupt_handler->request_interrupt(LCD_INTERRUPT);
            }
            scanline_coincidence ? set_bit(curr_status, 2) : unset_bit(curr_status, 2);
            unset_bit(curr_status, 1);
            unset_bit(curr_status, 0);
            break;
        case V_BLANK:
            unset_bit(curr_status, 1);
            set_bit(curr_status, 0);
            interrupt_handler->request_interrupt(V_BLANK_INTERRUPT);
            break;
    }
    mmu->write_byte(LCD_STATUS_REGISTER, curr_status);
}

void GPU::draw_scanline(uint8_t line_num) {
    if (!is_lcd_enabled())
        return;
    if (is_bg_enabled())
        draw_background(line_num);
    if (is_window_enabled())
        draw_window(line_num);
}

uint8_t GPU::get_status() {
    return mmu->read_byte(LCD_STATUS_REGISTER);
}

uint8_t GPU::get_control() {
    return mmu->read_byte(LCD_CONTROL_REGISTER);
}

uint8_t GPU::get_scroll_y() {
    return mmu->read_byte(SCROLL_Y_REGISTER);
}

uint8_t GPU::get_scroll_x() {
    return mmu->read_byte(SCROLL_X_REGISTER);
}

uint8_t GPU::get_window_y() {
    return mmu->read_byte(WINDOW_Y_REGISTER);
}

uint8_t GPU::get_window_x() {
    return mmu->read_byte(WINDOW_X_REGISTER);
}

uint8_t GPU::get_scanline_compare() {
    return mmu->read_byte(SCANLINE_COMPARE_REGISTER);
}

bool GPU::is_lcd_enabled() {
    return get_bit(get_control(), 7);
}

bool GPU::should_use_window_tile_map() {
    return !get_bit(get_control(), 6);
}

bool GPU::is_window_enabled() {
    return get_bit(get_control(), 5);
}

bool GPU::should_use_tile_set_zero() {
    return get_bit(get_control(), 4);
}

bool GPU::should_use_tile_map_zero() {
    return !get_bit(get_control(), 3);
}

bool GPU::sprite_size() {
    return get_bit(get_control(), 2);
}

bool GPU::are_sprites_enabled() {
    return get_bit(get_control(), 1);
}

bool GPU::is_bg_enabled() {
    return get_bit(get_control(), 0);
}

void GPU::render() {
    process_events();
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, nullptr, pixels, VIEWPORT_WIDTH * sizeof(uint32_t));
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

/*
 * Keymap:
 * Used to map from computer keypresses into the keys array
 */
std::unordered_map<int, KEY> keymap = {
        {SDLK_a,      KEY_A},
        {SDLK_s,      KEY_B},
        {SDLK_RETURN, KEY_SELECT},
        {SDLK_SPACE,  KEY_START},
        {SDLK_UP,     KEY_UP},
        {SDLK_DOWN,   KEY_DOWN},
        {SDLK_LEFT,   KEY_LEFT},
        {SDLK_RIGHT,  KEY_RIGHT}
};

void GPU::process_events() {
    SDL_Event event;
    int key_type;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                exit(0);
            case SDL_KEYDOWN:
                key_type = event.key.keysym.sym;
                if (keymap.count(key_type)) {
                    joypad->press_key(keymap[key_type]);
                }
                break;
            case SDL_KEYUP:
                key_type = event.key.keysym.sym;
                if (keymap.count(key_type)) {
                    joypad->release_key(keymap[key_type]);
                }
                break;
        }
    }
}
