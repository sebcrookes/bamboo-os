#include "int.h"

#include "../mem/pmm.h"
#include "../mem/vmm.h"

#include "../inc/stdio.h"

#include "isr.h"
#include "idt.h"

#include "lapic.h"

void int_init() {
    asm("cli");

    //printf("%C[Interrupts]%C - Setting up interrupts...\n", COLOUR_KERNEL_INFO, COLOUR_PRINT);

    idt_init();

    for(int i = 0; i < 256; i++) {
        idt_set_handler(i, &isr_default_handler, 0xE);
    }

    idt_set_handler(0, &isr_div_by_zero_handler, 0xE);
    idt_set_handler(0x0D, &isr_gp_fault_handler, 0xE);
    idt_set_handler(0x0E, &isr_page_fault_handler, 0xE);

    idt_install();

    lapic_init();

    printf("%C[Interrupts]%C - Interrupts installed\n", COLOUR_KERNEL_INFO, COLOUR_PRINT);

    asm("sti");
}
