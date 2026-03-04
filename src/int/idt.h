#pragma once

#include <stdint.h>

#include "int.h"

#define IDT_INTERRUPT_GATE 0b1110
#define IDT_TRAP_GATE 0b1111

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

typedef struct __attribute__((packed)) {
    idt_entry_t descriptors[256];
} idt_t;

void idt_init();
void idt_set_handler(uint8_t vector, void* handler, uint8_t gate_type);
void idt_install();

extern void _idt_load(void* idtr);
