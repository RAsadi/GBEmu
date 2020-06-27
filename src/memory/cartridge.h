//
// Created by Rayan Asadi on 2020-06-30.
//

#ifndef GBEMU_CARTRIDGE_H
#define GBEMU_CARTRIDGE_H

#include <string>
#include <vector>
#include <chrono>
#include "../utils/constants.h"
#include "../utils/enums.h"

enum RTC_REGISTER_TYPES {
    RTC_SECONDS = 0x08,
    RTC_MINUTES = 0x09,
    RTC_HOURS = 0x0A,
    RTC_DAYS = 0x0B,
    RTC_MISC = 0x0C
};

class Cartridge {
public:
    /*
     * Memory banking in the gameboy is handled one of three ways:
     * No memory banking (games like tetris)
     * MBC1 (used for majority of games)
     * MBC2
     *
     * We also need to keep track of which bank is loaded into memory at 0x4000 - 0x7FFF
     */
    BANK_MODE bank_mode;
    uint8_t curr_rom_bank;
    uint8_t curr_ram_bank;
    bool is_ram_enabled = false;
    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;

    // MBC3 has a Real Time Clock (RTC) built into it, this is for that
    bool ram_over_rtc = true;
    bool is_latched = false;
    bool is_latching = false;
    uint8_t clock_timer;
    RTC_REGISTER_TYPES rtc_mode;
    std::chrono::milliseconds start_time;
    uint8_t rtc_registers[5];
//public
    Cartridge();
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t byte);
    void load_rom(std::string file_path); // TODO: Make this take a buffer or array instead?
    BANK_MODE get_bank_mode();

    uint8_t get_rtc_register();
    void set_rtc_registers();
    void set_rtc_mode(uint8_t byte);

    void set_rtc_register(uint16_t addr, uint8_t byte);
};


#endif //GBEMU_CARTRIDGE_H
