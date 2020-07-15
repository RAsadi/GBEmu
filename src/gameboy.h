//
// Created by Rayan Asadi on 2020-07-14.
//

#ifndef GBEMU_GAMEBOY_H
#define GBEMU_GAMEBOY_H

#include <string>

class Cartridge;
class MMU;
class CPU;
class GPU;
class InterruptHandler;
class Joypad;
class Timer;

class Gameboy {
    Cartridge *cartridge;
    MMU *mmu;
    GPU *gpu;
    CPU *cpu;
    InterruptHandler *interrupt_handler;
    Joypad *joypad;
    Timer *timer;
public:
    Gameboy(std::string &rom_path);
    void run();
};


#endif //GBEMU_GAMEBOY_H
