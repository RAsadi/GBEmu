//
// Created by Rayan Asadi on 2020-06-30.
//

#ifndef GBEMU_TIMER_H
#define GBEMU_TIMER_H

#include <cstdint>

class MMU;
class InterruptHandler;

class Timer {
public:
    Timer(MMU *mmu, InterruptHandler *interrupt_handler);
    bool is_clock_enabled() const;
    uint8_t get_clock_frequency() const;
    void set_clock_frequency();
    void update_timers(uint8_t cycles);
private:
    MMU *mmu;
    InterruptHandler *interrupt_handler;
    /*
     * The Gameboy has two types of timers. One controllable timer which can be set to update at
     * specific frequencies, and which fires an interrupt upon reaching 255, and one non controllable
     * timer (called the divider register) which constantly ticks up per clock.
     *
     * The divider register ticks up at 16384 HZ, or CPU_CLOCK_SPEED / 256, So every 256 clocks we will
     * increment the divider register
     */
    int timer_counter, divider_counter;
};


#endif //GBEMU_TIMER_H
