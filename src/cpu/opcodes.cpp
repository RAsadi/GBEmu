//
// Created by Rayan Asadi on 2020-07-11.
//

#include "cpu.h"
#include "../interrupt_handler.h"
#include "../memory/mmu.h"
#include "../utils/bit_utils.h"

void CPU::setup_opcode_table() {
    /*
     * See https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html for all the opcodes
     */
    opcode_table = {
            //////////////////////////////////////////////////////// 8-bit Load Immediate
            {0x06, {"LD B, N", [this]{LOAD_IMMEDIATE(b); return 8;}}},
            {0x0E, {"LD C, N", [this]{LOAD_IMMEDIATE(c); return 8;}}},
            {0x16, {"LD D, N", [this]{LOAD_IMMEDIATE(d); return 8;}}},
            {0x1E, {"LD E, N", [this]{LOAD_IMMEDIATE(e); return 8;}}},
            {0x26, {"LD H, N", [this]{LOAD_IMMEDIATE(h); return 8;}}},
            {0x2E, {"LD L, N", [this]{LOAD_IMMEDIATE(l); return 8;}}},
            {0x3E, {"LD A, N", [this]{LOAD_IMMEDIATE(a); return 8;}}},
            //////////////////////////////////////////////////////// 8-bit Loads
            /////////////////// Load to A
            /// Basic loads from registers
            {0x7F, {"LD A, A", [this]{LOAD(a, a); return 4;}}},
            {0x78, {"LD A, B", [this]{LOAD(a, b); return 4;}}},
            {0x79, {"LD A, C", [this]{LOAD(a, c); return 4;}}},
            {0x7A, {"LD A, D", [this]{LOAD(a, d); return 4;}}},
            {0x7B, {"LD A, E", [this]{LOAD(a, e); return 4;}}},
            {0x7C, {"LD A, H", [this]{LOAD(a, h); return 4;}}},
            {0x7D, {"LD A, L", [this]{LOAD(a, l); return 4;}}},
            /// Loads from register pairs as addresses
            {0x7E, {"LD A, (HL)", [this]{LOAD(a, get_hl_value()); return 8;}}},
            {0x0A, {"LD A, (BC)", [this]{LOAD(a, get_bc_value()); return 8;}}},
            {0x1A, {"LD A, (DE)", [this]{LOAD(a, get_de_value()); return 8;}}},
            {0xFA, {"LD A, (nn)", [this]{LOAD(a, get_byte_from_immediate_as_addr()); return 16;}}},
            {0x3A, {"LD A, (HL-)", [this]{LOAD(a, get_hl_value()); set_hl(get_hl() - 1); return 8;}}},
            {0x2A, {"LD A, (HL+)", [this]{LOAD(a, get_hl_value()); set_hl(get_hl() + 1); return 8;}}},
            /// Loads from registers + 0xFF00 as addresses
            {0xF2, {"LD A, (0xFF00 + C)", [this]{LOAD(a, this->mmu->read_byte(c + 0xFF00)); return 8;}}},
            {0xF0, {"LD A, (0xFF00 + N)", [this]{LOAD(a, this->mmu->read_byte(get_immediate_byte() + 0xFF00)); return 12;}}},
            /////////////////// Load to B
            {0x40, {"LD B, B", [this]{LOAD(b, b); return 4;}}},
            {0x41, {"LD B, C", [this]{LOAD(b, c); return 4;}}},
            {0x42, {"LD B, D", [this]{LOAD(b, d); return 4;}}},
            {0x43, {"LD B, E", [this]{LOAD(b, e); return 4;}}},
            {0x44, {"LD B, H", [this]{LOAD(b, h); return 4;}}},
            {0x45, {"LD B, L", [this]{LOAD(b, l); return 4;}}},
            {0x46, {"LD B, (HL)", [this]{LOAD(b, get_hl_value()); return 8;}}},
            {0x47, {"LD B, A", [this]{LOAD(b, a); return 4;}}},
            /////////////////// Load to C
            {0x48, {"LD C, B", [this]{LOAD(c, b); return 4;}}},
            {0x49, {"LD C, C", [this]{LOAD(c, c); return 4;}}},
            {0x4A, {"LD C, D", [this]{LOAD(c, d); return 4;}}},
            {0x4B, {"LD C, E", [this]{LOAD(c, e); return 4;}}},
            {0x4C, {"LD C, H", [this]{LOAD(c, h); return 4;}}},
            {0x4D, {"LD C, L", [this]{LOAD(c, l); return 4;}}},
            {0x4E, {"LD C, (HL)", [this]{LOAD(c, get_hl_value()); return 8;}}},
            {0x4F, {"LD C, A", [this]{LOAD(c, a); return 4;}}},
            /////////////////// Load to D
            {0x50, {"LD D, B", [this]{LOAD(d, b); return 4;}}},
            {0x51, {"LD D, C", [this]{LOAD(d, c); return 4;}}},
            {0x52, {"LD D, D", [this]{LOAD(d, d); return 4;}}},
            {0x53, {"LD D, E", [this]{LOAD(d, e); return 4;}}},
            {0x54, {"LD D, H", [this]{LOAD(d, h); return 4;}}},
            {0x55, {"LD D, L", [this]{LOAD(d, l); return 4;}}},
            {0x56, {"LD D, (HL)", [this]{LOAD(d, get_hl_value()); return 8;}}},
            {0x57, {"LD D, A", [this]{LOAD(d, a); return 4;}}},
            /////////////////// Load to E
            {0x58, {"LD E, B", [this]{LOAD(e, b); return 4;}}},
            {0x59, {"LD E, C", [this]{LOAD(e, c); return 4;}}},
            {0x5A, {"LD E, D", [this]{LOAD(e, d); return 4;}}},
            {0x5B, {"LD E, E", [this]{LOAD(e, e); return 4;}}},
            {0x5C, {"LD E, H", [this]{LOAD(e, h); return 4;}}},
            {0x5D, {"LD E, L", [this]{LOAD(e, l); return 4;}}},
            {0x5E, {"LD E, (HL)", [this]{LOAD(e, get_hl_value()); return 8;}}},
            {0x5F, {"LD E, A", [this]{LOAD(e, a); return 4;}}},
            /////////////////// Load to H
            {0x60, {"LD H, B", [this]{LOAD(h, b); return 4;}}},
            {0x61, {"LD H, C", [this]{LOAD(h, c); return 4;}}},
            {0x62, {"LD H, D", [this]{LOAD(h, d); return 4;}}},
            {0x63, {"LD H, E", [this]{LOAD(h, e); return 4;}}},
            {0x64, {"LD H, H", [this]{LOAD(h, h); return 4;}}},
            {0x65, {"LD H, L", [this]{LOAD(h, l); return 4;}}},
            {0x66, {"LD H, (HL)", [this]{LOAD(h, get_hl_value()); return 8;}}},
            {0x67, {"LD H, A", [this]{LOAD(h, a); return 4;}}},
            /////////////////// Load to L
            {0x68, {"LD L, B", [this]{LOAD(l, b); return 4;}}},
            {0x69, {"LD L, C", [this]{LOAD(l, c); return 4;}}},
            {0x6A, {"LD L, D", [this]{LOAD(l, d); return 4;}}},
            {0x6B, {"LD L, E", [this]{LOAD(l, e); return 4;}}},
            {0x6C, {"LD L, H", [this]{LOAD(l, h); return 4;}}},
            {0x6D, {"LD L, L", [this]{LOAD(l, l); return 4;}}},
            {0x6E, {"LD L, (HL)", [this]{LOAD(l, get_hl_value()); return 8;}}},
            {0x6F, {"LD L, A", [this]{LOAD(l, a); return 4;}}},
            /////////////////// Load to HL address
            {0x70, {"LD (HL), B", [this]{LOAD_TO_ADDR(get_hl(), b); return 8;}}},
            {0x71, {"LD (HL), C", [this]{LOAD_TO_ADDR(get_hl(), c); return 8;}}},
            {0x72, {"LD (HL), D", [this]{LOAD_TO_ADDR(get_hl(), d); return 8;}}},
            {0x73, {"LD (HL), E", [this]{LOAD_TO_ADDR(get_hl(), e); return 8;}}},
            {0x74, {"LD (HL), H", [this]{LOAD_TO_ADDR(get_hl(), h); return 8;}}},
            {0x75, {"LD (HL), L", [this]{LOAD_TO_ADDR(get_hl(), l); return 8;}}},
            {0x77, {"LD (HL), A", [this]{LOAD_TO_ADDR(get_hl(), a); return 8;}}},
            {0x36, {"LD (HL), nn", [this]{LOAD_TO_ADDR_IMMEDIATE(get_hl()); return 12;}}},
            {0x32, {"LD (HL-), A", [this]{LOAD_TO_ADDR(get_hl(), a); set_hl(get_hl() - 1); return 8;}}},
            {0x22, {"LD (HL+), A", [this]{LOAD_TO_ADDR(get_hl(), a); set_hl(get_hl() + 1); return 8;}}},
            /////////////////// Load to BC address
            {0x02, {"LD (BC), A", [this]{LOAD_TO_ADDR(get_bc(), a); return 8;}}},
            /////////////////// Load to DE address
            {0x12, {"LD (DE), A", [this]{LOAD_TO_ADDR(get_de(), a); return 8;}}},
            /////////////////// Load to immediate address
            {0xEA, {"LD (nn), A", [this]{LOAD_TO_ADDR(get_immediate_word(), a); return 16;}}},
            /// Loads to registers / immediates  + 0xFF00
            {0xE2, {"LD (0xFF00 + C), A", [this]{LOAD_TO_ADDR(c + 0xFF00, a); return 8;}}},
            {0xE0, {"LD (0xFF00 + N), A", [this]{LOAD_TO_ADDR( get_immediate_byte() + 0xFF00, a); return 12;}}},

            //////////////////////////////////////////////////////// 16-bit Loads
            /////////////////// Load to registers
            {0xF9, {"LD SP, HL", [this]{set_sp(get_hl()); return 8;}}},
            {0xF8, {"LD HL, SP + n", [this]{
                int8_t immediate = static_cast<int8_t>(get_immediate_byte());
                int res = sp + immediate;
                set_flag(FLAG_Z, false);
                set_flag(FLAG_N, false);
                set_flag(FLAG_H, ((sp ^ immediate ^ (res & 0xFFFF)) & 0x10) == 0x10);
                set_flag(FLAG_C, ((sp ^ immediate ^ (res & 0xFFFF)) & 0x100) == 0x100);
                set_hl(static_cast<uint16_t>(res));
                return 12;
            }}},
            /// Immediates
            {0x01, {"LD BC, nn", [this]{set_bc(get_immediate_word()); return 12;}}},
            {0x11, {"LD DE, nn", [this]{set_de(get_immediate_word()); return 12;}}},
            {0x21, {"LD HL, nn", [this]{set_hl(get_immediate_word()); return 12;}}},
            {0x31, {"LD SP, nn", [this]{set_sp(get_immediate_word()); return 12;}}},
            /////////////////// Load to immediates
            {0x08, {"LD (nn), SP", [this]{LOAD_TO_ADDR(get_immediate_word(), sp); return 20;}}},
            /////////////////// Stack ops
            /// Register pushing
            {0xF5, {"PUSH AF", [this]{PUSH(get_af()); return 16;}}},
            {0xC5, {"PUSH BC", [this]{PUSH(get_bc()); return 16;}}},
            {0xD5, {"PUSH DE", [this]{PUSH(get_de()); return 16;}}},
            {0xE5, {"PUSH HL", [this]{PUSH(get_hl()); return 16;}}},
            /// Register popping
            {0xF1, {"POP AF", [this]{
                set_af(POP_WORD());
                f &= 0b11110000; // Mask out the bottom 4 bits on F
                return 12;
            }}},
            {0xC1, {"POP BC", [this]{set_bc(POP_WORD()); return 12;}}},
            {0xD1, {"POP DE", [this]{set_de(POP_WORD()); return 12;}}},
            {0xE1, {"POP HL", [this]{set_hl(POP_WORD()); return 12;}}},

            //////////////////////////////////////////////////////// 8-bit ALU ops
            /////////////////// Operations to A
            /// Adds to A
            {0x87, {"ADD A, A", [this]{ADD_8BIT_NO_CARRY(a, a); return 4;}}},
            {0x80, {"ADD A, B", [this]{ADD_8BIT_NO_CARRY(a, b); return 4;}}},
            {0x81, {"ADD A, C", [this]{ADD_8BIT_NO_CARRY(a, c); return 4;}}},
            {0x82, {"ADD A, D", [this]{ADD_8BIT_NO_CARRY(a, d); return 4;}}},
            {0x83, {"ADD A, E", [this]{ADD_8BIT_NO_CARRY(a, e); return 4;}}},
            {0x84, {"ADD A, H", [this]{ADD_8BIT_NO_CARRY(a, h); return 4;}}},
            {0x85, {"ADD A, L", [this]{ADD_8BIT_NO_CARRY(a, l); return 4;}}},
            {0x86, {"ADD A, (HL)", [this]{ADD_8BIT_NO_CARRY(a, get_hl_value()); return 8;}}},
            {0xC6, {"ADD A, N", [this]{ADD_8BIT_NO_CARRY(a, get_immediate_byte()); return 8;}}},
            {0x8F, {"ADDC A, A", [this]{ADD_8BIT_CARRY(a, a); return 4;}}},
            {0x88, {"ADDC A, B", [this]{ADD_8BIT_CARRY(a, b); return 4;}}},
            {0x89, {"ADDC A, C", [this]{ADD_8BIT_CARRY(   a, c); return 4;}}},
            {0x8A, {"ADDC A, D", [this]{ADD_8BIT_CARRY(a, d); return 4;}}},
            {0x8B, {"ADDC A, E", [this]{ADD_8BIT_CARRY(a, e); return 4;}}},
            {0x8C, {"ADDC A, H", [this]{ADD_8BIT_CARRY(a, h); return 4;}}},
            {0x8D, {"ADDC A, L", [this]{ADD_8BIT_CARRY(a, l); return 4;}}},
            {0x8E, {"ADDC A, (HL)", [this]{ADD_8BIT_CARRY(a, get_hl_value()); return 8;}}},
            {0xCE, {"ADDC A, N", [this]{ADD_8BIT_CARRY(a, get_immediate_byte()); return 8;}}},
            /// Subs from A
            {0x97, {"SUB A, A", [this]{SUB_8BIT_NO_CARRY(a, a); return 4;}}},
            {0x90, {"SUB A, B", [this]{SUB_8BIT_NO_CARRY(a, b); return 4;}}},
            {0x91, {"SUB A, C", [this]{SUB_8BIT_NO_CARRY(a, c); return 4;}}},
            {0x92, {"SUB A, D", [this]{SUB_8BIT_NO_CARRY(a, d); return 4;}}},
            {0x93, {"SUB A, E", [this]{SUB_8BIT_NO_CARRY(a, e); return 4;}}},
            {0x94, {"SUB A, H", [this]{SUB_8BIT_NO_CARRY(a, h); return 4;}}},
            {0x95, {"SUB A, L", [this]{SUB_8BIT_NO_CARRY(a, l); return 4;}}},
            {0x96, {"SUB A, (HL)", [this]{SUB_8BIT_NO_CARRY(a, get_hl_value()); return 8;}}},
            {0xD6, {"SUB A, N", [this]{SUB_8BIT_NO_CARRY(a, get_immediate_byte()); return 8;}}},
            {0xDE, {"SUBC, A, n", [this]{SUB_8BIT_CARRY(a, get_immediate_byte()); return 8;}}},
            {0x9F, {"SUBC A, A", [this]{SUB_8BIT_CARRY(a, a); return 4;}}},
            {0x98, {"SUBC A, B", [this]{SUB_8BIT_CARRY(a, b); return 4;}}},
            {0x99, {"SUBC A, C", [this]{SUB_8BIT_CARRY(a, c); return 4;}}},
            {0x9A, {"SUBC A, D", [this]{SUB_8BIT_CARRY(a, d); return 4;}}},
            {0x9B, {"SUBC A, E", [this]{SUB_8BIT_CARRY(a, e); return 4;}}},
            {0x9C, {"SUBC A, H", [this]{SUB_8BIT_CARRY(a, h); return 4;}}},
            {0x9D, {"SUBC A, L", [this]{SUB_8BIT_CARRY(a, l); return 4;}}},
            {0x9E, {"SUBC A, (HL)", [this]{SUB_8BIT_CARRY(a, get_hl_value()); return 8;}}},
//            {0xD6, [this]{SUB_8BIT_CARRY(a, get_immediate_byte()); return 8;}}, // unknown opcode
            /// AND with A
            {0xA7, {"AND A, A", [this]{AND_8BIT(a, a); return 4;}}},
            {0xA0, {"AND A, B", [this]{AND_8BIT(a, b); return 4;}}},
            {0xA1, {"AND A, C", [this]{AND_8BIT(a, c); return 4;}}},
            {0xA2, {"AND A, D", [this]{AND_8BIT(a, d); return 4;}}},
            {0xA3, {"AND A, E", [this]{AND_8BIT(a, e); return 4;}}},
            {0xA4, {"AND A, H", [this]{AND_8BIT(a, h); return 4;}}},
            {0xA5, {"AND A, L", [this]{AND_8BIT(a, l); return 4;}}},
            {0xA6, {"AND A, (HL)", [this]{AND_8BIT(a, get_hl_value()); return 8;}}},
            {0xE6, {"AND A, n", [this]{AND_8BIT(a, get_immediate_byte()); return 8;}}},
            /// OR with A
            {0xB7, {"OR A, A", [this]{OR_8BIT(a, a); return 4;}}},
            {0xB0, {"OR A, B", [this]{OR_8BIT(a, b); return 4;}}},
            {0xB1, {"OR A, C", [this]{OR_8BIT(a, c); return 4;}}},
            {0xB2, {"OR A, D", [this]{OR_8BIT(a, d); return 4;}}},
            {0xB3, {"OR A, E", [this]{OR_8BIT(a, e); return 4;}}},
            {0xB4, {"OR A, H", [this]{OR_8BIT(a, h); return 4;}}},
            {0xB5, {"OR A, L", [this]{OR_8BIT(a, l); return 4;}}},
            {0xB6, {"OR A, (HL)", [this]{OR_8BIT(a, get_hl_value()); return 8;}}},
            {0xF6, {"OR A, N", [this]{OR_8BIT(a, get_immediate_byte()); return 8;}}},
            /// XOR with A
            {0xAF, {"XOR A, A", [this]{XOR_8BIT(a, a); return 4;}}},
            {0xA8, {"XOR A, B", [this]{XOR_8BIT(a, b); return 4;}}},
            {0xA9, {"XOR A, C", [this]{XOR_8BIT(a, c); return 4;}}},
            {0xAA, {"XOR A, D", [this]{XOR_8BIT(a, d); return 4;}}},
            {0xAB, {"XOR A, E", [this]{XOR_8BIT(a, e); return 4;}}},
            {0xAC, {"XOR A, H", [this]{XOR_8BIT(a, h); return 4;}}},
            {0xAD, {"XOR A, L", [this]{XOR_8BIT(a, l); return 4;}}},
            {0xAE, {"XOR A, (HL)", [this]{XOR_8BIT(a, get_hl_value()); return 8;}}},
            {0xEE, {"XOR A, N", [this]{XOR_8BIT(a, get_immediate_byte()); return 8;}}},
            /// Compare with A
            {0xBF, {"CP A, A", [this]{COMPARE_8BIT(a, a); return 4;}}},
            {0xB8, {"CP A, B", [this]{COMPARE_8BIT(a, b); return 4;}}},
            {0xB9, {"CP A, C", [this]{COMPARE_8BIT(a, c); return 4;}}},
            {0xBA, {"CP A, D", [this]{COMPARE_8BIT(a, d); return 4;}}},
            {0xBB, {"CP A, E", [this]{COMPARE_8BIT(a, e); return 4;}}},
            {0xBC, {"CP A, H", [this]{COMPARE_8BIT(a, h); return 4;}}},
            {0xBD, {"CP A, L", [this]{COMPARE_8BIT(a, l); return 4;}}},
            {0xBE, {"CP A, (HL)", [this]{COMPARE_8BIT(a, get_hl_value()); return 8;}}},
            {0xFE, {"CP A, N", [this]{COMPARE_8BIT(a, get_immediate_byte()); return 8;}}},
            /////////////////// Increments to registers
            {0x3C, {"INC A", [this]{INCREMENT_8BIT(a); return 4;}}},
            {0x04, {"INC B", [this]{INCREMENT_8BIT(b); return 4;}}},
            {0x0C, {"INC C", [this]{INCREMENT_8BIT(c); return 4;}}},
            {0x14, {"INC D", [this]{INCREMENT_8BIT(d); return 4;}}},
            {0x1C, {"INC E", [this]{INCREMENT_8BIT(e); return 4;}}},
            {0x24, {"INC H", [this]{INCREMENT_8BIT(h); return 4;}}},
            {0x2C, {"INC L", [this]{INCREMENT_8BIT(l); return 4;}}},
            {0x34, {"INC (HL)", [this]{INCREMENT_8BIT_AT_ADDR(get_hl()); return 12;}}},
            /////////////////// Decrements to registers
            {0x3D, {"DEC A", [this]{DECREMENT_8BIT(a); return 4;}}},
            {0x05, {"DEC B", [this]{DECREMENT_8BIT(b); return 4;}}},
            {0x0D, {"DEC C", [this]{DECREMENT_8BIT(c); return 4;}}},
            {0x15, {"DEC D", [this]{DECREMENT_8BIT(d); return 4;}}},
            {0x1D, {"DEC E", [this]{DECREMENT_8BIT(e); return 4;}}},
            {0x25, {"DEC H", [this]{DECREMENT_8BIT(h); return 4;}}},
            {0x2D, {"DEC L", [this]{DECREMENT_8BIT(l); return 4;}}},
            {0x35, {"DEC (HL)", [this]{DECREMENT_8BIT_AT_ADDR(get_hl()); return 12;}}},

            //////////////////////////////////////////////////////// 16-bit ALU ops
            // need to use lambda to bind member functions
            // Not very readable, essentially we are passing in pointers to
            // getter and setters of our 16 bit registers
            /// Adds to HL
            {0x09, {"ADD HL, BC", [this]{ADD_16BIT([this]{return get_hl();}, [this](uint16_t x){set_hl(x);}, get_bc()); return 8;}}},
            {0x19, {"ADD HL, DE", [this]{ADD_16BIT([this]{return get_hl();}, [this](uint16_t x){set_hl(x);}, get_de()); return 8;}}},
            {0x29, {"ADD HL, HL", [this]{ADD_16BIT([this]{return get_hl();}, [this](uint16_t x){set_hl(x);}, get_hl()); return 8;}}},
            {0x39, {"ADD HL, SP", [this]{ADD_16BIT([this]{return get_hl();}, [this](uint16_t x){set_hl(x);}, sp); return 8;}}},
            /// Adds to SP
            {0xE8, {"ADD SP, n", [this]{ADD_16BIT_SP(); return 16;}}},
            /// Increments on reg pairs
            {0x03, {"INC BC", [this]{INCREMENT_16BIT([this]{return get_bc();}, [this](uint16_t x){set_bc(x);}); return 8;}}},
            {0x13, {"INC DE", [this]{INCREMENT_16BIT([this]{return get_de();}, [this](uint16_t x){set_de(x);}); return 8;}}},
            {0x23, {"INC HL", [this]{INCREMENT_16BIT([this]{return get_hl();}, [this](uint16_t x){set_hl(x);}); return 8;}}},
            {0x33, {"INC SP", [this]{INCREMENT_16BIT([this]{return get_sp();}, [this](uint16_t x){set_sp(x);}); return 8;}}},
            /// Decrements on reg pairs
            {0x0B, {"DEC BC", [this]{DECREMENT_16BIT([this]{return get_bc();}, [this](uint16_t x){set_bc(x);}); return 8;}}},
            {0x1B, {"DEC DE", [this]{DECREMENT_16BIT([this]{return get_de();}, [this](uint16_t x){set_de(x);}); return 8;}}},
            {0x2B, {"DEC HL", [this]{DECREMENT_16BIT([this]{return get_hl();}, [this](uint16_t x){set_hl(x);}); return 8;}}},
            {0x3B, {"DEC SP", [this]{DECREMENT_16BIT([this]{return get_sp();}, [this](uint16_t x){set_sp(x);}); return 8;}}},
            //////////////////////////////////////////////////////// Misc operations
            {0x27, {"DAA", [this]{DECIMAL_ADJUST(a); return 4;}}},
            {0x2F, {"~A", [this]{COMPLEMENT_8BIT(a); return 4;}}},
            {0x3F, {"~CARRY", [this]{COMPLEMENT_CARRY(); return 4;}}},
            {0x37, {"SET_CARRY", [this]{SET_CARRY(); return 4;}}},
            {0x00, {"NOP", [this]{NOP(); return 4;}}},
            {0x76, {"HALT", [this]{HALT(); return 4;}}},
            {0xF3, {"DISABLE INTERRUPTS", [this]{this->interrupt_handler->disable(); return 4;}}},
            {0xFB, {"ENABLE INTERRUPTS", [this]{this->interrupt_handler->enable(); return 4;}}},
            /// Bit rotates
            {0x07, {"RLC A", [this]{ROTATE_LEFT(a); set_flag(FLAG_Z, false); return 4;}}},
            {0x17, {"RL A", [this]{ROTATE_LEFT_CARRY_TO_0(a); set_flag(FLAG_Z, false); return 4;}}},
            {0x0F, {"RRC A", [this]{ROTATE_RIGHT(a); set_flag(FLAG_Z, false); return 4;}}},
            {0x1F, {"RR A", [this]{ROTATE_RIGHT_CARRY_TO_7(a); set_flag(FLAG_Z, false); return 4;}}},
            /// Jumps
            {0xC3, {"JUMP nn", [this]{JUMP_TO_IMMEDIATE_WORD(); return 16;}}},
            {0xC2, {"JUMP NZ, nn", [this]{return CONDITIONAL_JUMP_TO_IMMEDIATE_WORD(Z_UNSET); }}},
            {0xCA, {"JUMP Z, nn", [this]{return CONDITIONAL_JUMP_TO_IMMEDIATE_WORD(Z_SET); }}},
            {0xD2, {"JUMP NC, nn", [this]{return CONDITIONAL_JUMP_TO_IMMEDIATE_WORD(C_UNSET);}}},
            {0xDA, {"JUMP C, nn", [this]{return CONDITIONAL_JUMP_TO_IMMEDIATE_WORD(C_SET);}}},
            {0xE9, {"JUMP HL", [this]{JUMP(get_hl()); return 4;}}},
            {0x18, {"JUMP BY n", [this]{JUMP_BY_IMMEDIATE_BYTE(); return 12;}}},
            {0x20, {"JUMP BY NZ, n", [this]{return CONDITIONAL_JUMP_BY_IMMEDIATE_BYTE(Z_UNSET);}}},
            {0x28, {"JUMP BY Z, n", [this]{return CONDITIONAL_JUMP_BY_IMMEDIATE_BYTE(Z_SET);}}},
            {0x30, {"JUMP BY NC, n", [this]{return CONDITIONAL_JUMP_BY_IMMEDIATE_BYTE(C_UNSET);}}},
            {0x38, {"JUMP BY C, n", [this]{return CONDITIONAL_JUMP_BY_IMMEDIATE_BYTE(C_SET);}}},
            {0xCD, {"CALL n", [this]{CALL_IMMEDIATE(); return 24;}}},
            {0xC4, {"CALL NZ, n", [this]{return CONDITIONAL_CALL_IMMEDIATE(Z_UNSET);}}},
            {0xCC, {"CALL Z, n", [this]{return CONDITIONAL_CALL_IMMEDIATE(Z_SET);}}},
            {0xD4, {"CALL NC, n", [this]{return CONDITIONAL_CALL_IMMEDIATE(C_UNSET);}}},
            {0xDC, {"CALL C, n", [this]{return CONDITIONAL_CALL_IMMEDIATE(C_SET);}}},
            /// Restarts
            {0xC7, {"RESET 0x00", [this]{RESET(0x00); return 16;}}},
            {0xCF, {"RESET 0x08", [this]{RESET(0x08); return 16;}}},
            {0xD7, {"RESET 0x10", [this]{RESET(0x10); return 16;}}},
            {0xDF, {"RESET 0x18", [this]{RESET(0x18); return 16;}}},
            {0xE7, {"RESET 0x20", [this]{RESET(0x20); return 16;}}},
            {0xEF, {"RESET 0x28", [this]{RESET(0x28); return 16;}}},
            {0xF7, {"RESET 0x30", [this]{RESET(0x30); return 16;}}},
            {0xFF, {"RESET 0x38", [this]{RESET(0x38); return 16;}}},
            /// Returns
            {0xC9, {"RETURN ", [this]{RETURN_FROM_STACK(); return 16;}}},
            {0xC0, {"RETURN NZ", [this]{return CONDITIONAL_RETURN_FROM_STACK(Z_UNSET);}}},
            {0xC8, {"RETURN Z", [this]{return CONDITIONAL_RETURN_FROM_STACK(Z_SET);}}},
            {0xD0, {"RETURN NC", [this]{return CONDITIONAL_RETURN_FROM_STACK(C_UNSET);}}},
            {0xD8, {"RETURN C", [this]{return CONDITIONAL_RETURN_FROM_STACK(C_SET);}}},
            {0xD9, {"RETURN WITH INTERRUPTS", [this]{RETURN_FROM_STACK_WITH_INTERRUPTS(); return 16;}}},
            // Extended ops
            { 0xCB, {"extended op handler", [this]{return handle_extended_op();}}},
            {0x10, {"STOP", [this]{return 4;}}},
    };
    // If opcode is CB,
    extended_opcode_table = {
            /// SWAP
            {0x37, {"SWAP A", [this]{SWAP(a); return 8;}}},
            {0x30, {"SWAP B", [this]{SWAP(b); return 8;}}},
            {0x31, {"SWAP C", [this]{SWAP(c); return 8;}}},
            {0x32, {"SWAP D", [this]{SWAP(d); return 8;}}},
            {0x33, {"SWAP E", [this]{SWAP(e); return 8;}}},
            {0x34, {"SWAP H", [this]{SWAP(h); return 8;}}},
            {0x35, {"SWAP L", [this]{SWAP(l); return 8;}}},
            {0x36, {"SWAP (HL)", [this]{apply_op_to_addr(get_hl(), [this](uint8_t &x){SWAP(x);}); return 16;}}},
            /////////////////// Bit Rotates
            /// Left Rotates
            {0x07, {"RLC A", [this]{ROTATE_LEFT(a); return 8;}}},
            {0x00, {"RLC B", [this]{ROTATE_LEFT(b); return 8;}}},
            {0x01, {"RLC C", [this]{ROTATE_LEFT(c); return 8;}}},
            {0x02, {"RLC D", [this]{ROTATE_LEFT(d); return 8;}}},
            {0x03, {"RLC E", [this]{ROTATE_LEFT(e); return 8;}}},
            {0x04, {"RLC H", [this]{ROTATE_LEFT(h); return 8;}}},
            {0x05, {"RLC L", [this]{ROTATE_LEFT(l); return 8;}}},
            {0x06, {"RLC (HL)", [this]{apply_op_to_addr(get_hl(), [this](uint8_t &x){ROTATE_LEFT(x);}); return 16;}}},
            {0x17, {"RL A", [this]{ROTATE_LEFT_CARRY_TO_0(a);  return 8;}}},
            {0x10, {"RL B", [this]{ROTATE_LEFT_CARRY_TO_0(b); return 8;}}},
            {0x11, {"RL C", [this]{ROTATE_LEFT_CARRY_TO_0(c); return 8;}}},
            {0x12, {"RL D", [this]{ROTATE_LEFT_CARRY_TO_0(d); return 8;}}},
            {0x13, {"RL E", [this]{ROTATE_LEFT_CARRY_TO_0(e); return 8;}}},
            {0x14, {"RL H", [this]{ROTATE_LEFT_CARRY_TO_0(h); return 8;}}},
            {0x15, {"RL L", [this]{ROTATE_LEFT_CARRY_TO_0(l); return 8;}}},
            {0x16, {"RL (HL)", [this]{apply_op_to_addr(get_hl(), [this](uint8_t &x){ROTATE_LEFT_CARRY_TO_0(x);}); return 16;}}},
            /// Right Rotates
            {0x0F, {"RRC A", [this]{ROTATE_RIGHT(a); return 8;}}},
            {0x08, {"RRC B", [this]{ROTATE_RIGHT(b); return 8;}}},
            {0x09, {"RRC C", [this]{ROTATE_RIGHT(c); return 8;}}},
            {0x0A, {"RRC D", [this]{ROTATE_RIGHT(d); return 8;}}},
            {0x0B, {"RRC E", [this]{ROTATE_RIGHT(e); return 8;}}},
            {0x0C, {"RRC H", [this]{ROTATE_RIGHT(h); return 8;}}},
            {0x0D, {"RRC L", [this]{ROTATE_RIGHT(l); return 8;}}},
            {0x0E, {"RRC (HL)", [this]{apply_op_to_addr(get_hl(), [this](uint8_t &x){ROTATE_RIGHT(x);}); return 16;}}},
            {0x1F, {"RR A", [this]{ROTATE_RIGHT_CARRY_TO_7(a); return 8;}}},
            {0x18, {"RR B", [this]{ROTATE_RIGHT_CARRY_TO_7(b); return 8;}}},
            {0x19, {"RR C", [this]{ROTATE_RIGHT_CARRY_TO_7(c); return 8;}}},
            {0x1A, {"RR D", [this]{ROTATE_RIGHT_CARRY_TO_7(d); return 8;}}},
            {0x1B, {"RR E", [this]{ROTATE_RIGHT_CARRY_TO_7(e); return 8;}}},
            {0x1C, {"RR H", [this]{ROTATE_RIGHT_CARRY_TO_7(h); return 8;}}},
            {0x1D, {"RR L", [this]{ROTATE_RIGHT_CARRY_TO_7(l); return 8;}}},
            {0x1E, {"RR (HL)", [this]{apply_op_to_addr(get_hl(), [this](uint8_t &x){ROTATE_RIGHT_CARRY_TO_7(x);}); return 16;}}},
            /// Shift left
            {0x27, {"SL A", [this]{SHIFT_LEFT(a); return 8;}}},
            {0x20, {"SL B", [this]{SHIFT_LEFT(b); return 8;}}},
            {0x21, {"SL C", [this]{SHIFT_LEFT(c); return 8;}}},
            {0x22, {"SL D", [this]{SHIFT_LEFT(d); return 8;}}},
            {0x23, {"SL E", [this]{SHIFT_LEFT(e); return 8;}}},
            {0x24, {"SL H", [this]{SHIFT_LEFT(h); return 8;}}},
            {0x25, {"SL L", [this]{SHIFT_LEFT(l); return 8;}}},
            {0x26, {"SL (HL)", [this]{apply_op_to_addr(get_hl(), [this](uint8_t &x){SHIFT_LEFT(x);}); return 16;}}},
            /// Shift right no msb change
            {0x2F, {"SR A", [this]{SHIFT_RIGHT(a); return 8;}}},
            {0x28, {"SR B", [this]{SHIFT_RIGHT(b); return 8;}}},
            {0x29, {"SR C", [this]{SHIFT_RIGHT(c); return 8;}}},
            {0x2A, {"SR D", [this]{SHIFT_RIGHT(d); return 8;}}},
            {0x2B, {"SR E", [this]{SHIFT_RIGHT(e); return 8;}}},
            {0x2C, {"SR H", [this]{SHIFT_RIGHT(h); return 8;}}},
            {0x2D, {"SR L", [this]{SHIFT_RIGHT(l); return 8;}}},
            {0x2E, {"SR (HL)", [this]{apply_op_to_addr(get_hl(), [this](uint8_t &x){SHIFT_RIGHT(x);}); return 16;}}},
            /// Shift right MSB 0
            {0x3F, {"SR MSB 0 A", [this]{SHIFT_RIGHT_MSB_0(a); return 8;}}},
            {0x38, {"SR MSB 0 B", [this]{SHIFT_RIGHT_MSB_0(b); return 8;}}},
            {0x39, {"SR MSB 0 C", [this]{SHIFT_RIGHT_MSB_0(c); return 8;}}},
            {0x3A, {"SR MSB 0 D", [this]{SHIFT_RIGHT_MSB_0(d); return 8;}}},
            {0x3B, {"SR MSB 0 E", [this]{SHIFT_RIGHT_MSB_0(e); return 8;}}},
            {0x3C, {"SR MSB 0 H", [this]{SHIFT_RIGHT_MSB_0(h); return 8;}}},
            {0x3D, {"SR MSB 0 L", [this]{SHIFT_RIGHT_MSB_0(l); return 8;}}},
            {0x3E, {"SR MSB 0 (HL)", [this]{apply_op_to_addr(get_hl(), [this](uint8_t &x){SHIFT_RIGHT_MSB_0(x);}); return 16;}}},
            ////// Bit ops
            /// test bit
            {0x40, {"TEST B, 0", [this]{TEST_BIT(b, 0); return 8;}}},
            {0x41, {"TEST C, 0", [this]{TEST_BIT(c, 0); return 8;}}},
            {0x42, {"TEST D, 0", [this]{TEST_BIT(d, 0); return 8;}}},
            {0x43, {"TEST E, 0", [this]{TEST_BIT(e, 0); return 8;}}},
            {0x44, {"TEST H, 0", [this]{TEST_BIT(h, 0); return 8;}}},
            {0x45, {"TEST L, 0", [this]{TEST_BIT(l, 0); return 8;}}},
            {0x46, {"TEST (HL), 0", [this]{TEST_BIT(this->mmu->read_byte(get_hl()), 0); return 16;}}},
            {0x47, {"TEST A, 0", [this]{TEST_BIT(a, 0); return 8;}}},
            {0x48, {"TEST B, 1", [this]{TEST_BIT(b, 1); return 8;}}},
            {0x49, {"TEST C, 1", [this]{TEST_BIT(c, 1); return 8;}}},
            {0x4A, {"TEST D, 1", [this]{TEST_BIT(d, 1); return 8;}}},
            {0x4B, {"TEST E, 1", [this]{TEST_BIT(e, 1); return 8;}}},
            {0x4C, {"TEST H, 1", [this]{TEST_BIT(h, 1); return 8;}}},
            {0x4D, {"TEST L, 1", [this]{TEST_BIT(l, 1); return 8;}}},
            {0x4E, {"TEST (HL), 1", [this]{TEST_BIT(this->mmu->read_byte(get_hl()), 1); return 16;}}},
            {0x4F, {"TEST A, 1", [this]{TEST_BIT(a, 1); return 8;}}},
            {0x50, {"TEST B, 2", [this]{TEST_BIT(b, 2); return 8;}}},
            {0x51, {"TEST C, 2", [this]{TEST_BIT(c, 2); return 8;}}},
            {0x52, {"TEST D, 2", [this]{TEST_BIT(d, 2); return 8;}}},
            {0x53, {"TEST E, 2", [this]{TEST_BIT(e, 2); return 8;}}},
            {0x54, {"TEST H, 2", [this]{TEST_BIT(h, 2); return 8;}}},
            {0x55, {"TEST L, 2", [this]{TEST_BIT(l, 2); return 8;}}},
            {0x56, {"TEST (HL), 2", [this]{TEST_BIT(this->mmu->read_byte(get_hl()), 2); return 16;}}},
            {0x57, {"TEST A, 2", [this]{TEST_BIT(a, 2); return 8;}}},
            {0x58, {"TEST B, 3", [this]{TEST_BIT(b, 3); return 8;}}},
            {0x59, {"TEST C, 3", [this]{TEST_BIT(c, 3); return 8;}}},
            {0x5A, {"TEST D, 3", [this]{TEST_BIT(d, 3); return 8;}}},
            {0x5B, {"TEST E, 3", [this]{TEST_BIT(e, 3); return 8;}}},
            {0x5C, {"TEST H, 3", [this]{TEST_BIT(h, 3); return 8;}}},
            {0x5D, {"TEST L, 3", [this]{TEST_BIT(l, 3); return 8;}}},
            {0x5E, {"TEST (HL), 3", [this]{TEST_BIT(this->mmu->read_byte(get_hl()), 3); return 16;}}},
            {0x5F, {"TEST A, 3", [this]{TEST_BIT(a, 3); return 8;}}},
            {0x60, {"TEST B, 4", [this]{TEST_BIT(b, 4); return 8;}}},
            {0x61, {"TEST C, 4", [this]{TEST_BIT(c, 4); return 8;}}},
            {0x62, {"TEST D, 4", [this]{TEST_BIT(d, 4); return 8;}}},
            {0x63, {"TEST E, 4", [this]{TEST_BIT(e, 4); return 8;}}},
            {0x64, {"TEST H, 4", [this]{TEST_BIT(h, 4); return 8;}}},
            {0x65, {"TEST L, 4", [this]{TEST_BIT(l, 4); return 8;}}},
            {0x66, {"TEST (HL), 4", [this]{TEST_BIT(this->mmu->read_byte(get_hl()), 4); return 16;}}},
            {0x67, {"TEST A, 4", [this]{TEST_BIT(a, 4); return 8;}}},
            {0x68, {"TEST B, 5", [this]{TEST_BIT(b, 5); return 8;}}},
            {0x69, {"TEST C, 5", [this]{TEST_BIT(c, 5); return 8;}}},
            {0x6A, {"TEST D, 5", [this]{TEST_BIT(d, 5); return 8;}}},
            {0x6B, {"TEST E, 5", [this]{TEST_BIT(e, 5); return 8;}}},
            {0x6C, {"TEST H, 5", [this]{TEST_BIT(h, 5); return 8;}}},
            {0x6D, {"TEST L, 5", [this]{TEST_BIT(l, 5); return 8;}}},
            {0x6E, {"TEST (HL), 5", [this]{TEST_BIT(this->mmu->read_byte(get_hl()), 5); return 16;}}},
            {0x6F, {"TEST A, 5", [this]{TEST_BIT(a, 5); return 8;}}},
            {0x70, {"TEST B, 6", [this]{TEST_BIT(b, 6); return 8;}}},
            {0x71, {"TEST C, 6", [this]{TEST_BIT(c, 6); return 8;}}},
            {0x72, {"TEST D, 6", [this]{TEST_BIT(d, 6); return 8;}}},
            {0x73, {"TEST E, 6", [this]{TEST_BIT(e, 6); return 8;}}},
            {0x74, {"TEST H, 6", [this]{TEST_BIT(h, 6); return 8;}}},
            {0x75, {"TEST L, 6", [this]{TEST_BIT(l, 6); return 8;}}},
            {0x76, {"TEST (HL), 6", [this]{TEST_BIT(this->mmu->read_byte(get_hl()), 6); return 16;}}},
            {0x77, {"TEST A, 6", [this]{TEST_BIT(a, 6); return 8;}}},
            {0x78, {"TEST B, 7", [this]{TEST_BIT(b, 7); return 8;}}},
            {0x79, {"TEST C, 7", [this]{TEST_BIT(c, 7); return 8;}}},
            {0x7A, {"TEST D, 7", [this]{TEST_BIT(d, 7); return 8;}}},
            {0x7B, {"TEST E, 7", [this]{TEST_BIT(e, 7); return 8;}}},
            {0x7C, {"TEST H, 7", [this]{TEST_BIT(h, 7); return 8;}}},
            {0x7D, {"TEST L, 7", [this]{TEST_BIT(l, 7); return 8;}}},
            {0x7E, {"TEST (HL), 7", [this]{TEST_BIT(this->mmu->read_byte(get_hl()), 7); return 16;}}},
            {0x7F, {"TEST A, 7", [this]{TEST_BIT(a, 7); return 8;}}},

            /// Reset bit
            {0x80, {"RESET B, 0", [this]{RESET_BIT(b, 0); return 8;}}},
            {0x81, {"RESET C, 0", [this]{RESET_BIT(c, 0); return 8;}}},
            {0x82, {"RESET D, 0", [this]{RESET_BIT(d, 0); return 8;}}},
            {0x83, {"RESET E, 0", [this]{RESET_BIT(e, 0); return 8;}}},
            {0x84, {"RESET H, 0", [this]{RESET_BIT(h, 0); return 8;}}},
            {0x85, {"RESET L, 0", [this]{RESET_BIT(l, 0); return 8;}}},
            {0x86, {"RESET (HL), 0", [this]{RESET_BIT_FROM_ADDR(get_hl(), 0); return 16;}}},
            {0x87, {"RESET A, 0", [this]{RESET_BIT(a, 0); return 8;}}},
            {0x88, {"RESET B, 1", [this]{RESET_BIT(b, 1); return 8;}}},
            {0x89, {"RESET C, 1", [this]{RESET_BIT(c, 1); return 8;}}},
            {0x8A, {"RESET D, 1", [this]{RESET_BIT(d, 1); return 8;}}},
            {0x8B, {"RESET E, 1", [this]{RESET_BIT(e, 1); return 8;}}},
            {0x8C, {"RESET H, 1", [this]{RESET_BIT(h, 1); return 8;}}},
            {0x8D, {"RESET L, 1", [this]{RESET_BIT(l, 1); return 8;}}},
            {0x8E, {"RESET (HL), 1", [this]{RESET_BIT_FROM_ADDR(get_hl(), 1); return 16;}}},
            {0x8F, {"RESET A, 1", [this]{RESET_BIT(a, 1); return 8;}}},
            {0x90, {"RESET B, 2", [this]{RESET_BIT(b, 2); return 8;}}},
            {0x91, {"RESET C, 2", [this]{RESET_BIT(c, 2); return 8;}}},
            {0x92, {"RESET D, 2", [this]{RESET_BIT(d, 2); return 8;}}},
            {0x93, {"RESET E, 2", [this]{RESET_BIT(e, 2); return 8;}}},
            {0x94, {"RESET H, 2", [this]{RESET_BIT(h, 2); return 8;}}},
            {0x95, {"RESET L, 2", [this]{RESET_BIT(l, 2); return 8;}}},
            {0x96, {"RESET (HL), 2", [this]{RESET_BIT_FROM_ADDR(get_hl(), 2); return 16;}}},
            {0x97, {"RESET A, 2", [this]{RESET_BIT(a, 2); return 8;}}},
            {0x98, {"RESET B, 3", [this]{RESET_BIT(b, 3); return 8;}}},
            {0x99, {"RESET C, 3", [this]{RESET_BIT(c, 3); return 8;}}},
            {0x9A, {"RESET D, 3", [this]{RESET_BIT(d, 3); return 8;}}},
            {0x9B, {"RESET E, 3", [this]{RESET_BIT(e, 3); return 8;}}},
            {0x9C, {"RESET H, 3", [this]{RESET_BIT(h, 3); return 8;}}},
            {0x9D, {"RESET L, 3", [this]{RESET_BIT(l, 3); return 8;}}},
            {0x9E, {"RESET (HL), 3", [this]{RESET_BIT_FROM_ADDR(get_hl(), 3); return 16;}}},
            {0x9F, {"RESET A, 3", [this]{RESET_BIT(a, 3); return 8;}}},
            {0xA0, {"RESET B, 4", [this]{RESET_BIT(b, 4); return 8;}}},
            {0xA1, {"RESET C, 4", [this]{RESET_BIT(c, 4); return 8;}}},
            {0xA2, {"RESET D, 4", [this]{RESET_BIT(d, 4); return 8;}}},
            {0xA3, {"RESET E, 4", [this]{RESET_BIT(e, 4); return 8;}}},
            {0xA4, {"RESET H, 4", [this]{RESET_BIT(h, 4); return 8;}}},
            {0xA5, {"RESET L, 4", [this]{RESET_BIT(l, 4); return 8;}}},
            {0xA6, {"RESET (HL), 4", [this]{RESET_BIT_FROM_ADDR(get_hl(), 4); return 16;}}},
            {0xA7, {"RESET A, 4", [this]{RESET_BIT(a, 4); return 8;}}},
            {0xA8, {"RESET B, 5", [this]{RESET_BIT(b, 5); return 8;}}},
            {0xA9, {"RESET C, 5", [this]{RESET_BIT(c, 5); return 8;}}},
            {0xAA, {"RESET D, 5", [this]{RESET_BIT(d, 5); return 8;}}},
            {0xAB, {"RESET E, 5", [this]{RESET_BIT(e, 5); return 8;}}},
            {0xAC, {"RESET H, 5", [this]{RESET_BIT(h, 5); return 8;}}},
            {0xAD, {"RESET L, 5", [this]{RESET_BIT(l, 5); return 8;}}},
            {0xAE, {"RESET (HL), 5", [this]{RESET_BIT_FROM_ADDR(get_hl(), 5); return 16;}}},
            {0xAF, {"RESET A, 5", [this]{RESET_BIT(a, 5); return 8;}}},
            {0xB0, {"RESET B, 6", [this]{RESET_BIT(b, 6); return 8;}}},
            {0xB1, {"RESET C, 6", [this]{RESET_BIT(c, 6); return 8;}}},
            {0xB2, {"RESET D, 6", [this]{RESET_BIT(d, 6); return 8;}}},
            {0xB3, {"RESET E, 6", [this]{RESET_BIT(e, 6); return 8;}}},
            {0xB4, {"RESET H, 6", [this]{RESET_BIT(h, 6); return 8;}}},
            {0xB5, {"RESET L, 6", [this]{RESET_BIT(l, 6); return 8;}}},
            {0xB6, {"RESET (HL), 6", [this]{RESET_BIT_FROM_ADDR(get_hl(), 6); return 16;}}},
            {0xB7, {"RESET A, 6", [this]{RESET_BIT(a, 6); return 8;}}},
            {0xB8, {"RESET B, 7", [this]{RESET_BIT(b, 7); return 8;}}},
            {0xB9, {"RESET C, 7", [this]{RESET_BIT(c, 7); return 8;}}},
            {0xBA, {"RESET D, 7", [this]{RESET_BIT(d, 7); return 8;}}},
            {0xBB, {"RESET E, 7", [this]{RESET_BIT(e, 7); return 8;}}},
            {0xBC, {"RESET H, 7", [this]{RESET_BIT(h, 7); return 8;}}},
            {0xBD, {"RESET L, 7", [this]{RESET_BIT(l, 7); return 8;}}},
            {0xBE, {"RESET (HL), 7", [this]{RESET_BIT_FROM_ADDR(get_hl(), 7); return 16;}}},
            {0xBF, {"RESET A, 7", [this]{RESET_BIT(a, 7); return 8;}}},

            /// Set bit
            {0xC0, {"SET B, 0", [this]{SET_BIT(b, 0); return 8;}}},
            {0xC1, {"SET C, 0", [this]{SET_BIT(c, 0); return 8;}}},
            {0xC2, {"SET D, 0", [this]{SET_BIT(d, 0); return 8;}}},
            {0xC3, {"SET E, 0", [this]{SET_BIT(e, 0); return 8;}}},
            {0xC4, {"SET H, 0", [this]{SET_BIT(h, 0); return 8;}}},
            {0xC5, {"SET L, 0", [this]{SET_BIT(l, 0); return 8;}}},
            {0xC6, {"SET (HL), 0", [this]{SET_BIT_FROM_ADDR(get_hl(), 0); return 16;}}},
            {0xC7, {"SET A, 0", [this]{SET_BIT(a, 0); return 8;}}},
            {0xC8, {"SET B, 1", [this]{SET_BIT(b, 1 ); return 8;}}},
            {0xC9, {"SET C, 1", [this]{SET_BIT(c, 1); return 8;}}},
            {0xCA, {"SET D, 1", [this]{SET_BIT(d, 1); return 8;}}},
            {0xCB, {"SET E, 1", [this]{SET_BIT(e, 1); return 8;}}},
            {0xCC, {"SET H, 1", [this]{SET_BIT(h, 1); return 8;}}},
            {0xCD, {"SET L, 1", [this]{SET_BIT(l, 1); return 8;}}},
            {0xCE, {"SET (HL), 1", [this]{SET_BIT_FROM_ADDR(get_hl(), 1); return 16;}}},
            {0xCF, {"SET A, 1", [this]{SET_BIT(a, 1); return 8;}}},
            {0xD0, {"SET B, 2", [this]{SET_BIT(b, 2); return 8;}}},
            {0xD1, {"SET C, 2", [this]{SET_BIT(c, 2); return 8;}}},
            {0xD2, {"SET D, 2", [this]{SET_BIT(d, 2); return 8;}}},
            {0xD3, {"SET E, 2", [this]{SET_BIT(e, 2); return 8;}}},
            {0xD4, {"SET H, 2", [this]{SET_BIT(h, 2); return 8;}}},
            {0xD5, {"SET L, 2", [this]{SET_BIT(l, 2); return 8;}}},
            {0xD6, {"SET (HL), 2", [this]{SET_BIT_FROM_ADDR(get_hl(), 2); return 16;}}},
            {0xD7, {"SET A, 2", [this]{SET_BIT(a, 2); return 8;}}},
            {0xD8, {"SET B, 3", [this]{SET_BIT(b, 3); return 8;}}},
            {0xD9, {"SET C, 3", [this]{SET_BIT(c, 3); return 8;}}},
            {0xDA, {"SET D, 3", [this]{SET_BIT(d, 3); return 8;}}},
            {0xDB, {"SET E, 3", [this]{SET_BIT(e, 3); return 8;}}},
            {0xDC, {"SET H, 3", [this]{SET_BIT(h, 3); return 8;}}},
            {0xDD, {"SET L, 3", [this]{SET_BIT(l, 3); return 8;}}},
            {0xDE, {"SET (HL), 3", [this]{SET_BIT_FROM_ADDR(get_hl(), 3); return 16;}}},
            {0xDF, {"SET A, 3", [this]{SET_BIT(a, 3); return 8;}}},
            {0xE0, {"SET B, 4", [this]{SET_BIT(b, 4); return 8;}}},
            {0xE1, {"SET C, 4", [this]{SET_BIT(c, 4); return 8;}}},
            {0xE2, {"SET D, 4", [this]{SET_BIT(d, 4); return 8;}}},
            {0xE3, {"SET E, 4", [this]{SET_BIT(e, 4); return 8;}}},
            {0xE4, {"SET H, 4", [this]{SET_BIT(h, 4); return 8;}}},
            {0xE5, {"SET L, 4", [this]{SET_BIT(l, 4); return 8;}}},
            {0xE6, {"SET (HL), 4", [this]{SET_BIT_FROM_ADDR(get_hl(), 4); return 16;}}},
            {0xE7, {"SET A, 4", [this]{SET_BIT(a, 4); return 8;}}},
            {0xE8, {"SET B, 5", [this]{SET_BIT(b, 5); return 8;}}},
            {0xE9, {"SET C, 5", [this]{SET_BIT(c, 5); return 8;}}},
            {0xEA, {"SET D, 5", [this]{SET_BIT(d, 5); return 8;}}},
            {0xEB, {"SET E, 5", [this]{SET_BIT(e, 5); return 8;}}},
            {0xEC, {"SET H, 5", [this]{SET_BIT(h, 5); return 8;}}},
            {0xED, {"SET L, 5", [this]{SET_BIT(l, 5); return 8;}}},
            {0xEE, {"SET (HL), 5", [this]{SET_BIT_FROM_ADDR(get_hl(), 5); return 16;}}},
            {0xEF, {"SET A, 5", [this]{SET_BIT(a, 5); return 8;}}},
            {0xF0, {"SET B, 6", [this]{SET_BIT(b, 6); return 8;}}},
            {0xF1, {"SET C, 6", [this]{SET_BIT(c, 6); return 8;}}},
            {0xF2, {"SET D, 6", [this]{SET_BIT(d, 6); return 8;}}},
            {0xF3, {"SET E, 6", [this]{SET_BIT(e, 6); return 8;}}},
            {0xF4, {"SET H, 6", [this]{SET_BIT(h, 6); return 8;}}},
            {0xF5, {"SET L, 6", [this]{SET_BIT(l, 6); return 8;}}},
            {0xF6, {"SET (HL), 6", [this]{SET_BIT_FROM_ADDR(get_hl(), 6); return 16;}}},
            {0xF7, {"SET A, 6", [this]{SET_BIT(a, 6); return 8;}}},
            {0xF8, {"SET B, 7", [this]{SET_BIT(b, 7); return 8;}}},
            {0xF9, {"SET C, 7", [this]{SET_BIT(c, 7); return 8;}}},
            {0xFA, {"SET D, 7", [this]{SET_BIT(d, 7); return 8;}}},
            {0xFB, {"SET E, 7", [this]{SET_BIT(e, 7); return 8;}}},
            {0xFC, {"SET H, 7", [this]{SET_BIT(h, 7); return 8;}}},
            {0xFD, {"SET L, 7", [this]{SET_BIT(l, 7); return 8;}}},
            {0xFE, {"SET (HL), 7", [this]{SET_BIT_FROM_ADDR(get_hl(), 7); return 16;}}},
            {0xFF, {"SET A, 7", [this]{SET_BIT(a, 7); return 8;}}},
    };
}

