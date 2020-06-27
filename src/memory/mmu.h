//
// Created by Rayan Asadi on 2020-06-28.
//

#ifndef GBEMU_MMU_H
#define GBEMU_MMU_H

#include <cstdint>
#include <string>

class Cartridge;
class Joypad;
class CPU;
class Timer;

class MMU {
private:
    Cartridge *cartridge;
    Joypad *joypad;
    CPU *cpu;
    Timer *timer;

    void dma_transfer(uint8_t byte);
    /*
     * The Gameboy has a memory bus of size 65536 bytes
     * This memory is divided into 9 different sections
     *
     * 0x0000 - 0x3FFF
     * The first 16k bytes of the cartridge are always located in this section of memory.
     * As well, on boot Bios resides from 0x0000 - 0x00FF.
     * Once the gameboy is booted and runs the bios instructions, that range is turned into addressable memory
     * for the cartridge.
     * Also, 0x0100 - 0x014F should contain metadata about the cartridge
     *
     * 0x4000 - 0x7FFF
     * These 16k bytes are also for use for a 16k chunk of the cartridge. Cartridges often contain a chip that allows
     * them to swap out this 16k section for another 16k chunk of memory, allowing them to run more than just 62k bytes
     * of instructions. Theoretically, since you can just constantly swap this section out, there is no limit to the
     * size of the cartridge.
     *
     * 0x8000 - 0x9FFF
     * Tile data and sprite sheet used for graphics
     *
     * 0xA000 - 0xBFFF
     * External RAM that can be used by the cartridge
     *
     * 0xC000 - 0xDFFF
     * Internal RAM used by the CPU
     *
     * 0xE000 - 0xFDFF
     * An exact copy of internal RAM, not sure yet why this exists.
     *
     * 0xFE00 - 0xFE9F
     * Stores information about the 40 sprites to render
     *
     * 0xFF00 - 0xFF7F
     * Memory mapped I/O, input and output devices write and read from here along with the cpu
     *
     * 0xFF80 - 0xFFFF
     * Page zero, used as a high speed area for reads and writes, where most of the heavy lifting will take place.
     */
    uint8_t memory[0xFFFF + 1];
    bool is_booted;
public:
    MMU(Cartridge *cartridge);
    void set_joypad(Joypad *joypad);
    void set_cpu(CPU *cpu);
    void set_timer(Timer *timer);

    uint8_t read_byte(uint16_t addr);
    uint8_t unsafe_read_byte(uint16_t addr);
    uint16_t read_word(uint16_t addr);
    void write_byte(uint16_t addr, uint8_t byte);
    void unsafe_write_byte(uint16_t addr, uint8_t byte);
    void write_word(uint16_t addr, uint16_t word);
    void push_word_to_stack(uint16_t word);
    uint16_t pop_word_from_stack();
};


#endif //GBEMU_MMU_H
