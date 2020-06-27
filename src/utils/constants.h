//
// Created by Rayan Asadi on 2020-06-30.
//

#ifndef GBEMU_CONSTANTS_H
#define GBEMU_CONSTANTS_H

#include <cstdint>

/*
 * The gameboy refreshes the screen 60 times per second, and executes
 * 4194304 cycles per second.
 */
const int CLOCK_SPEED = 4194304; // Hz
const int REFRESH_RATE = 60; // Hz
const int CYCLES_PER_FRAME = CLOCK_SPEED / REFRESH_RATE;
//
//// Cartridge constants
//const uint32_t MAX_CARTRIDGE_SIZE = 0x200000;
//const uint32_t ROM_BANK_CARTRIDGE_ADDR = 0x148;
//
//
//const uint16_t TILE_DATA_MEMORY_START_UNSIGNED = 0x8000; // -> 0x8FFF
//const uint16_t TILE_DATA_MEMORY_START_SIGNED = 0x8800; // -> 0x97FF
//const uint16_t BACKGROUND_DATA_MEMORY_START_LOWER = 0x9800; // -> 0x9BFF
//const uint16_t BACKGROUND_DATA_MEMORY_START_UPPER = 0x9C00; // -> 9x9FFF
//
//const uint32_t SCANLINES_PER_FRAME = 144;
//const uint32_t CLOCKS_PER_FRAME = (CLOCKS_PER_SCANLINE * SCANLINES_PER_FRAME) + CLOCKS_PER_VBLANK;
//
//
//// bits 1 and 2 represent the LCD status
//// bit 3, 4, 5 are interrupt enabled flags
//// bit 6 is a flag to cause interrupt if bit 2 is also enabled
//
//
///* From pan docs
// * Bit 7 - LCD Display Enable (0=Off, 1=On)
// * Bit 6 - Window Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
// * Bit 5 - Window Display Enable (0=Off, 1=On)
// * Bit 4 - BG & Window Tile Data Select (0=8800-97FF, 1=8000-8FFF)
// * Bit 3 - BG Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
// * Bit 2 - OBJ (Sprite) Size (0=8x8, 1=8x16)
// * Bit 1 - OBJ (Sprite) Display Enable (0=Off, 1=On)
// * Bit 0 - BG Display (for CGB see below) (0=Off, 1=On)
// */
//
//// Memory constants
//const uint8_t MAX_RAM_BANKS = 4;
//const uint16_t RAM_BANK_SIZE = 0x2000;
//const uint16_t DMA_REGISTER = 0xFF46;
//
//// Interrupt constants
//
//// Joy stick constants


//// Address constants
const uint16_t JOYPAD_REGISTER = 0xFF00;
const uint16_t DIVIDER_REGISTER_ADDRESS = 0xFF04;
const uint16_t TIMER_ADDRESS = 0xFF05;
const uint16_t TIMER_MODULATOR_ADDRESS = 0xFF06;
const uint16_t TIMER_CONTROLLER_ADDRESS = 0xFF07;
const uint16_t LCD_CONTROL_REGISTER = 0xFF40;
const uint16_t LCD_STATUS_REGISTER = 0xFF41;
const uint16_t SCANLINE_REGISTER = 0xFF44;
const uint16_t SCANLINE_COMPARE_REGISTER = 0xFF45;
const uint16_t DMA_REGISTER = 0xFF46;
const uint16_t INTERRUPT_REQUEST_REGISTER = 0xFF0F;
const uint16_t INTERRUPT_ENABLED_REGISTER = 0xFFFF;
#endif //GBEMU_CONSTANTS_H