/////////////////////////////////////////////// Operations

void CPU::LOAD_IMMEDIATE(uint8_t &reg) {
    reg = get_immediate_byte();
}

void CPU::LOAD(uint8_t &reg, uint8_t value) {
    reg = value;
}

void CPU::LOAD_TO_ADDR(uint16_t addr, uint8_t value) {
    mmu->write_byte(addr, value);
}

void CPU::LOAD_TO_ADDR(uint16_t addr, uint16_t value) {
    mmu->write_word(addr, value);
}

void CPU::LOAD_TO_ADDR_IMMEDIATE(uint16_t addr) {
    uint8_t immediate = get_immediate_byte();
    mmu->write_byte(addr, immediate);
}

void CPU::PUSH(uint16_t value) {
    mmu->push_word_to_stack(value);
}

uint16_t CPU::POP_WORD() {
    return mmu->pop_word_from_stack();
}

/// 8-bit ALU ops
void CPU::_ADD_8BIT(uint8_t &reg, uint8_t value, uint8_t carry_flag) {
    bool should_carry = (reg + value + carry_flag) > 0xFF;
    bool should_half_carry = ((reg & 0xF) + (value & 0xF) + carry_flag) > 0xF;
    reg = (reg + value + carry_flag) & 0xFF;
    bool should_set_zero = reg == 0;
    set_flag(FLAG_C, should_carry);
    set_flag(FLAG_H, should_half_carry);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, should_set_zero);
}

