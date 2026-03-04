#include "madt.h"

#include "../inc/stdio.h"

sdt_hdr_t* hdr;
lapic_bsp_t* bsp_lapic;

void acpi_parse_madt() {
    hdr = acpi_get_table("APIC");
}
