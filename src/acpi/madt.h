#pragma once

#include "acpi.h"

/**
 * The struct which comes directly after the header, telling
 * us the address of the BSP's Local APIC.
 */
typedef struct __attribute__((packed)) {
    uint32_t address;
    uint32_t flags;
} madt_local_apic_bsp_t;

/**
 * The struct at the start of each entry. Tells us the type of the
 * entry, and the length of the entry (including this struct).
 */
typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t length;
} madt_entry_info_t;

/**
 * The struct for type 0 entries. Tells us about Local APICs in the
 * system - both their ID and the processor ID they are connected to.
 */
typedef struct __attribute__((packed)) {
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} madt_local_apic_t;

/**
 * The struct for type 1 entries. Tells us about I/O APICs in the
 * system - their ID, their address, and their base GSI which they
 * cover.
 */
typedef struct __attribute__((packed)) {
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_address;
    uint32_t gsi_base;
} madt_ioapic_t;

/**
 * A general struct for flags about specific interrupts.
 */
typedef struct __attribute__((packed)) {
    uint8_t polarity : 2;
    uint8_t trigger : 2;
    uint16_t reserved : 12;
} madt_flags_t;

/**
 * The struct for type 2 entries. Tells us about how IRQs from
 * the old PIC-based system map to GSIs.
 */
typedef struct __attribute__((packed)) {
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t gsi;
    madt_flags_t flags;
} madt_int_src_override_t;

/**
 * The struct for type 3 entries. Tells us which GSIs should be
 * NMIs at the I/O APIC level.
 */
typedef struct __attribute__((packed)) {
    madt_flags_t flags;
    uint32_t gsi;
} madt_ioapic_nmi_src_t;

/**
 * The struct for type 4 entries. Tells us which processors should
 * have which LINT lines enabled for NMIs.
 */
typedef struct __attribute__((packed)) {
    uint8_t acpi_processor_id; // 0xFF = all 
    madt_flags_t flags;
    uint8_t lint_number;
} madt_lapic_nmi_t;

/**
 * The struct for type 5 entries. Tells us the address of the BSP's
 * Local APIC, if it is a 64-bit address.
 */
typedef struct __attribute__((packed)) {
    uint16_t reserved;
    uint64_t address;
} madt_lapic_addr_override_t;

/**
 * Parses the MADT table, making copies of info about Local APICs,
 * I/O APICs, and any other data about mappings and interrupts.
 * 
 * @return true if all parsing happened as expected
 */
bool acpi_parse_madt();
