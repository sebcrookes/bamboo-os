#pragma once

#include <stdint.h>

/* Stores human-readable cached PCI descriptors which are allocated when the device is discovered */
typedef struct {
    const char* name;
    const char* vendor;
    const char* class;
    const char* subclass;
} pci_descriptors_t;

/* Key-value pair for subclass (and prog_if) and the descriptor for it */
typedef struct {
    uint8_t subclass;
    int16_t prog_if; // Using signed int rather than uint8 allows for a "catch all" if the prog_if hasn't matched/doesn't matter
    const char* descriptor;
} pci_subclass_kv_t;

/* Key-value pair for a vendor and the descriptor for it */
typedef struct {
    uint16_t vendor;
    const char* descriptor;
} pci_vendor_kv_t;

pci_descriptors_t* pci_get_descriptors(uint16_t vendor, uint16_t device_id, uint8_t class_, uint8_t subclass, uint8_t prog_if);

const char* pci_get_subclass_descriptor(pci_subclass_kv_t* descriptor_arr, uint8_t subclass, uint8_t prog_if);
