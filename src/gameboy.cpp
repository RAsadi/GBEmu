//
// Created by Rayan Asadi on 2020-07-14.
//

#include "gameboy.h"
#include <string>

#include "cpu/cpu.h"

#include "memory/cartridge.h"
#include "memory/mmu.h"

#include "interrupt_handler.h"
#include "joypad.h"
#include "gpu.h"
#include "timer.h"

#include <SDL.h>

const int FPS = 60;
const int MS_PER_FRAME = 1000 / FPS;

Gameboy::Gameboy(std::string &rom_path) {
    cartridge = new Cartridge();
    cartridge->load_rom(rom_path);
    mmu = new MMU(cartridge);
    cpu = new CPU(mmu);
    interrupt_handler = new InterruptHandler(mmu, cpu);
    joypad = new Joypad(mmu, interrupt_handler);
    timer = new Timer(mmu, interrupt_handler);
    gpu = new GPU(mmu, interrupt_handler, joypad);

    // Janky dependency injection here
    cpu->set_interrupt_handler(interrupt_handler);
    mmu->set_cpu(cpu);
    mmu->set_timer(timer);
    mmu->set_joypad(joypad);
}

void Gameboy::run() {
    uint8_t opcode;
    int curr_cycles, new_cycles;
    uint32_t prev_time, cycle_time_ms;

    while (true) {
        curr_cycles = 0;
        prev_time = SDL_GetTicks();
        while (curr_cycles < CYCLES_PER_FRAME) {
            opcode = mmu->read_byte(cpu->get_pc());
            new_cycles = cpu->handle_op(opcode);
            curr_cycles += new_cycles;
            gpu->clock_step(new_cycles);
            timer->update_timers(new_cycles);
            interrupt_handler->handle_interrupts();
        }

        // Delay for framerate, TODO: reworking this to based on sound
        cycle_time_ms = SDL_GetTicks() - prev_time;
        if (cycle_time_ms < MS_PER_FRAME) {
            SDL_Delay(MS_PER_FRAME - cycle_time_ms);
        }
    }
}