void CPU::ADD_8BIT_CARRY(uint8_t &reg, uint8_t value) {
    _ADD_8BIT(reg, value, get_flag(FLAG_C));
}

void CPU::ADD_8BIT_NO_CARRY(uint8_t &reg, uint8_t value) {
    _ADD_8BIT(reg, value, 0);
}

void CPU::_SUB_8BIT(uint8_t &reg, uint8_t value, uint8_t carry_flag) {
    bool should_borrow = reg < (value + carry_flag);
    bool should_half_borrow = (reg & 0xF) < ((value & 0xF) + carry_flag);
    reg = reg - (value + carry_flag);
    set_flag(FLAG_C, should_borrow);
    set_flag(FLAG_H, should_half_borrow);
    set_flag(FLAG_N, true);
    set_flag(FLAG_Z, reg == 0);
}

void CPU::SUB_8BIT_CARRY(uint8_t &reg, uint8_t value) {
    _SUB_8BIT(reg, value, get_flag(FLAG_C));
}

void CPU::SUB_8BIT_NO_CARRY(uint8_t &reg, uint8_t value) {
    _SUB_8BIT(reg, value, 0);

}

void CPU::AND_8BIT(uint8_t &reg, uint8_t value) {
    reg &= value;
    bool should_set_zero = reg == 0;
    set_flag(FLAG_C, false);
    set_flag(FLAG_H, true);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, should_set_zero);
}

