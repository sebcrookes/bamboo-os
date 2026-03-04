#include "mcfg.h"

#include "../inc/stdio.h"
#include "../inc/faults.h"

mcfg_hdr_t* mcfg = NULL;

vector_t* mcfg_segment_groups = NULL;

void acpi_parse_mcfg() {
    mcfg = (mcfg_hdr_t*) acpi_get_table("MCFG");

    faults_assert(acpi_is_valid_sdt_checksum(&mcfg->header), "Invalid MCFG checksum");

    mcfg_segment_groups = vector_init();

    // Getting each subsequent entry
    uint64_t entries = (mcfg->header.length - sizeof(mcfg_hdr_t)) / sizeof(mcfg_segment_group_t);

    for(uint64_t i = 0; i < entries; i++) {
        mcfg_segment_group_t* config = (mcfg_segment_group_t*)(((uint8_t*) mcfg) + sizeof(mcfg_hdr_t) + (i * sizeof(mcfg_segment_group_t)));

        vector_add(mcfg_segment_groups, (void*) config);
    }

    printf("%C[MCFG]%C - Parsed MCFG\n", COLOUR_KERNEL_INFO, COLOUR_PRINT);
}

mcfg_hdr_t* acpi_get_mcfg() {
    return mcfg;
}

vector_t* acpi_mcfg_get_segment_groups() {
    return mcfg_segment_groups;
}
