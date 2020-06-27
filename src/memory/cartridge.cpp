//
// Created by Rayan Asadi on 2020-06-30.
//
#include <fstream>
#include <string>
#include "cartridge.h"
#include "spdlog/spdlog.h"

std::chrono::milliseconds now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

Cartridge::Cartridge() {
    curr_rom_bank = 0;
    curr_ram_bank = 0;
    start_time = now();
    memset(rtc_registers, 0, 5);
}

void Cartridge::load_rom(std::string file_path) {
    // Shamelessly taken from
    // https://stackoverflow.com/questions/15138353/how-to-read-a-binary-file-into-a-vector-of-unsigned-chars
    std::ifstream file_stream(file_path.c_str(), std::ios::in | std::ios::binary);
    file_stream.unsetf(std::ios::skipws);

    // get its size:
    std::streampos file_size;

    file_stream.seekg(0, std::ios::end);
    file_size = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);


    rom.reserve(file_size);
    rom.insert(rom.begin(), std::istream_iterator<uint8_t>(file_stream), std::istream_iterator<uint8_t>());

    // Initialize RAM
    uint8_t ram_size = rom.at(0x148);
    uint real_ram_size;
    switch (ram_size) {
        case 0x00:
            real_ram_size = 0;
            break;
        case 0x01:
            real_ram_size = 0x800;
            break;
        case 0x02:
            real_ram_size = 0x2000;
            break;
        case 0x03:
            real_ram_size = 0x8000;
            break;
        case 0x04:
            real_ram_size = 0x20000;
            break;
        case 0x05:
            real_ram_size = 0x10000;
            break;
        default:
            spdlog::warn("Unknown RAM size {:d}", ram_size);
            real_ram_size = 0;
            break;
    }
    ram = std::vector<uint8_t>(real_ram_size, 0);
    // Set bank mode
    bank_mode = get_bank_mode();
}

BANK_MODE Cartridge::get_bank_mode() {
    spdlog::info("Rom type is {:d}", rom.at(0x147));
    switch (rom.at(0x147)) {
        case 0x01:
        case 0x02:
        case 0x03:
        case 0xFF:
            return BANK_MODE::MBC1;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            return BANK_MODE::MBC3;
        default:
            return BANK_MODE::NONE;
    }
}

uint8_t Cartridge::read(uint16_t addr) {
    switch (bank_mode) {
        case NONE:
            return rom.at(addr);
        case MBC1:
            if (addr >= 0 && addr < 0x4000) {
                return rom.at(addr);
            } else if (addr >= 0x4000 && addr < 0x8000) { // read from the current ROM bank
                uint bank_addr = (addr - 0x4000) + 0x4000 * curr_rom_bank;
                return rom.at(bank_addr);
            } else if (addr >= 0xA000 && addr < 0xC000) {
                uint ram_addr = (addr - 0xA000) + 0x2000 * curr_ram_bank;
                return ram.at(ram_addr);
            }
            spdlog::critical("Reading from unknown address 0x{:X} in MBC1 mode", addr);
            break;
        case MBC2:
            spdlog::warn("Reading from MBC2");
            break;
        case MBC3:
            if (addr >= 0 && addr < 0x4000) {
                return rom.at(addr);
            } else if (addr >= 0x4000 && addr < 0x8000) { // read from the current ROM bank
                uint bank_addr = (addr - 0x4000) + 0x4000 * curr_rom_bank;
                return rom.at(bank_addr);
            } else if (addr >= 0xA000 && addr < 0xC000) {
                if (ram_over_rtc) { // Only difference from MBC1
                    uint ram_addr = (addr - 0xA000) + 0x2000 * curr_ram_bank;
                    return ram.at(ram_addr);
                } else {
                    return get_rtc_register();
                }
            }
            spdlog::critical("Reading from unknown address 0x{:X} in MBC3 mode", addr);
            break;
    }
    return 0x0;
}


