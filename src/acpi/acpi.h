#pragma once

#include <stdint.h>

#include "../bootdata.h"

typedef struct __attribute__((packed)) {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdt_address; // The PHYSICAL address to the RSDT
} rsdp_t;

typedef struct __attribute__((packed)) {
    rsdp_t rsdp_v1;

    uint32_t length;
    uint64_t xsdt_address; // The PHYSICAL address to the XSDT
    uint8_t extended_checksum;
    uint8_t reserved[3];
} rsdp_v2_t;

typedef struct __attribute__((packed)) {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEMID[6];
    char OEM_table_id[8];
    uint32_t OEM_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} sdt_hdr_t;

/* Functions */

void acpi_init(boot_data_t* bootdata);

void acpi_parse_rsdp(void* rsdp_address);

void acpi_parse_rsdt();
void acpi_parse_rsdt_tables();

void acpi_parse_xsdt();
void acpi_parse_xsdt_tables();

sdt_hdr_t* acpi_get_table(const char* signature);

/* Helper functions */
bool acpi_table_exists(const char* signature);
bool acpi_is_signature_equal(sdt_hdr_t* hdr, const char* signature);
bool acpi_is_valid_sdt_checksum(sdt_hdr_t* hdr);
