//
// Created by Rayan Asadi on 2020-06-30.
//

#ifndef GBEMU_INTERRUPT_HANDLER_H
#define GBEMU_INTERRUPT_HANDLER_H

#include "utils/enums.h"

class MMU;
class CPU;
/*
 * The gameboy has 4 interrupts which need to be handled, which correspond to the following bit
 * flags set on the interrupt_request_register
 * bit 0 - V-Blank interrupt
 * bit 1 - LCD interrupt
 * bit 2 - Timer interrupt
 * bit 4 - Joypad interrupt
 * This order also represents order of priority when handling interrupts.
 *
 * The interrupt handler will manage requests to handle interrupts of these four kinds
 */
class InterruptHandler {
public:
    bool interrupts_are_enabled;
    MMU *mmu;
    CPU *cpu;
    void handle_interrupts(INTERRUPT_TYPE type);
    InterruptHandler(MMU *mmu, CPU *cpu);
    void request_interrupt(INTERRUPT_TYPE type);
    void handle_interrupts();
    void enable();
    void disable();
};


#endif //GBEMU_INTERRUPT_HANDLER_H
