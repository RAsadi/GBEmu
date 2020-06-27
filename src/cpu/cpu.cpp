//
// Created by Rayan Asadi on 2020-06-27.
//

#include "cpu.h"
#include "../interrupt_handler.h"
#include "../memory/mmu.h"
#include "../utils/bit_utils.h"

CPU::CPU(MMU *mmu) {
    this->mmu = mmu;
    is_halted = false;
    should_disable_interrupts = 0;
    should_enable_interrupts = 0;
    setup_opcode_table();
}

/// Register getter and setters
uint16_t CPU::get_af() const {
    return (a << 8) | f;
}

void CPU::set_af(uint16_t value) {
    a = ((value & 0xFF00) >> 8);
    f = value & 0x00FF;
}

uint16_t CPU::get_bc() const {
    return (b << 8) | c;
}

void CPU::set_bc(uint16_t value) {
    b = ((value & 0xFF00) >> 8);
    c = value & 0x00FF;
}

uint8_t CPU::get_bc_value() const {
    return mmu->read_byte(get_bc());
}

uint16_t CPU::get_de() const {
    return (d << 8) | e;
}

void CPU::set_de(uint16_t value) {
    d = ((value & 0xFF00) >> 8);
    e = value & 0x00FF;
}

uint8_t CPU::get_de_value() const {
    return mmu->read_byte(get_de());
}

uint16_t CPU::get_hl() const {
    return (h << 8) | l;
}

void CPU::set_hl(uint16_t value) {
    h = ((value & 0xFF00) >> 8);
    l = value & 0x00FF;
}

uint8_t CPU::get_hl_value() const {
    return mmu->read_byte(get_hl());
}

uint16_t CPU::get_pc() const {
    return pc;
}

void CPU::set_pc(uint16_t pc) {
    this->pc = pc;
}

uint16_t CPU::get_sp() const {
    return sp;
}

void CPU::set_sp(uint16_t sp) {
    this->sp = sp;
}

/// Flag operations
bool CPU::get_flag(uint8_t flag) const {
    return get_bit(f, flag);
}

void CPU::set_flag(uint8_t flag, bool value) {
    if (value) {
        set_bit(f, flag);
    } else {
        unset_bit(f, flag);
    }
}

uint8_t CPU::handle_op(uint8_t opcode) {
    if (!is_halted) {
        pc++;
        // Enables and disables wait one instruction before actually ocurring
        if (should_disable_interrupts == 2) {
            should_disable_interrupts--;
        }
        if (should_enable_interrupts == 2) {
            should_enable_interrupts--;
        }
        if (should_disable_interrupts == 1) {
            interrupt_handler->disable();
            should_disable_interrupts = 0;
        }
        if (should_enable_interrupts == 1) {
            interrupt_handler->enable();
            should_enable_interrupts = 0;
        }
        previous_op = opcode_table[opcode].name;
        return opcode_table[opcode].func();
    }
    return 4;
}

void CPU::set_interrupt_handler(InterruptHandler *interrupt_handler) {
    this->interrupt_handler = interrupt_handler;
}