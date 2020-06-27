//
// Created by Rayan Asadi on 2020-07-03.
//

#include "joypad.h"
#include <unordered_map>
#include "memory/mmu.h"
#include "interrupt_handler.h"
#include "utils/bit_utils.h"

std::unordered_map<KEY, uint8_t> KEY_BIT_MAP = {
        {KEY_RIGHT,  0},
        {KEY_LEFT,   1},
        {KEY_UP,     2},
        {KEY_DOWN,   3},
        {KEY_A,      4},
        {KEY_B,      5},
        {KEY_SELECT,  6},
        {KEY_START, 7}
};

Joypad::Joypad(MMU *mmu, InterruptHandler *interrupt_handler) {
    this->interrupt_handler = interrupt_handler;
    this->mmu = mmu;
}

uint8_t Joypad::get_state() const {
    /*
     * The value of the internal state is a bit weird, check the header file for more info
     */
    uint8_t state = 0b1111;
    if (is_dir) {
        for (int i = 0; i < 4; i++) {
            if (joypad_state[i])
                unset_bit(state, i);
        }
    } else {
        set_bit(state, 4);
    }

    if (is_button) {
        for (int i = 4; i < 8; i++) {
            if (joypad_state[i])
                unset_bit(state, i - 4);
        }
    } else {
        set_bit(state, 5);
    }
    return state;
}

void Joypad::write(uint8_t byte) {
    is_dir = !get_bit(byte, 4);
    is_button = !get_bit(byte, 5);
}

void Joypad::press_key(KEY key) {
    joypad_state[KEY_BIT_MAP[key]] = true;
    interrupt_handler->request_interrupt(JOYPAD_INTERRUPT);

}

void Joypad::release_key(KEY key) {
    joypad_state[KEY_BIT_MAP[key]] = false;
    interrupt_handler->request_interrupt(JOYPAD_INTERRUPT);
}