#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "pci_tables.h"

#define PCI_CMD_IO_SPACE 0b1
#define PCI_CMD_MEM_SPACE 0b1 << 1
#define PCI_CMD_BUS_MASTER 0b1 << 2
#define PCI_CMD_SPECIAL_CYCLES 0b1 << 3
#define PCI_CMD_MEM_W_INVAL_EN 0b1 << 4
#define PCI_CMD_VGA_PALETTE_SNOOP 0b1 << 5
#define PCI_CMD_PARITY_ERR_RESP 0b1 << 6
#define PCI_CMD_SERR_EN 0b1 << 8
#define PCI_CMD_FAST_B2B_EN 0b1 << 9
#define PCI_CMD_INT_DISABLE 0b1 << 10

#define PCI_STATUS_INTERRUPT 0b1 << 3
#define PCI_STATUS_CAP_LIST 0b1 << 4
#define PCI_STATUS_66MHZ_CAP 0b1 << 5
#define PCI_STATUS_FAST_B2B_CAP 0b1 << 7
#define PCI_STATUS_MASTER_PARITY_ERR 0b1 << 8
#define PCI_STATUS_DEVSEL_TIMING_LO 0b1 << 9
#define PCI_STATUS_DEVSEL_TIMING_HI 0b1 << 10
#define PCI_STATUS_SIG_TARGET_ABORT 0b1 << 11
#define PCI_STATUS_RECV_TARGET_ABORT 0b1 << 12
#define PCI_STATUS_RECV_MASTER_ABORT 0b1 << 13
#define PCI_STATUS_SIG_SYSTEM_ERR 0b1 << 14
#define PCI_STATUS_PARITY_ERR_DETECT 0b1 << 15

typedef struct __attribute__((packed)) {
    uint16_t vendor;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class_;
    uint8_t cache_line_sz;
    uint8_t latency_timer;
    uint8_t hdr_type;
    uint8_t bist;
} pci_device_hdr_t;

typedef struct __attribute__((packed)) {
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
    uint32_t cardbus_cis_ptr;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint32_t expansion_rom_base;
    uint8_t capabilities_ptr;
    uint8_t reserved[7];
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;
} pci_extended_hdr_t; // The header which follows the device header for most devices (hdr_type = 0x0)

typedef struct __attribute__((packed)) {
    uint32_t bar0;
    uint32_t bar1;
    uint8_t primary_bus_num;
    uint8_t secondary_bus_num;
    uint8_t subordinate_bus_num;
    uint8_t secondary_latency_timer;
    uint8_t io_base;
    uint8_t io_limit;
    uint16_t secondary_status;
    uint16_t mem_base;
    uint16_t mem_limit;
    uint16_t prefetchable_mem_base;
    uint16_t prefetchable_mem_limit;
    uint32_t prefetchable_base_upper;
    uint32_t prefetchable_limit_upper;
    uint16_t io_base_upper;
    uint16_t io_limit_upper;
    uint8_t capabilities_ptr;
    uint8_t reserved[3];
    uint32_t expansion_rom_base;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t bridge_control;
} pci_bridge_0x1_hdr_t; // The header which follows the device header for PCI-to-PCI bridge (hdr_type = 0x1)

typedef struct {
    pci_device_hdr_t* header;
    pci_descriptors_t* descriptors;
    
    // Only one of the following will actually be in use, the others will be null
    pci_extended_hdr_t* extended_header;
    pci_bridge_0x1_hdr_t* bridge_0x1_header;
} pci_device_t;

void pci_init();
void pci_init_builtin_drivers();

void pci_register_device(pci_device_hdr_t* hdr);

bool pci_is_status_flag_set(pci_device_t* dev, uint16_t bit);

void pci_set_command_flag(pci_device_t* dev, uint16_t command);
void pci_clear_command_flag(pci_device_t* dev, uint16_t flag);