void CPU::OR_8BIT(uint8_t &reg, uint8_t value) {
    reg |= value;
    set_flag(FLAG_C, false);
    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, reg == 0);
}

void CPU::XOR_8BIT(uint8_t &reg, uint8_t value) {
    reg ^= value;
    set_flag(FLAG_C, false);
    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, reg == 0);
}

void CPU::COMPARE_8BIT(uint8_t reg, uint8_t value) {
    uint8_t res = static_cast<uint8_t>(reg - value);
    set_flag(FLAG_Z, res == 0);
    set_flag(FLAG_N, true);
    set_flag(FLAG_H, ((reg & 0xf) - (value & 0xf) ) < 0);
    set_flag(FLAG_C, reg < value);
}

void CPU::INCREMENT_8BIT(uint8_t &reg) {
    // same as adding 1, except carry is not affected
    reg ++;
    set_flag(FLAG_Z, reg == 0);
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, (reg & 0xF) == 0x0);
}

void CPU::INCREMENT_8BIT_AT_ADDR(uint16_t addr){
    uint8_t value = mmu->read_byte(addr);
    value++;
    mmu->write_byte(addr, value);
    set_flag(FLAG_H, (value & 0xF) == 0x0);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, value == 0);
}

void CPU::DECREMENT_8BIT(uint8_t &reg) {
    // same as subbing 1, except carry is not affected
    reg --;
    set_flag(FLAG_Z, reg == 0);
    set_flag(FLAG_N, true);
    set_flag(FLAG_H, (reg & 0xF) == 0xF);
}

