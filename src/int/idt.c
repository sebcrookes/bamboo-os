#include "idt.h"

#include "../mem/pmm.h"

idt_t* idt;
void* idtr;

/**
 * Allocates memory for and sets up the IDT and IDTR structures.
 */
void idt_init() {
    idt = (idt_t*) pmm_alloc();
    idtr = (void*) pmm_alloc();

    // Setting fields in the IDTR using pointer arithmetic as the compiler pads Intel's 6-byte structures
    uint16_t* idtr_size = (uint16_t*)(idtr);
    uint64_t* idtr_base = (uint64_t*)(idtr_size + 1);

    *idtr_size = (256 * sizeof(idt_entry_t)) - 1;
    *idtr_base = (uint64_t) idt;
}

/**
 * Sets the ISR for a given interrupt vector.
 * 
 * @param vector the interrupt vector the handler is for
 * @param handler the ISR to be called
 * @param gate_type the type of gate for that interrupt (IDT_INTERRUPT_GATE or IDT_TRAP_GATE)
 */
void idt_set_handler(uint8_t vector, void* handler, uint8_t gate_type) {
    idt_entry_t* entry = &idt->descriptors[vector];

    uint64_t isr_addr = (uint64_t) handler;

    // Setting the address of the ISR
    entry->offset0 = isr_addr & ((uint64_t) 0xFFFF);
    entry->offset1 = (isr_addr >> 16) & ((uint64_t) 0xFFFF);
    entry->offset2 = (isr_addr >> 32) & ((uint64_t) 0xFFFFFFFF);

    entry->segment_selector = 5 << 3; // With Limine, the "64-bit kernel code" segment is the 5th (and shift by 3 to get segment selector)

    entry->bits.ist = 0;
    entry->bits.dpl = 0;

    entry->bits.gate_type = gate_type;
    entry->bits.present = 1;
}

/**
 * Loads the IDTR into the CPUs internal registers.
 */
void idt_install() {
    _idt_load((void*) idtr);
}
