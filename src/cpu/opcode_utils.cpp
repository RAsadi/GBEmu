//
// Created by Rayan Asadi on 2020-07-11.
//

#include "cpu.h"
#include "../memory/mmu.h"

uint8_t CPU::get_immediate_byte() {
    uint8_t immediate = mmu->read_byte(pc);
    pc++;
    return immediate;
}

uint16_t CPU::get_immediate_word() {
    uint16_t immediate = mmu->read_word(pc);
    pc += 2;
    return immediate;
}

uint8_t CPU::get_byte_from_immediate_as_addr() {
    return mmu->read_byte(get_immediate_word());
}

void CPU::apply_op_to_addr(uint16_t addr, std::function<void(uint8_t&)> op) const {
    uint8_t value = mmu->read_byte(addr);
    op(value);
    mmu->write_byte(addr, value);
}

uint8_t CPU::handle_extended_op() {
    uint8_t opcode = get_immediate_byte();
    previous_op = extended_opcode_table[opcode].name;
    return extended_opcode_table[opcode].func();
}