void CPU::DECREMENT_8BIT_AT_ADDR(uint16_t addr){
    uint8_t value = mmu->read_byte(addr);
    value --;
    mmu->write_byte(addr, value);
    set_flag(FLAG_H, (value & 0xF) == 0xF);
    set_flag(FLAG_N, true);
    set_flag(FLAG_Z, value == 0);
}

/// 16-bit ALU ops
void CPU::ADD_16BIT(const std::function<uint16_t()>& get, const std::function<void(uint16_t)>& set, uint16_t value) {
    uint16_t reg = get();
    bool should_half_carry = ((reg & 0xFFF) + (value & 0xFFF)) > 0xFFF;
    bool should_carry = (reg + value) > 0xFFFF;
    reg = (reg + value) & 0xFFFF;
    set(reg);
    set_flag(FLAG_C, should_carry);
    set_flag(FLAG_H, should_half_carry);
    set_flag(FLAG_N, false);
}

void CPU::ADD_16BIT_SP() {
    uint16_t reg = sp;
    int8_t value = static_cast<int8_t>(get_immediate_byte());
    bool should_half_carry = ((reg ^ value ^ ((reg + value) & 0xFFFF)) & 0x10) == 0x10;
    bool should_carry = (((reg ^ value ^ ((reg + value) & 0xFFFF)) & 0x100) == 0x100);
    reg = (reg + value) & 0xFFFF;
    sp = reg;
    set_flag(FLAG_C, should_carry);
    set_flag(FLAG_H, should_half_carry);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, false);
}

