//
// Created by Rayan Asadi on 2020-07-02.
//

#ifndef GBEMU_BIT_UTILS_H
#define GBEMU_BIT_UTILS_H
#include <cstdint>

bool get_bit(uint8_t byte, uint8_t bit);
void set_bit(uint8_t &byte, uint8_t bit);
void unset_bit(uint8_t &byte, uint8_t bit);

#endif //GBEMU_BIT_UTILS_H
