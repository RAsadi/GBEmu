//
// Created by Rayan Asadi on 2020-06-27.
//

#ifndef GBEMU_CPU_H
#define GBEMU_CPU_H

#include <cstdint>
#include <unordered_map>
#include <string>
#include "../utils/enums.h"

class MMU;
class InterruptHandler;

class Instruction {
public:
    std::string name;
    std::function<uint8_t()> func;
};

class CPU {
public:
    CPU(MMU *mmu);

    // Sometimes external components need to read from the pc and sp
    uint16_t get_pc() const;
    void set_pc(uint16_t pc);

    uint16_t get_sp() const;
    void set_sp(uint16_t sp);

    /*
     * Flags:
     * The Gameboy CPU has four flags, represented in the 8 bit 'f' register by the top 4 bits.
     * Could use bitset to implement this but I didn't want to
     * Bit 7: zero flag, set to true if operation result is equal to 0
     * Bit 6: subtraction flag, set to true if operation was a subtraction
     * Bit 5: half carry flag, set to true if operation resulted in overflow from bottom 4 bits to upper 4 bits
     * Bit 4: carry flag, set to true if operation resulted in overflow
     */
    uint8_t FLAG_Z = 7;
    uint8_t FLAG_N = 6;
    uint8_t FLAG_H = 5;
    uint8_t FLAG_C = 4;

    /*
     * handle_op takes in an opcode and fully handles that operation,
     * including moving internal clocks and updating pc.
     * Returns number of cycles used for handling the opcode
     */
    uint8_t handle_op(uint8_t opcode);

    // TODO: make this not public
    bool is_halted;
    std::string previous_op = "";
    void set_interrupt_handler(InterruptHandler *interrupt_handler);
//private:
    // TODO: fix this weird depedency injection thing
    MMU *mmu;
    InterruptHandler *interrupt_handler;

    // Interrupts enabling and disabling are set after a one instruction delay
    // so we need to deal with that
    uint8_t should_disable_interrupts;
    uint8_t should_enable_interrupts;

    /*
     * 8 bit registers:
     * the a register is used as the source and destination for almost all the 8-bit ops.
     * the f register is used for flags. The top four bits in f (bottom four are ignored)
     *  are used as flags, each of which represent a CPU state. They are defined above
     * the b and c registers are commonly used as counters for loops (apparently), but can be used for
     *  pretty much anything
     * the d and e registers are generally treated as a 16 bit register for storing destination addrs,
     *  but can be used for pretty much anything
     * the h and l registers are used almost exclusively for indirect addressing (instead of passing in
     *  an instruction to an operation, it can just use the 16 bit value in hl)
     *
     */
    uint8_t a, f, b, c, d, e, h, l;

    /*
     * 16 bit registers:
     * the sp register is used as the stack pointer
     * the pc register is used as the program counter
     */
    uint16_t sp, pc;

    // Flag getter and setters
    bool get_flag(uint8_t flag) const;
    void set_flag(uint8_t flag, bool value);

    /*
     * Functions for register access
     *
     * For the Gameboy cpu, the registers af, bc, de, and hl are tied together, in the sense that they
     * are gotten and set using a 16 bit value, the top 8 which represent first register in the pair,
     * (a, b, d, h) and the bottom 8 which represent the second register in the pair (f,c,e,l)
     */
    uint16_t get_af() const;
    void set_af(uint16_t value);
    uint8_t get_af_value();

    uint16_t get_bc() const;
    void set_bc(uint16_t value);
    uint8_t get_bc_value() const;

    uint16_t get_de() const;
    void set_de(uint16_t value);
    uint8_t get_de_value() const;

    uint16_t get_hl() const;
    void set_hl(uint16_t value);
    uint8_t get_hl_value() const;

    std::unordered_map<uint8_t, Instruction> opcode_table;
    std::unordered_map<uint8_t, Instruction> extended_opcode_table;
    void setup_opcode_table();

    /// opcode helpers
    uint8_t get_immediate_byte();
    uint8_t get_byte_from_immediate_as_addr();
    uint16_t get_immediate_word();
    uint8_t handle_extended_op();
    void apply_op_to_addr(uint16_t addr, std::function<void(uint8_t&)> op) const;