void CPU::INCREMENT_16BIT(const std::function<uint16_t()>& get, const std::function<void(uint16_t)>& set) {
    set(get() + 1);
}

void CPU::DECREMENT_16BIT(const std::function<uint16_t()>& get, const std::function<void(uint16_t)>& set) {
    set(get() - 1);
}

// Swapping the upper and lower nibbles of an 8bit value
void CPU::SWAP(uint8_t &reg) {
    uint8_t bottom_4  = reg & 0b1111;
    reg = reg >> 4; // move top 4 to the position of bottom four
    reg |= bottom_4 << 4; // move bottom four to place of top four
    bool should_set_zero = reg == 0;
    set_flag(FLAG_C, false);
    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, should_set_zero);
}

// Adjusts reg so the correct repr of binary coded decimal is obtained
// Runs after an add or subtract, to bring the reg to be properly coded as a BCD.
// Basically, if a half carry occurs, we need to add/sub 6 to the lower nibble
// If a carry occurs, we need to add/sub 6 to the upper nibble
// see: https://forums.nesdev.com/viewtopic.php?t=15944 for reference,
// borrowed the code direct from there
void CPU::DECIMAL_ADJUST(uint8_t &reg) {
    if (get_flag(FLAG_N)) { // subtraction case
        if (get_flag(FLAG_C)) {
            reg -= 0x60;
        }
        if (get_flag(FLAG_H)) {
            reg -= 0x6;
        }
    } else { // addition case
        if (get_flag(FLAG_C) || reg > 0x99) {
            reg += 0x60;
            set_flag(FLAG_C, true);
        }
        if (get_flag(FLAG_H) || (reg & 0x0F) > 0x09) {
            reg += 0x6;
        }
    }
    bool should_set_zero = reg == 0;
    set_flag(FLAG_H, false);
    set_flag(FLAG_Z, should_set_zero);
}