void Cartridge::write(uint16_t addr, uint8_t byte) {
    switch (bank_mode) {
        case NONE:
            spdlog::warn("Undefined behaviour, writing to rom without MBC");
            break;
        case MBC1:
            if (addr >= 0 && addr < 0x2000) {
                is_ram_enabled = true;
            } else if (addr >= 0x2000 && addr < 0x4000) {
                // Writing to this area sets the rom bank depending on the 5 bits written here
                // Special cases, see https://gbdev.io/pandocs/#mbc1
                if (byte == 0x0) {
                    curr_rom_bank = 0x1;
                } else if (byte == 0x20) {
                    curr_rom_bank = 0x21;
                } else if (byte == 0x40) {
                    curr_rom_bank = 0x41;
                } else if (byte == 0x60) {
                    curr_rom_bank = 0x61;
                } else { // normal case
                    curr_rom_bank = byte & 0b11111;
                } // TODO: Handle setting uppers bits of rom bank and selecting rom/ram mode
            } else if (addr >= 0xA000 && addr < 0xC000) {
                if (!is_ram_enabled)
                    return;
                uint16_t ram_addr = (addr - 0xA000) + 0x2000 * curr_ram_bank;
                ram.at(ram_addr) = byte;
            }
            break;
        case MBC2: // TODO: this
            break;
        case MBC3:
            if (addr >= 0 && addr < 0x2000) {
                is_ram_enabled = true;
            } else if (addr >= 0x2000 && addr < 0x4000) {
                // Writing to this area sets the rom bank depending on the 5 bits written here
                // Special cases, see https://gbdev.io/pandocs/#mbc3
                if (byte == 0x0) {
                    curr_rom_bank = 0x1;
                } else { // normal case
                    curr_rom_bank = byte & 0b1111111;
                } // TODO: Handle setting uppers bits of rom bank and selecting rom/ram mode
            } else if (addr >= 0x4000 && addr < 0x6000) {
                if (byte <= 0x03) {
                    ram_over_rtc = true;
                    curr_ram_bank = byte;
                } else if (byte >= 0x08 && byte <= 0x0C) {
                    ram_over_rtc = false;
                    set_rtc_mode(byte);
                }
            } else if (addr >= 0x6000 && addr < 0x8000) { // Latch clock data
                if (byte == 0x00) {
                    is_latching = true;
                } else if (byte == 0x01 && is_latching) {
                    set_rtc_registers();
                    is_latched = !is_latched;
                } else {
                    is_latching = false;
                }
            } else if (addr >= 0xA000 && addr < 0xC000) {
                if (ram_over_rtc) {
                    if (!is_ram_enabled)
                        return;
                    uint16_t ram_addr = (addr - 0xA000) + 0x2000 * curr_ram_bank;
                    ram.at(ram_addr) = byte;
                } else {
                    set_rtc_register(rtc_mode, byte);
                }
            }
            break;
    }
}

void Cartridge::set_rtc_register(uint16_t addr, uint8_t byte) {
    set_rtc_registers();
    if (addr <= 0x0C) {
        rtc_registers[0x08 - addr] = byte;
    }
}

void Cartridge::set_rtc_registers() {
    if (!is_latched) {
        std::chrono::milliseconds new_time = now();
        uint total_seconds = (start_time - new_time).count();
        uint8_t seconds = total_seconds % 60;
        uint8_t minutes = (total_seconds / 60) % 60;
        uint8_t hours = (total_seconds / (60 * 60)) % 60;
        uint8_t days = (total_seconds / (60 * 60 * 24)) % 60;
        uint8_t msb_days = ((total_seconds / (60 * 60 * 24)) % 512) >> 8;
        uint8_t halt_flag = 1 << 6;
        // TODO: add the day overflow bit
        uint8_t misc_bits = halt_flag | msb_days;

        rtc_registers[0] = seconds;
        rtc_registers[1] = minutes;
        rtc_registers[2] = hours;
        rtc_registers[3] = days;
        rtc_registers[4] = misc_bits;
    }
}

void Cartridge::set_rtc_mode(uint8_t byte) {
    switch(byte) {
        case 0x08:
            rtc_mode = RTC_SECONDS;
            break;
        case 0x09:
            rtc_mode = RTC_MINUTES;
            break;
        case 0x0A:
            rtc_mode = RTC_HOURS;
            break;
        case 0x0B:
            rtc_mode = RTC_DAYS;
        case 0x0C:
            rtc_mode = RTC_MISC;
            break;
        default:
            spdlog::warn("Unknown value for setting rtc register 0x{:X}", byte);
            rtc_mode = RTC_SECONDS;
            break;
    }
}

uint8_t Cartridge::get_rtc_register() {
    return rtc_registers[rtc_mode - 0x08];
}