/**
 * acpi.h - header file containing structs and function prototypes relating
 * to the parsing of the ACPI tables.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "../bootdata.h"

/**
 * The RSDP (Root System Description Pointer) is a struct pointing to the
 * RSDT (Root System Description Table). This is for version 1.0.
 */
typedef struct __attribute__((packed)) {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdt_address; // The PHYSICAL address to the RSDT
} rsdp_v1_t;

/**
 * The RSDP V2.0 is an extended version of the RSDP V1.0, containing the
 * location of the XSDT (eXtended System Descriptor Table). The RSDT
 * is deprecated if we are on V2.
 */
typedef struct __attribute__((packed)) {
    rsdp_v1_t rsdp_v1;

    uint32_t length;
    uint64_t xsdt_address; // The PHYSICAL address to the XSDT
    uint8_t extended_checksum;
    uint8_t reserved[3];
} rsdp_v2_t;

/**
 * Struct at the start of all ACPI tables, containing information about
 * that table.
 */
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

/**
 * Struct mapping a table signature to a handler (parser).
 */
typedef struct {
    __attribute__((nonstring)) const char signature[4];
    bool (*handler)();
} acpi_table_handler_t;

/* Functions */

/**
 * Parses all available ACPI tables.
 * 
 * @param bootdata the information from the bootloader (so we can
 * access the RSDP)
 */
void acpi_init(boot_data_t* bootdata);

/**
 * Parses the RSDP and ensures it is valid.
 * 
 * @param rsdp_address the address of the RSDP
 */
void acpi_parse_rsdp(void* rsdp_address);

/**
 * Parses the RSDT and ensures it is valid.
 */
void acpi_parse_rsdt();

/**
 * Parses all of the tables given within the RSDT. ONLY to be called
 * if we are certain the RSDT exists and has already been parsed.
 */
void acpi_parse_rsdt_tables();

/**
 * Parses the XSDT and ensures it is valid.
 */
void acpi_parse_xsdt();

/**
 * Parses all of the tables given within the XSDT. ONLY to be called
 * if we are certain the XSDT exists and has already been parsed.
 */
void acpi_parse_xsdt_tables();

/**
 * Checks if a table has already been parsed (exclusing RSDT and XSDT).
 * 
 * @param signature the signature of the table to query
 * @return true if the table has been parsed, and false if not
 */
bool acpi_has_parsed(const char* signature);

/**
 * Returns the header of a table in either the RSDT or the XSDT (depending
 * on which version of ACPI is being used), given its signature.
 * 
 * @param signature the signature of the table to find
 * @return a pointer to the table header, or NULL if not found
 */
sdt_hdr_t* acpi_get_table(const char* signature);

/* Helper functions */

/**
 * Returns whether or not a table exists in the RSDT or the XSDT, given
 * its signature.
 * 
 * @param signature the signature of the table to query
 * @return true if the table exists, and false if not
 */
bool acpi_table_exists(const char* signature);

/**
 * Checks whether or not the signature in a table header is equal to
 * another signature.
 * 
 * @param hdr the header to check the signature of
 * @param signature the signature to check the header against (MUST be
 * 4 characters)
 * @return true if the signatures are equal, and false if not
 */
bool acpi_header_signature_equals(sdt_hdr_t* hdr, const char* signature);

/**
 * Checks whether or not two 4-character signatures are equal.
 * 
 * @param s1 the first 4-char signature
 * @param s2 the second 4-char signature
 * @return true if the two signatures are equal, and false if not
 */
bool acpi_signatures_equal(const char* s1, const char* s2);

/**
 * Checks whether or not a table header contains a valid checksum.
 * 
 * @param hdr the header of the table
 * @return true if the checksum is valid, false if not
 */
bool acpi_is_valid_sdt_checksum(sdt_hdr_t* hdr);
