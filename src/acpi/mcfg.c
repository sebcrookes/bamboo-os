/**
 * mcfg.c - C file containing the implementation of mcfg.h, parsing the
 * MCFG table.
 */

#include "mcfg.h"

#include "../inc/stdio.h"
#include "../inc/string.h"
#include "../inc/faults.h"

#include "../mem/heap.h"

mcfg_hdr_t* mcfg = NULL;

vector_t* mcfg_segment_groups = NULL;

bool acpi_parse_mcfg() {
    mcfg = (mcfg_hdr_t*) acpi_get_table("MCFG");
    if(mcfg == NULL) return false;

    if(!acpi_is_valid_sdt_checksum(&mcfg->header)) {
        printf("%C[MCFG]%C - Invalid MCFG checksum\n", COLOUR_KERNEL_INFO, COLOUR_PRINT);
        return false;
    }

    mcfg_segment_groups = vector_init();

    // Getting each subsequent entry
    uint64_t entries = (mcfg->header.length - sizeof(mcfg_hdr_t)) / sizeof(mcfg_segment_group_t);

    for(uint64_t i = 0; i < entries; i++) {
        mcfg_segment_group_t* config = (mcfg_segment_group_t*)(((uint8_t*) mcfg) + sizeof(mcfg_hdr_t) + (i * sizeof(mcfg_segment_group_t)));

        // Making a copy of the struct
        mcfg_segment_group_t* copy = (mcfg_segment_group_t*) malloc(sizeof(mcfg_segment_group_t));
        memcpy((void*) copy, (void*) config, sizeof(mcfg_segment_group_t));

        // Adding the copy to the list
        vector_add(mcfg_segment_groups, (void*) copy);
    }

    printf("%C[MCFG]%C - Parsed MCFG\n", COLOUR_KERNEL_INFO, COLOUR_PRINT);

    return true;
}

mcfg_hdr_t* acpi_get_mcfg() {
    return mcfg;
}

vector_t* acpi_mcfg_get_segment_groups() {
    return mcfg_segment_groups;
}
