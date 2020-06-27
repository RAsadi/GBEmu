//
// Created by Rayan Asadi on 2020-06-30.
//

#include "interrupt_handler.h"
#include <unordered_map>
#include "utils/constants.h"
#include "utils/bit_utils.h"
#include "memory/mmu.h"
#include "cpu/cpu.h"
#include "spdlog/spdlog.h"

const uint16_t V_BLANK_INTERRUPT_ROUTINE_ADDR = 0x0040;
const uint16_t LCD_INTERRUPT_ROUTINE_ADDR = 0x0048;
const uint16_t TIMER_INTERRUPT_ROUTINE_ADDR = 0x0050;
const uint16_t JOYPAD_INTERRUPT_ROUTINE_ADDR = 0x0060;

std::unordered_map<INTERRUPT_TYPE, uint8_t> INTERRUPT_TYPE_TO_BIT_MAP = {
        {V_BLANK_INTERRUPT, 0},
        {LCD_INTERRUPT,     1},
        {TIMER_INTERRUPT,   2},
        {JOYPAD_INTERRUPT, 4}
};

std::unordered_map<uint8_t, INTERRUPT_TYPE> BIT_TO_INTERRUPT_TYPE_MAP = {
        {0, V_BLANK_INTERRUPT},
        {1, LCD_INTERRUPT},
        {2, TIMER_INTERRUPT},
        {4, JOYPAD_INTERRUPT}
};

InterruptHandler::InterruptHandler(MMU *mmu, CPU *cpu) {
    this->mmu = mmu;
    this->cpu = cpu;
    interrupts_are_enabled = true;
}

void InterruptHandler::request_interrupt(INTERRUPT_TYPE type) {
    uint8_t bit_to_set = INTERRUPT_TYPE_TO_BIT_MAP[type];
    uint8_t previous_requested_bits = mmu->read_byte(INTERRUPT_REQUEST_REGISTER);
    set_bit(previous_requested_bits, bit_to_set);
    mmu->write_byte(INTERRUPT_REQUEST_REGISTER, previous_requested_bits);
}

void InterruptHandler::handle_interrupts() {
    if (interrupts_are_enabled) {
        uint8_t enabled_interrupts = mmu->read_byte(INTERRUPT_ENABLED_REGISTER);
        uint8_t requested_interrupts = mmu->read_byte(INTERRUPT_REQUEST_REGISTER);
        uint8_t interrupts_to_do = (requested_interrupts & enabled_interrupts);
        if (!interrupts_to_do)
            return;
        // save current pc by pushing onto stack
        mmu->push_word_to_stack(cpu->get_pc());
        for (int i = 0; i < 5; i++) {
            if (get_bit(interrupts_to_do, i) && i != 3) {
                spdlog::debug("Doing interrupt {:d}", i);
                handle_interrupts(BIT_TO_INTERRUPT_TYPE_MAP[i]);
                return;
            }
        }
    }
}

void InterruptHandler::handle_interrupts(INTERRUPT_TYPE type) {
    cpu->is_halted = false;
    interrupts_are_enabled = false;
    uint8_t requested_interrupts = mmu->read_byte(INTERRUPT_REQUEST_REGISTER);
    unset_bit(requested_interrupts, INTERRUPT_TYPE_TO_BIT_MAP[type]);
    mmu->write_byte(INTERRUPT_REQUEST_REGISTER, requested_interrupts);

    switch(type) {
        case V_BLANK_INTERRUPT:
            cpu->set_pc(V_BLANK_INTERRUPT_ROUTINE_ADDR);
            break;
        case LCD_INTERRUPT:
            cpu->set_pc(LCD_INTERRUPT_ROUTINE_ADDR);
            break;
        case TIMER_INTERRUPT:
            cpu->set_pc(TIMER_INTERRUPT_ROUTINE_ADDR);
            break;
        case JOYPAD_INTERRUPT:
            cpu->set_pc(JOYPAD_INTERRUPT_ROUTINE_ADDR);
            break;
    }
}

void InterruptHandler::enable() {
    interrupts_are_enabled = true;
}

void InterruptHandler::disable() {
    interrupts_are_enabled = false;
}
