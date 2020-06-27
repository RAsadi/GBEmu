//
// Created by Rayan Asadi on 2020-06-30.
//

#ifndef GBEMU_ENUMS_H
#define GBEMU_ENUMS_H

enum BANK_MODE {
    NONE,
    MBC1,
    MBC2,
    MBC3
};

enum INTERRUPT_TYPE {
    V_BLANK_INTERRUPT,
    LCD_INTERRUPT,
    TIMER_INTERRUPT,
    JOYPAD_INTERRUPT
};

enum LCD_MODE {
    ACCESS_OAM,
    ACCESS_VRAM,
    H_BLANK,
    V_BLANK,
};

enum KEY {
    KEY_A,
    KEY_B,
    KEY_START,
    KEY_SELECT,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT
};

enum JUMP_CONDITION {
    Z_SET,
    Z_UNSET,
    C_SET,
    C_UNSET
};

#endif //GBEMU_ENUMS_H
