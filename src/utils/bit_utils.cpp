//
// Created by Rayan Asadi on 2020-07-13.
//

#include "bit_utils.h"

bool get_bit(uint8_t byte, uint8_t bit) {
    return (byte >> bit) & 0b1;
}

void set_bit(uint8_t &byte, uint8_t bit) {
    byte |= (0b1 << bit);
}

void unset_bit(uint8_t &byte, uint8_t bit) {
    byte &= ~(0b1 << bit);
}