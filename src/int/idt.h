#pragma once

#include <stdint.h>

#include "int.h"

#define IDT_INTERRUPT_GATE 0b1110
#define IDT_TRAP_GATE 0b1111

/**
 * Struct for a single entry in the IDT.
 */
typedef struct __attribute__((packed)) {
    uint16_t offset0;
    uint16_t segment_selector;
    
    struct __attribute__((packed)) {
        uint8_t ist : 3;
        uint8_t reserved0 : 5;
        uint8_t gate_type : 4;
        uint8_t reserved1 : 1;
        uint8_t dpl : 2;
        uint8_t present : 1;
    } bits;

    uint16_t offset1;
    uint32_t offset2;
    uint32_t reserved;
} idt_entry_t;

/**
 * Struct for IDT, containing 256 entries.
 */
typedef struct __attribute__((packed)) {
    idt_entry_t descriptors[256];
} idt_t;

/**
 * Allocates memory for and sets up the IDT and IDTR structures.
 */
void idt_init();

/**
 * Sets the ISR for a given interrupt vector.
 * 
 * @param vector the interrupt vector the handler is for
 * @param handler the ISR to be called
 * @param gate_type the type of gate for that interrupt (IDT_INTERRUPT_GATE or IDT_TRAP_GATE)
 */
void idt_set_handler(uint8_t vector, void* handler, uint8_t gate_type);

/**
 * Loads the IDTR into the CPUs internal registers.
 */
void idt_install();

/**
 * Assembly subroutine which runs the 'lidt' instruction.
 * 
 * @param idtr the IDTR to be loaded into the CPU
 */
extern void _idt_load(void* idtr);
