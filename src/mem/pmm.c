#include "pmm.h"

#include "../inc/stdio.h"
#include "../include/string.h"

#include "vmm.h"

uint64_t higher_half_offset = 0;

uint64_t memmap_num_entries = 0;
memmap_entry_t** memmap_entries;

uint64_t pmm_usable_pages = 0;
uint64_t pmm_bitmap_pages = 0;
uint64_t pmm_num_allocated_pages = 0;

/* The bitmap storing whether or not pages are allocated is stored in the
   very first pages of usable memory. Free pages are indexed by the */

void pmm_init(boot_data_t* bootdata) {
    higher_half_offset = bootdata->higher_half.offset;
    
    memmap_num_entries = bootdata->memmap.num_entries;
    memmap_entries = bootdata->memmap.entries;

    /* Calculating available memory */
    for(uint64_t i = 0; i < memmap_num_entries; i++) {
        memmap_entry_t* entry = memmap_entries[i];        
        if(entry->type == MEMMAP_USABLE) {
            pmm_usable_pages += entry->length / 0x1000;
        }
    }

    /* Calculating number of pages needed for bitmap (each byte = one page) */
    pmm_bitmap_pages = ((pmm_usable_pages) / (0x1000 * 8)) + 1;

    /* Setting all pages to unallocated, then allocating the bitmap pages */
    for(uint64_t i = 0; i < pmm_bitmap_pages; i++) {
        uint64_t phys_address = pmm_get_address_by_index(i);
        memset((void*)pmm_phys_to_virt(phys_address), 0, 0x1000);
    }

    for(uint64_t i = 0; i < pmm_bitmap_pages; i++) {
        pmm_set_usable_page_allocated_by_index(i, true);
    }

    /* Print confirmation that everything has been initialised correctly */
    printf("%C[PMM]%C - PMM initialised, usable memory: %X bytes\n", COLOUR_KERNEL_INFO, COLOUR_PRINT, pmm_get_total_usable_memory());
}

uint64_t pmm_phys_to_virt(uint64_t phys) {
    return phys + higher_half_offset;
}

uint64_t pmm_virt_to_phys(uint64_t virt) {
    if(vmm_has_initialised()) {
        return (uint64_t)vmm_virt_to_phys((void*)virt);
    }

    return virt - higher_half_offset;
}

// Returns the address of the page which was given as an index of the paged usable memory
uint64_t pmm_get_address_by_index(uint64_t index) {
    uint64_t current_index = 0;
    for(uint64_t i = 0; i < memmap_num_entries; i++) {
        memmap_entry_t* entry = memmap_entries[i];

        if(entry->type == MEMMAP_USABLE) {
            if(current_index + (entry->length / 0x1000) <= index) {
                current_index += entry->length / 0x1000;
                continue;
            } else {
                /* The first usable page is often 0x1000. This is then mapped (by bootloader) to 0xFFFF800000001000.
                Therefore, the user MUST convert the physical address to a virtual address to prevent page faults. */
                uint64_t position = entry->base + (0x1000 * (index - current_index));
                return position;
            }
        }
    }

    return UINT64_MAX; // Return this value as if we return 0 (null), that could be considered as a valid page
}

// Returns the index of the page which was given as a physical address
uint64_t pmm_get_index_by_address(uint64_t address) {
    uint64_t current_index = 0;

    uint64_t physical_address = address;

    for(uint64_t i = 0; i < memmap_num_entries; i++) {
        memmap_entry_t* entry = memmap_entries[i];

        if(entry->type == MEMMAP_USABLE) {
            if(physical_address >= entry->base && physical_address < entry->base + entry->length) {
                return current_index + (uint64_t)((physical_address - entry->base) / 0x1000);
            }

            current_index += entry->length / 0x1000;
        }
    }

    return UINT64_MAX; // Return this value as if we return 0 (null), that could be considered as a valid page
}

// Returns whether or not a page, given as an index of the paged usable memory, has been allocated
bool pmm_is_usable_page_allocated_by_index(uint64_t index) {
    uint64_t bitmap_page = index / (0x1000 * 8);
    
    uint64_t bit_of_page = index - (bitmap_page * (0x1000 * 8));
    uint64_t byte_of_page = bit_of_page / 8;
    uint8_t bit_of_byte = bit_of_page - (8 * byte_of_page);

    uint8_t* page = (uint8_t*)(void*) pmm_get_address_by_index(bitmap_page);
    if((uint64_t)page == UINT64_MAX) return true;

    page = (uint8_t*)(void*) pmm_phys_to_virt((uint64_t)(void*)page);

    uint8_t byte = page[byte_of_page];

    return byte & (0b1 << bit_of_byte);
}

// Sets whether or not a page, given as an index of the paged usable memory, is allocated
void pmm_set_usable_page_allocated_by_index(uint64_t index, bool allocated) {
    uint64_t bitmap_page = index / (0x1000 * 8);
    
    uint64_t bit_of_page = index - (bitmap_page * (0x1000 * 8));
    uint64_t byte_of_page = bit_of_page / 8;
    uint8_t bit_of_byte = bit_of_page - (byte_of_page * 8);

    uint8_t* page = (uint8_t*)(void*) pmm_get_address_by_index(bitmap_page);
    if((uint64_t)page == UINT64_MAX) return;

    // Getting the virtual address of the bitmap page. This is never remapped so we can safely just add on the offset
    page = (uint8_t*)(void*) pmm_phys_to_virt((uint64_t)(void*)page);

    uint8_t byte = page[byte_of_page];

    uint8_t new_byte = byte;

    if(allocated) new_byte = byte | 0b1 << bit_of_byte;
    else new_byte = byte & ~(0b1 << bit_of_byte);

    // Change the number of allocated pages if there has actually been a change
    if(allocated && new_byte != byte) pmm_num_allocated_pages++;
    else if(!allocated && new_byte != byte) pmm_num_allocated_pages--;

    page[byte_of_page] = new_byte;
}

// Returns the physical address of a free page
void* pmm_alloc_phys() {
    for(uint64_t i = 0; i < pmm_usable_pages; i++) {
        if(!pmm_is_usable_page_allocated_by_index(i)) {
            pmm_set_usable_page_allocated_by_index(i, true);

            void* address = (void*)pmm_get_address_by_index(i);
            if(address == (void*)UINT64_MAX) return 0;
            
            return address;
        }
    }

    return 0;
}

// Returns the virtual address of a free page
void* pmm_alloc() {
    void* phys = pmm_alloc_phys();
    return (void*)pmm_phys_to_virt((uint64_t)phys);
}

void* pmm_allocz() {
    void* page = pmm_alloc();
    memset(page, 0, 0x1000);
    return page;
}

void pmm_free(void* page) {
    uint64_t phys = pmm_virt_to_phys((uint64_t)page);
    if(phys == UINT64_MAX) return;

    uint64_t index = pmm_get_index_by_address(phys);

    if(index >= pmm_bitmap_pages) { // Prevents the freeing of bitmap pages
        pmm_set_usable_page_allocated_by_index(index, false);

        // Remapping the page to where it should be, just in case the user has moved it
        if(vmm_has_initialised()) {
            vmm_map_page((void*)phys, (void*)pmm_phys_to_virt(phys));
        }
    }
}

uint64_t pmm_get_total_usable_memory() {
    return pmm_usable_pages * 0x1000;
}

uint64_t pmm_get_current_free_memory() {
    return (pmm_usable_pages - pmm_num_allocated_pages) * 0x1000;
}