void CPU::COMPLEMENT_8BIT(uint8_t &reg) {
    reg = ~reg;
    set_flag(FLAG_N, true);
    set_flag(FLAG_H, true);
}

void CPU::COMPLEMENT_CARRY() {
    set_flag(FLAG_C, !get_flag(FLAG_C));
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, false);
}

void CPU::SET_CARRY() {
    set_flag(FLAG_C, true);
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, false);
}

void CPU::NOP() {}

void CPU::HALT() {
    is_halted = true;
}

void CPU::STOP() {} // TODO: this


/*
 * rotate left, and put carry to bit 0
 */
void CPU::ROTATE_LEFT_CARRY_TO_0(uint8_t &reg) {
    bool bit_7 = get_bit(reg, 7);
    reg = reg << 1;
    reg |= get_flag(FLAG_C);
    set_flag(FLAG_C, bit_7);
    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, reg == 0);
}

/*
 * rotate left, and put bit 7 to bit 0
 */
void CPU::ROTATE_LEFT(uint8_t &reg) {
    bool bit_7 = get_bit(reg, 7);
    reg = reg << 1;
    reg |= bit_7;
    set_flag(FLAG_C, bit_7);
    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, reg == 0);
}

/*
 * rotate right, and put carry to bit 7
 */
void CPU::ROTATE_RIGHT_CARRY_TO_7(uint8_t &reg) {
    bool bit_0 = get_bit(reg, 0);
    reg = reg >> 1;
    reg |= get_flag(FLAG_C) << 7;
    set_flag(FLAG_C, bit_0);
    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, reg == 0);
}

