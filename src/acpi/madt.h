#pragma once

#include "acpi.h"

typedef struct __attribute__((packed)) {
    uint8_t address : 4;
    uint8_t flags : 4;
} lapic_bsp_t;

void acpi_parse_madt();
