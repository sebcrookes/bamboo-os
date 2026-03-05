#include "madt.h"

#include "../inc/stdio.h"
#include "../inc/vector.h"
#include "../inc/string.h"
#include "../mem/heap.h"

sdt_hdr_t* hdr;
uint64_t local_apic_bsp_addr;

vector_t* lapics;
vector_t* ioapics;

vector_t* interrupt_source_overrides;
vector_t* interrupt_ioapic_nmis;
vector_t* interrupt_lapic_nmis;

bool acpi_parse_madt() {
    // Getting the "APIC" (MADT) table
    hdr = acpi_get_table("APIC");
    if(hdr == NULL) return false;

    lapics = vector_init();
    ioapics = vector_init();

    interrupt_source_overrides = vector_init();
    interrupt_ioapic_nmis = vector_init();
    interrupt_lapic_nmis = vector_init();

    // Getting the address of the BSPs local APIC (this may be overridden via a type 5 entry)
    madt_local_apic_bsp_t* lapic_bsp = (madt_local_apic_bsp_t*)(hdr + 1);
    local_apic_bsp_addr = lapic_bsp->address;

    // Looping over each entry in the table
    uint64_t i = sizeof(sdt_hdr_t) + sizeof(madt_local_apic_bsp_t);
    while(i < hdr->length) {
        madt_entry_info_t* entry_info = (madt_entry_info_t*)(((uint8_t*) hdr) + i);

        switch(entry_info->type) {
            case 0: { // Processor Local APIC
                madt_local_apic_t* lapic = (madt_local_apic_t*)(entry_info + 1);

                // Making a copy and adding it to the list
                madt_local_apic_t* copy = (madt_local_apic_t*) malloc(sizeof(madt_local_apic_t));
                memcpy((void*) copy, (void*) lapic, sizeof(madt_local_apic_t));

                vector_add(lapics, (void*) copy);
            } break;

            case 1: { // I/O APIC
                madt_ioapic_t* ioapic = (madt_ioapic_t*)(entry_info + 1);

                // Making a copy and adding it to the list
                madt_ioapic_t* copy = (madt_ioapic_t*) malloc(sizeof(madt_ioapic_t));
                memcpy((void*) copy, (void*) ioapic, sizeof(madt_ioapic_t));
                
                vector_add(ioapics, (void*) copy);
            } break;

            case 2: { // I/O APIC Interrupt Source Override
                madt_int_src_override_t* int_src_override = (madt_int_src_override_t*)(entry_info + 1);

                madt_int_src_override_t* copy = (madt_int_src_override_t*) malloc(sizeof(madt_int_src_override_t));
                memcpy((void*) copy, (void*) int_src_override, sizeof(madt_int_src_override_t));

                vector_add(interrupt_source_overrides, (void*) copy);
            } break;

            case 3: { // I/O APIC NMI Source
                madt_ioapic_nmi_src_t* nmi_src = (madt_ioapic_nmi_src_t*)(entry_info + 1);

                madt_ioapic_nmi_src_t* copy = (madt_ioapic_nmi_src_t*) malloc(sizeof(madt_ioapic_nmi_src_t));
                memcpy((void*) copy, (void*) nmi_src, sizeof(madt_ioapic_nmi_src_t));
                
                vector_add(interrupt_ioapic_nmis, (void*) copy);
            } break;

            case 4: { // Local APIC NMI
                madt_lapic_nmi_t* nmi = (madt_lapic_nmi_t*)(entry_info + 1);

                madt_lapic_nmi_t* copy = (madt_lapic_nmi_t*) malloc(sizeof(madt_lapic_nmi_t));
                memcpy((void*) copy, (void*) nmi, sizeof(madt_lapic_nmi_t));
                
                vector_add(interrupt_lapic_nmis, (void*) copy);
            } break;

            case 5: { // Local APIC Address Override
                madt_lapic_addr_override_t* override = (madt_lapic_addr_override_t*)(entry_info + 1);

                local_apic_bsp_addr = override->address;
            } break;

            case 9: { // Processor Local x2APIC
            } break;

            case 10: { // Local x2APIC NMI
            } break;

            default: {
                printf("%C[MADT]%C - Error - encountered entry of type %d\n", COLOUR_KERNEL_INFO, COLOUR_PRINT, entry_info->type);
                return false;
            } break;
        }

        i += entry_info->length;
    }

    // Pretty-printer to tell the user how many I/O APICs and Local APICs there are
    uint64_t num_ioapics = vector_size(ioapics);
    uint64_t num_lapics = vector_size(lapics);

    printf("%C[MADT]%C - Found %d I/O APIC", COLOUR_KERNEL_INFO, COLOUR_PRINT, num_ioapics);

    if(num_ioapics > 1) {
        printf("s");
    }

    printf(" and %d Local APIC", num_lapics);

    if(num_lapics > 1) {
        printf("s");
    }

    printf("\n");

    return true;
}