/*
 * rotate right, and put bit 0 to bit 7
 */
void CPU::ROTATE_RIGHT(uint8_t &reg) {
    bool bit_0 = get_bit(reg, 0);
    reg = reg >> 1;
    reg |= bit_0 << 7;
    set_flag(FLAG_C, bit_0);
    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, reg == 0);
}


/*
 * Shift left into carry, LSB set to zero
 */
void CPU::SHIFT_LEFT(uint8_t &reg) {
    bool bit_7 = get_bit(reg, 7);
    reg = reg << 1;
    set_flag(FLAG_C, bit_7);
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, false);
    set_flag(FLAG_Z, reg == 0);
}


/*
 * Shift right into carry, MSB unchanged
 */
void CPU::SHIFT_RIGHT(uint8_t &reg) {
    bool bit_7 = get_bit(reg, 7);
    bool bit_0 = get_bit(reg, 0);
    reg = reg >> 1;
    bit_7 ? set_bit(reg, 7) : unset_bit(reg, 7);
    set_flag(FLAG_C, bit_0);
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, false);
    set_flag(FLAG_Z, reg == 0);
}

/*
 * Shift right into carry, MSB set to zero
 */
void CPU::SHIFT_RIGHT_MSB_0(uint8_t &reg) {
    bool bit_0 = get_bit(reg, 0);
    reg = reg >> 1;
    set_flag(FLAG_C, bit_0);
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, false);
    set_flag(FLAG_Z, reg == 0);
}

void CPU::TEST_BIT(uint8_t reg, uint8_t bit) {
    set_flag(FLAG_Z, !get_bit(reg, bit));
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, true);
}

void CPU::SET_BIT(uint8_t &reg, uint8_t bit) {
    set_bit(reg, bit);
}

void CPU::SET_BIT_FROM_ADDR(uint16_t addr, uint8_t bit) {
    uint8_t bit_to_set = mmu->read_byte(addr);
    SET_BIT(bit_to_set, bit);
    mmu->write_byte(addr, bit_to_set);
}

void CPU::RESET_BIT(uint8_t &reg, uint8_t bit) {
    unset_bit(reg, bit);
}

void CPU::RESET_BIT_FROM_ADDR(uint16_t addr, uint8_t bit) {
    uint8_t bit_to_reset = mmu->read_byte(addr);
    unset_bit(bit_to_reset, bit);
    mmu->write_byte(addr, bit_to_reset);
}


void CPU::JUMP(uint16_t addr) {
    pc = addr;
}

void CPU::JUMP_TO_IMMEDIATE_WORD() {
    uint16_t jump_addr = mmu->read_word(pc);
    pc = jump_addr;
}

uint8_t CPU::CONDITIONAL_JUMP_TO_IMMEDIATE_WORD(JUMP_CONDITION cond){
    uint16_t jump_addr = get_immediate_word();
    switch(cond) {
        case Z_UNSET:
            if (!get_flag(FLAG_Z)) {
                pc = jump_addr;
                return 16;
            }
            break;
        case Z_SET:
            if (get_flag(FLAG_Z)) {
                pc = jump_addr;
                return 16;
            }
            break;
        case C_UNSET:
            if (!get_flag(FLAG_C)) {
                pc = jump_addr;
                return 16;
            }
            break;
        case C_SET:
            if (get_flag(FLAG_C)) {
                pc = jump_addr;
                return 16;
            }
            break;
    }
    return 12;
}

void CPU::JUMP_BY_IMMEDIATE_BYTE() { // For these jumps the immediate byte is read as signed
    int8_t signed_byte = static_cast<int8_t>(get_immediate_byte());
    pc = static_cast<uint16_t>(pc + signed_byte);
}

uint8_t CPU::CONDITIONAL_JUMP_BY_IMMEDIATE_BYTE(JUMP_CONDITION cond) {
    int8_t signed_byte = static_cast<int8_t>(get_immediate_byte());
    switch(cond) {
        case Z_UNSET:
            if (!get_flag(FLAG_Z)) {
                pc = static_cast<uint16_t>(pc +signed_byte);
                return 12;
            }
            break;
        case Z_SET:
            if (get_flag(FLAG_Z)) {
                pc = static_cast<uint16_t>(pc +signed_byte);
                return 12;
            }
            break;
        case C_UNSET:
            if (!get_flag(FLAG_C)) {
                pc = static_cast<uint16_t>(pc +signed_byte);
                return 12;
            }
            break;
        case C_SET:
            if (get_flag(FLAG_C)) {
                pc = static_cast<uint16_t>(pc +signed_byte);
                return 12;
            }
            break;
    }
    return 8;
}

void CPU::CALL_IMMEDIATE() {
    uint16_t addr = get_immediate_word();
    mmu->push_word_to_stack(pc);
    pc = addr;
}

uint8_t CPU::CONDITIONAL_CALL_IMMEDIATE(JUMP_CONDITION cond) {
    uint16_t addr = get_immediate_word();
    switch(cond) {
        case Z_UNSET:
            if (!get_flag(FLAG_Z)) {
                mmu->push_word_to_stack(pc);
                pc = addr;
                return 24;
            }
            break;
        case Z_SET:
            if (get_flag(FLAG_Z)) {
                mmu->push_word_to_stack(pc);
                pc = addr;
                return 24;
            }
            break;
        case C_UNSET:
            if (!get_flag(FLAG_C)) {
                mmu->push_word_to_stack(pc);
                pc = addr;
                return 24;
            }
            break;
        case C_SET:
            if (get_flag(FLAG_C)) {
                mmu->push_word_to_stack(pc);
                pc = addr;
                return 24;
            }
            break;
    }
    return 12;
}

void CPU::RESET(uint8_t reset_location){
    mmu->push_word_to_stack(pc);
    pc = reset_location;
}

void CPU::RETURN_FROM_STACK() {
    pc = mmu->pop_word_from_stack();
}

uint8_t CPU::CONDITIONAL_RETURN_FROM_STACK(JUMP_CONDITION cond){
    switch(cond) {
        case Z_UNSET:
            if (!get_flag(FLAG_Z)) {
                pc = mmu->pop_word_from_stack();
                return 20;
            }
            break;
        case Z_SET:
            if (get_flag(FLAG_Z)) {
                pc = mmu->pop_word_from_stack();
                return 20;
            }
            break;
        case C_UNSET:
            if (!get_flag(FLAG_C)) {
                pc = mmu->pop_word_from_stack();
                return 20;
            }
            break;
        case C_SET:
            if (get_flag(FLAG_C)) {
                pc = mmu->pop_word_from_stack();
                return 20;
            }
            break;
    }
    return 8;
}

void CPU::RETURN_FROM_STACK_WITH_INTERRUPTS() {
    RETURN_FROM_STACK();
    interrupt_handler->enable();
}