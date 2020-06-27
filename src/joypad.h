//
// Created by Rayan Asadi on 2020-07-03.
//

#ifndef GBEMU_JOYPAD_H
#define GBEMU_JOYPAD_H

#include <cstdint>
#include "utils/enums.h"

class MMU;
class InterruptHandler;

class Joypad {

    /* According to pandocs, on read from the joypad register,
     * the games expect bits 1-4 of the returned byte to hold the status of either
     * the direction or button keys, depending on which is selected by bits 4 and 5
     *
     * The returned byte has the following format:
     * Bit 7 - Not used
     * Bit 6 - Not used
     * Bit 5 - P15 Select Button Keys (0=Select)
     * Bit 4 - P14 Select Direction Keys (0=Select)
     * Bit 3 - P13 Input Down or Start (0=Pressed) (Read Only)
     * Bit 2 - P12 Input Up or Select (0=Pressed) (Read Only)
     * Bit 1 - P11 Input Left or Button B (0=Pressed) (Read Only)
     * Bit 0 - P10 Input Right or Button A (0=Pressed) (Read Only)
     *
     * To represent the internal state of which buttons are pressed, we will use an array of 8 bits,
     * for each of the 8 keys. We need to store this separate from the actual state that is read by the games,
     * since buttons and directions overlap as shown above
     */
    bool joypad_state[8];
    MMU *mmu;
    InterruptHandler *interrupt_handler;
    bool is_dir = false;
    bool is_button = false;
public:
    Joypad(MMU *mmu, InterruptHandler *interrupt_handler);
    uint8_t get_state() const;
    void press_key(KEY key);
    void release_key(KEY key);
    void write(uint8_t byte);
};


#endif //GBEMU_JOYPAD_H
