//
// Created by Rayan Asadi on 2020-06-30.
//

#include "timer.h"
#include "memory/mmu.h"
#include "interrupt_handler.h"
#include "utils/bit_utils.h"
#include "utils/constants.h"
#include <unordered_map>

// Mapping from the bytes in the timer controller register, # of cpu cycles expected for timer updates.
std::unordered_map<uint8_t, int> TIMER_CONTROLLER_FREQUENCY_MAP = {
        {0, 1024},
        {1, 16},
        {2, 64},
        {3, 256}
};

Timer::Timer(MMU *mmu, InterruptHandler *interrupt_handler) {
    this->mmu = mmu;
    this->interrupt_handler = interrupt_handler;
    divider_counter = 256;
    set_clock_frequency();
}


void Timer::update_timers(uint8_t cycles) {
    // Handle divider reg
    divider_counter -= cycles;
    while (divider_counter <= 0) {
        divider_counter += 256;
        mmu->unsafe_write_byte(DIVIDER_REGISTER_ADDRESS, mmu->unsafe_read_byte(DIVIDER_REGISTER_ADDRESS) + 1);
    }

    // Handle normal timers
    if (is_clock_enabled()) {
        timer_counter -= cycles;

        while (timer_counter <= 0) {
            timer_counter += TIMER_CONTROLLER_FREQUENCY_MAP[get_clock_frequency()];

            // overflow check
            uint8_t curr_time = mmu->read_byte(TIMER_ADDRESS);
            curr_time++;
            if (curr_time == 0) { // check for overflow
                mmu->write_byte(TIMER_ADDRESS, mmu->read_byte(TIMER_MODULATOR_ADDRESS));
                interrupt_handler->request_interrupt(TIMER_INTERRUPT);
            } else {
                mmu->write_byte(TIMER_ADDRESS, curr_time);
            }
        }
    }
}

bool Timer::is_clock_enabled() const {
    uint8_t timer_controller = mmu->read_byte(TIMER_CONTROLLER_ADDRESS);
    return get_bit(timer_controller, 2);
}

uint8_t Timer::get_clock_frequency() const {
    uint8_t timer_controller = mmu->read_byte(TIMER_CONTROLLER_ADDRESS);
    return timer_controller & 0b11;
}

void Timer::set_clock_frequency() {
    timer_counter = TIMER_CONTROLLER_FREQUENCY_MAP[get_clock_frequency()];
}