    /// Load ops
    void LOAD_IMMEDIATE(uint8_t &reg);
    void LOAD(uint8_t &reg, uint8_t value);
    void LOAD_TO_ADDR(uint16_t addr, uint8_t value);
    void LOAD_TO_ADDR(uint16_t addr, uint16_t value);
    void LOAD_TO_ADDR_IMMEDIATE(uint16_t addr);
    void PUSH(uint16_t value);
    void PUSH(uint8_t value);
    uint16_t POP_WORD();

    /// 8-bit ALU ops
    void _ADD_8BIT(uint8_t &reg, uint8_t value, uint8_t carry_flag);
    void ADD_8BIT_CARRY(uint8_t &reg, uint8_t value);
    void ADD_8BIT_NO_CARRY(uint8_t &reg, uint8_t value);
    void _SUB_8BIT(uint8_t &reg, uint8_t value, uint8_t carry_flag);
    void SUB_8BIT_CARRY(uint8_t &reg, uint8_t value);
    void SUB_8BIT_NO_CARRY(uint8_t &reg, uint8_t value);
    void AND_8BIT(uint8_t &reg, uint8_t value);
    void OR_8BIT(uint8_t &reg, uint8_t value);
    void XOR_8BIT(uint8_t &reg, uint8_t value);
    void COMPARE_8BIT(uint8_t reg, uint8_t value);
    void INCREMENT_8BIT(uint8_t &reg);
    void DECREMENT_8BIT(uint8_t &reg);
    void INCREMENT_8BIT_AT_ADDR(uint16_t addr);
    void DECREMENT_8BIT_AT_ADDR(uint16_t addr);

    /// 16-bit ALU ops
    // Note that to deal with 16bit reg pairs, we used getters and setters, so we will continue that pattern
    // here by taking in getters and settings for 16 bit values. This is fine because we only really do this on
    // reg pairs, but would have been smarter to make reg pairs set and gettable directly
    void ADD_16BIT(const std::function<uint16_t(void)>& get, const std::function<void(uint16_t)>& set, uint16_t value);
    void INCREMENT_16BIT(const std::function<uint16_t()>& get, const std::function<void(uint16_t)>& set);
    void DECREMENT_16BIT(const std::function<uint16_t()>& get, const std::function<void(uint16_t)>& set);

    /// Misc ops
    void SWAP(uint8_t &reg);
    void DECIMAL_ADJUST(uint8_t &reg);
    void COMPLEMENT_8BIT(uint8_t &reg);
    void COMPLEMENT_CARRY();
    void SET_CARRY();
    void NOP();
    void HALT();
    void STOP();
    /// Bit Rotates
    void ROTATE_LEFT_CARRY_TO_0(uint8_t &reg);
    void ROTATE_LEFT(uint8_t &reg);
    void ROTATE_RIGHT_CARRY_TO_7(uint8_t &reg);
    void ROTATE_RIGHT(uint8_t &reg);
    /// Bit Shifts
    void SHIFT_LEFT(uint8_t &reg);
    void SHIFT_RIGHT(uint8_t &reg);
    void SHIFT_RIGHT_MSB_0(uint8_t &reg);
    /// Other bit ops
    void TEST_BIT(uint8_t reg, uint8_t bit);
    void SET_BIT(uint8_t &reg, uint8_t bit);
    void SET_BIT_FROM_ADDR(uint16_t addr, uint8_t bit);
    void RESET_BIT(uint8_t &reg, uint8_t bit);
    void RESET_BIT_FROM_ADDR(uint16_t addr, uint8_t bit);
    /// Jumps, calls and returns
    void JUMP(uint16_t addr);
    void JUMP_TO_IMMEDIATE_WORD();
    uint8_t CONDITIONAL_JUMP_TO_IMMEDIATE_WORD(JUMP_CONDITION cond);
    void JUMP_BY_IMMEDIATE_BYTE();
    uint8_t CONDITIONAL_JUMP_BY_IMMEDIATE_BYTE(JUMP_CONDITION cond);
    void CALL_IMMEDIATE();
    uint8_t CONDITIONAL_CALL_IMMEDIATE(JUMP_CONDITION cond);
    void RESET(uint8_t reset_location);
    void RETURN_FROM_STACK();
    uint8_t CONDITIONAL_RETURN_FROM_STACK(JUMP_CONDITION cond);
    void RETURN_FROM_STACK_WITH_INTERRUPTS();

    void ADD_16BIT_SP();
};



#endif //GBEMU_CPU_H
