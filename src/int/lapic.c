#include "lapic.h"

#include <cpuid.h>

#include "../inc/stdio.h"
#include "../inc/vector.h"

#include "../acpi/madt.h"
#include "../mem/vmm.h"

static vector_t* lapics;
static uint64_t lapic_base_addr;

void lapic_init() {
    lapics = acpi_get_lapics();
    lapic_base_addr = acpi_get_lapic_addr();

    for(int i = 0; i < vector_size(lapics); i++) {
        madt_local_apic_t* lapic = (madt_local_apic_t*) vector_get(lapics, i);
        //printf("%d\n", lapic->apic_id);
    }

    // Identity mapping the LAPIC base address
    vmm_map_page_flags((void*) lapic_base_addr, (void*) lapic_base_addr, PT_ENTRY_READ_WRITE | PT_ENTRY_CACHE_DISABLED);
}

uint8_t lapic_get_cpu_num() {
    uint32_t ebx, unused;
    __get_cpuid(1, &unused, &ebx, &unused, &unused);

    return (ebx >> 24) & 0xFF;
}
