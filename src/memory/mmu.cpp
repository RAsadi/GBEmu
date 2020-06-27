//
// Created by Rayan Asadi on 2020-06-28.
//

#include "mmu.h"
#include "../joypad.h"
#include "../timer.h"
#include "../cpu/cpu.h"
#include "../utils/constants.h"
#include "cartridge.h"
#include "spdlog/spdlog.h"

const uint16_t SPRITE_TABLE_START = 0xFE00; // -> 0xFE9F
const uint8_t SPRITE_RAM_LENGTH = 0xA0;

MMU::MMU(Cartridge *cartridge): memory() {
    this->cartridge = cartridge;
    is_booted = false;
    const uint8_t bootDMG[256] = {
            0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
            0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
            0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
            0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
            0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
            0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
            0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
            0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
            0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
            0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
            0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
            0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
            0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
            0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
            0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x00, 0x00, 0x23, 0x7D, 0xFE, 0x34, 0x20,
            0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x00, 0x00, 0x3E, 0x01, 0xE0, 0x50
    };
    memcpy(memory, bootDMG, 256);
}

uint8_t MMU::read_byte(uint16_t addr) {
    if (addr >= 0 && addr < 0x8000) { // Cartridge data
        if (addr < 0x100 && !is_booted)
            return memory[addr];
        return cartridge->read(addr);
    } else if (addr >= 0x8000 && addr < 0xA000) { // VRAM
        return memory[addr];
    } else if (addr >= 0xA000 && addr < 0xC000) { // Cartridge RAM
        return cartridge->read(addr);
    } else if (addr >= 0xc000 && addr < 0xDFFF) { // Internal RAM
        return memory[addr];
    } else if (addr >= 0xE000 && addr < 0xFE00) { // Mirrored RAM
        return memory[addr - 0x2000];
    } else if (addr >= 0xFE00 && addr < 0xFEA0) { // OAM / Sprite sheet
        return memory[addr];
    } else if (addr == JOYPAD_REGISTER) { // Joypad
        return joypad->get_state();
    } // TODO: Finish explicitly adding all the cases for easier debugging
    return memory[addr];
}

void MMU::write_byte(uint16_t addr, uint8_t byte) {
    if (addr != 0xFF04)
        spdlog::debug("Write {:02X}, {:02X}", addr, byte);
    if (!is_booted && addr == 0xFF50 && byte == 0x01) {
        is_booted = true;
    }
    //TODO: Fill out all cases explitcly
    if (addr >= 0 && addr < 0x8000) { // Cartridge memory
        cartridge->write(addr, byte);
        return;
    } else if (addr >= 0x8000 && addr < 0xA000) { // VRAM
        memory[addr] = byte;
        return;
    } else if (addr >= 0xA000 && addr < 0xC000) { // Cartridge RAM
        cartridge->write(addr, byte);
        return;
    } else if (addr >= 0xc000 && addr < 0xDFFF) { // Internal RAM
        memory[addr] = byte;
        return;
    } else if (addr >= 0xE000 && addr < 0xFE00) { // Mirrored RAM
        memory[addr - 0x2000] = byte;
        return;
    } else if (addr >= 0xFE00 && addr < 0xFEA0) { // OAM / Sprite sheet
        memory[addr] = byte;
        return;
    }else if (addr == DMA_REGISTER) { // DMA
        dma_transfer(byte);
        return;
    } else if (addr == SCANLINE_REGISTER) {
        memory[addr] = 0;
        return;
    } else if (addr == JOYPAD_REGISTER) {
        joypad->write(byte);
        return;
    } else if (addr == DIVIDER_REGISTER_ADDRESS) {
        memory[addr] = 0;
        return;
    } else if (addr == TIMER_CONTROLLER_ADDRESS) {
        memory[addr] = byte;
        timer->set_clock_frequency();
        return;
    }
    if (addr == 0xFFFF) {
//        spdlog::info("Write {:02X}, {:02X}", addr, byte);
    }
    memory[addr] = byte;
}

uint8_t MMU::unsafe_read_byte(uint16_t addr) {
    return memory[addr];
}

void MMU::unsafe_write_byte(uint16_t addr, uint8_t byte) {
    if (addr != 0xFF04)
        spdlog::debug("Write {:02X}, {:02X}", addr, byte);
    memory[addr] = byte;
}

void MMU::dma_transfer(uint8_t byte) {
    uint16_t addr = byte * 0x100;
    for (int i = 0; i < SPRITE_RAM_LENGTH; i++) {
        write_byte(SPRITE_TABLE_START + i, read_byte(addr + i));
    }
}

void MMU::set_joypad(Joypad *joypad) {
    this->joypad = joypad;
}

void MMU::set_cpu(CPU *cpu) {
    this->cpu = cpu;
}

void MMU::set_timer(Timer *timer) {
    this->timer = timer;
}

void MMU::push_word_to_stack(uint16_t word) {
    uint8_t hi = word >> 8;
    uint8_t lo = word & 0xFF;
    uint16_t sp = cpu->get_sp();
    sp--;
    write_byte(sp, hi);
    sp--;
    write_byte(sp, lo);
    cpu->set_sp(sp);
}

uint16_t MMU::pop_word_from_stack() {
    uint16_t sp = cpu->get_sp();
    uint16_t word = read_word(sp);
    sp += 2;
    cpu->set_sp(sp);
    return word;
}

uint16_t MMU::read_word(uint16_t addr) {
    uint16_t word = read_byte(addr + 1) << 8; // little endian
    word |= read_byte(addr);
    return word;
}

void MMU::write_word(uint16_t addr, uint16_t word) {
    write_byte(addr, word & 0xFF);
    write_byte(addr + 1, word >> 8);
}
