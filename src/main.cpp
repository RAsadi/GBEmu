//
// Created by Rayan Asadi on 2020-06-28.
//

#include "cpu/cpu.h"

#include "memory/cartridge.h"
#include "memory/mmu.h"

#include "interrupt_handler.h"
#include "joypad.h"
#include "gpu.h"
#include "timer.h"
#include "utils/constants.h"

#include <SDL.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

const int FPS = 60;
const int MS_PER_FRAME = 1000 / FPS;

int main(int argc, char *argv[]) {
//    // Setup logging
//    auto file_logger = spdlog::basic_logger_mt("file_logger", "log.txt", true);
//    spdlog::set_default_logger(file_logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("%v");


    // Cpu main loop
    Cartridge *cartridge = new Cartridge();
    cartridge->load_rom("pk.gb"); // TODO: load from args
    MMU *mmu = new MMU(cartridge);
    CPU *cpu = new CPU(mmu);
    InterruptHandler *interrupt_handler = new InterruptHandler(mmu, cpu);
    Joypad *joypad = new Joypad(mmu, interrupt_handler);
    Timer *timer = new Timer(mmu, interrupt_handler);
    GPU *gpu = new GPU(mmu, interrupt_handler, joypad);

    // Janky dependency injection here
    cpu->set_interrupt_handler(interrupt_handler);
    mmu->set_cpu(cpu);
    mmu->set_timer(timer);
    mmu->set_joypad(joypad);

    uint8_t opcode;
    int curr_cycles, new_cycles;
    uint prev_time, cycle_time_ms;

    // main emulation loop
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
