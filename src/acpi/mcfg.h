#pragma once

#include "acpi.h"

#include "../inc/vector.h"

typedef struct __attribute__((packed)) {
    sdt_hdr_t header;
    uint8_t reserved[8];
} mcfg_hdr_t;

typedef struct __attribute__((packed)) {
    uint64_t base_address;
    uint16_t pci_seg_group;
    uint8_t start_bus;
    uint8_t end_bus;
    uint32_t reserved;
} mcfg_segment_group_t;

void acpi_parse_mcfg();

mcfg_hdr_t* acpi_get_mcfg();

vector_t* acpi_mcfg_get_segment_groups();
