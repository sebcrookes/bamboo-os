#include "memory.h"

#include "../inc/stdio.h"

#include "pmm.h"
#include "vmm.h"

void memory_init(boot_data_t* bootdata) {
    printf("%C[Memory]%C - Higher-half mapping active, kernel at %X\n", COLOUR_KERNEL_INFO, COLOUR_PRINT, (uint64_t)bootdata->higher_half.kernel_start);

    pmm_init(bootdata);
    vmm_init(bootdata);

    /* === Create a new page map === */
    pt_t* pml4 = (pt_t*)pmm_allocz();

    /* We map all of the following types of memory to the higher-half by a fixed offset
    (offset given in bootdata->higher_half):
    
        - USABLE
        - BOOTLOADER_RECLAIMABLE
        - EXECUTABLE_AND_MODULES
        - FRAMEBUFFER
        - RESERVED
        - ACPI_RECLAIMABLE
        - ACPI_NVS
        - BAD_MEMORY

        We map the kernel to its position in the higher half (position given in bootdata->higher_half)
    */

    uint64_t higher_half_offset = bootdata->higher_half.offset;

    uint64_t mapped_page_count = 0;

    // Going through memory map to do everything in the above comment (except kernel mapping, which is done below)
    for(uint64_t i = 0; i < bootdata->memmap.num_entries; i++) {
        memmap_entry_t* entry = bootdata->memmap.entries[i];

        switch(entry->type) {
            case MEMMAP_USABLE:
            case MEMMAP_BOOTLOADER_RECLAIMABLE:
            case MEMMAP_EXECUTABLE_AND_MODULES:
            case MEMMAP_RESERVED:
            case MEMMAP_ACPI_RECLAIMABLE:
            case MEMMAP_ACPI_NVS:
            case MEMMAP_BAD_MEMORY:
            case MEMMAP_FRAMEBUFFER: {
                vmm_map_pages_pml4(pml4, (void*)entry->base, (void*)(entry->base + higher_half_offset), entry->length / 0x1000);
                mapped_page_count += entry->length / 0x1000;
            } break;
        }
    }

    // Mapping the kernel to its new location
    void* kernel_start_virt = (void*)bootdata->higher_half.kernel_start;
    void* kernel_start_phys = vmm_virt_to_phys(kernel_start_virt);

    uint64_t kernel_size = bootdata->higher_half.kernel_end - (uint64_t)kernel_start_virt;
    uint64_t kernel_pages = kernel_size / 0x1000;
    if(kernel_size % 0x1000 != 0) kernel_pages++;

    vmm_map_pages_pml4(pml4, kernel_start_phys, kernel_start_virt, kernel_pages);
    mapped_page_count += kernel_pages;

    vmm_set_pml4(pml4);
    
    printf("%C[VMM]%C - Created new page map - mapped %uMiB to the higher-half utilising larger pages for PDPT (1GiB) and PD (2MiB)\n", COLOUR_KERNEL_INFO, COLOUR_PRINT, mapped_page_count * 0x1000 / (1024*1024));
}
