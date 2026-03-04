#include "int.h"

#include "../mem/pmm.h"
#include "../mem/vmm.h"

#include "isr.h"
#include "idt.h"

/**
 * Initialises and configures interrupt handling.
 */
void int_init() {
    asm("cli");

    idt_init();

    for(int i = 0; i < 256; i++) {
        idt_set_handler(i, &isr_default_handler, 0xE);
    }

    idt_set_handler(0, &isr_div_by_zero_handler, 0xE);
    idt_set_handler(0x0D, &isr_gp_fault_handler, 0xE);
    idt_set_handler(0x0E, &isr_page_fault_handler, 0xE);

    idt_install();

    asm("sti");
}
