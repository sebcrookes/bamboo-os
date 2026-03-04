#include "kernel.h"

#include "inc/stdio.h"

#include "test.h"

#include "mem/memory.h"
#include "acpi/acpi.h"
#include "mem/heap.h"

#include "mem/pmm.h"

#include "pci/pci.h"

#include "int/int.h"

void kernel_init(boot_data_t* bootdata) {
    kernel_print_logo();
    printf("\n");

    memory_init(bootdata);

    heap_init((void*)0xFFFFFFF000000000);

    test_vmm();
    
    acpi_init(bootdata);

    int_init();
    
    pci_init();
    pci_init_builtin_drivers();

    printf("\n%C[Kernel]%C - Thanks for trying out BambooOS! There is currently no way to run programs. Learn more about the OS at https://github.com/sebcrookes/bamboo-os.", COLOUR_KERNEL_INFO, COLOUR_PRINT);
}

void kernel_print_logo() {
    uint32_t c = COLOUR_KERNEL_INFO;
    printf("%C  _____                                           _____   _____ \n", c);
    c += 0x1000;
    printf("%C |  _  \\  _____   _   _   _____   _____   _____  |  _  | |  ___|\n", c);
    c += 0x1000;
    printf("%C | |_| | |  _  | | \\_/ | |  _  | |  _  | |  _  | | | | | | |___ \n", c);
    c += 0x1000;
    printf("%C |  _ <  | |_| | |  _  | | |_) / | | | | | | | | | | | | |___  |\n", c);
    c += 0x1000;
    printf("%C | |_| | |  _  | | | | | | |_) \\ | |_| | | |_| | | |_| |  ___| |\n", c);
    c += 0x1000;
    printf("%C |_____/ |_| |_| |_| |_| |_____| |_____| |_____| |_____| |_____|\n", c);
}
