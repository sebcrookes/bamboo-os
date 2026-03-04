#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../limine/limine.h"

#include "inc/string.h"
#include "rendering/font.h"
#include "rendering/framebuffer.h"
#include "rendering/renderer.h"
#include "inc/faults.h"
#include "inc/stdio.h"

#include "bootdata.h"
#include "kernel.h"

extern char __KERNEL_START__;
extern char __KERNEL_END__;

__attribute__((used, section(".reqs_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".reqs")))
static volatile LIMINE_BASE_REVISION(3);

// Bootloader Info Feature
__attribute__((used, section(".reqs")))
static volatile struct limine_bootloader_info_request bootloader_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST,
    .revision = 0,
    .response = NULL
};

// Firmware Info Feature
__attribute__((used, section(".reqs")))
static volatile struct limine_firmware_type_request firmware_info_request = {
    .id = LIMINE_FIRMWARE_TYPE_REQUEST,
    .revision = 0,
    .response = NULL
};

// HHDM (Higher Half Direct Map) request
__attribute__((used, section(".reqs")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
    .response = NULL
};

// Memory map request
__attribute__((used, section(".reqs")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".reqs")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".reqs")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".reqs")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".reqs_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

boot_data_t bootdata;
bamboo_font_t font;

/* === The role of this file is to make limine requests, and do some basic initialisation.
       Control is then passed to the main "kernel.c" file to perform the rest of the init === */

void kentry(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED) faults_hang();

    // Set up the framebuffer
    framebuffer_init(&bootdata.framebuffer, framebuffer_request);

    font_load(&font, "ter-u14n.psf", module_request);
    renderer_init(&bootdata.framebuffer, &font);

    // Bootloader info request
    if(bootloader_info_request.response != NULL) {
        bootdata.bootloader.name = bootloader_info_request.response->name;
        bootdata.bootloader.version = bootloader_info_request.response->version;
        bootdata.bootloader.exists = true;
    } else {
        bootdata.bootloader.exists = false;
    }

    // Firmware info request
    if(firmware_info_request.response != NULL) {
        bootdata.firmware.type = firmware_info_request.response->firmware_type;
        bootdata.firmware.exists = true;
    } else {
        bootdata.firmware.exists = false;
    }

    // HHDM (Higher Half Direct Map) request
    if(hhdm_request.response != NULL) {
        bootdata.higher_half.offset = hhdm_request.response->offset;
        bootdata.higher_half.kernel_start = (uint64_t) &__KERNEL_START__;
        bootdata.higher_half.kernel_end = (uint64_t) &__KERNEL_END__;
        bootdata.higher_half.exists = true;
    } else {
        bootdata.higher_half.exists = false;
        faults_panic("Could not get higher-half mapping from bootloader");
    }

    // Memmap request
    if(memmap_request.response != NULL) {
        bootdata.memmap.num_entries = memmap_request.response->entry_count;
        bootdata.memmap.entries = (memmap_entry_t**) memmap_request.response->entries;
        bootdata.memmap.exists = true;
    } else {
        bootdata.memmap.exists = false;
        faults_panic("Could not get memory map from bootloader");
    }

    // RSDP request
    if(rsdp_request.response != NULL) {
        bootdata.rsdp.address = (uint64_t) rsdp_request.response->address;
        bootdata.rsdp.exists = true;
    } else {
        bootdata.rsdp.exists = false;
        faults_panic("Could not get RSDP address from bootloader");
    }

    // Clear the screen
    renderer_clear_screen(COLOUR_BACKGROUND);

    // Pass over control to the main kernel
    kernel_init(&bootdata);

    faults_hang();
}
