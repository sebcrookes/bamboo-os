#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "rendering/framebuffer.h"

/* Bootloader info */
#define BOOTDATA_FIRMWARE_TYPE_X86BIOS 0
#define BOOTDATA_FIRMWARE_TYPE_UEFI32 1
#define BOOTDATA_FIRMWARE_TYPE_UEFI64 2
#define BOOTDATA_FIRMWARE_TYPE_SBI 3

typedef struct {
    bool exists;
    char* name;
    char* version;
} bootloader_info_t;

/* Firmware info */

typedef struct {
    bool exists;
    uint64_t type;
} firmware_info_t;

/* Higher half info (memory mapping) */

typedef struct {
    bool exists;
    uint64_t offset;
    uint64_t kernel_start;
    uint64_t kernel_end;
} higher_half_info_t;

/* Memory map info */

#define MEMMAP_USABLE                 0
#define MEMMAP_RESERVED               1
#define MEMMAP_ACPI_RECLAIMABLE       2
#define MEMMAP_ACPI_NVS               3
#define MEMMAP_BAD_MEMORY             4
#define MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define MEMMAP_EXECUTABLE_AND_MODULES 6
#define MEMMAP_FRAMEBUFFER            7

typedef struct __attribute__((packed)) {
    uint64_t base;
    uint64_t length;
    uint64_t type;
} memmap_entry_t;

typedef struct {
    bool exists;
    uint64_t num_entries;
    memmap_entry_t** entries;
} memmap_info_t;

/* RSDP info */

typedef struct {
    bool exists;
    uint64_t address;
} rsdp_info_t;

/* This struct stores data gathered from the bootloader so that it can be used by the kernel. Each struct stores a value "exists" to tell if the data is present or not. */

typedef struct {
    bootloader_info_t bootloader;
    firmware_info_t firmware;
    higher_half_info_t higher_half;
    bamboo_fb_t framebuffer;
    memmap_info_t memmap;
    rsdp_info_t rsdp;
} boot_data_t;
