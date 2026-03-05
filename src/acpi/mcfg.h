/**
 * mcfg.h - header file containing structs and function prototypes
 * used in the parsing of the MCFG table.
 */

#pragma once

#include "acpi.h"

#include "../inc/vector.h"

/**
 * The header for the MCFG table. Contains 8 extra reserved octets.
 */
typedef struct __attribute__((packed)) {
    sdt_hdr_t header;
    uint8_t reserved[8];
} mcfg_hdr_t;

/**
 * Struct containing details about PCIe segments.
 */
typedef struct __attribute__((packed)) {
    uint64_t base_address;
    uint16_t pci_seg_group;
    uint8_t start_bus;
    uint8_t end_bus;
    uint32_t reserved;
} mcfg_segment_group_t;

/**
 * Parses the MCFG table, gathering data about the PCI segment groups.
 * 
 * @return true if all parsing takes place as expected
 */
bool acpi_parse_mcfg();

/**
 * Returns the MCFG header, if it has been parsed.
 * 
 * @return the MCFG header if it has been parsed, otherwise returns NULL
 */
mcfg_hdr_t* acpi_get_mcfg();

/**
 * Returns a vector of mcfg_segment_group_t* which store details about
 * PCIe segment groups.
 * 
 * @return a vector containing information about multiple segment groups,
 * and NULL if not yet parsed.
 */
vector_t* acpi_mcfg_get_segment_groups();
