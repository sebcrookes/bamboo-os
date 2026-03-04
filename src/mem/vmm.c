#include "vmm.h"

#include "../inc/stdio.h"

#include "pmm.h"

uint64_t vmm_page_tables_offset = 0;

pt_t* pml4 = (pt_t*)NULL; // Stores the currently in-use PML4

bool vmm_initialised = false;

void vmm_init(boot_data_t* bootdata) {
    /* All page tables have been moved to the higher half by the bootloader. Therefore we have to use
       an offset to get back and forth between virtual and physical addresses of page tables */
    vmm_page_tables_offset = bootdata->higher_half.offset;

    /* Get the bootloader's page table */
    uint64_t cr3 = 0;
    asm("\tmov %%cr3,%0" : "=r"(cr3));

    cr3 += vmm_page_tables_offset;
    pml4 = (pt_t*)cr3;

    printf("%C[VMM]%C - VMM initialised\n", COLOUR_KERNEL_INFO, COLOUR_PRINT);

    vmm_initialised = true;
}

/* Gets the physical address of a given virtual address in the current page table */
void* vmm_virt_to_phys(void* virtual_address) {
    if(!vmm_initialised) return (void*)UINT64_MAX;

    /* Getting the PDPT */
    pt_entry_t pml4_entry = pml4->entries[PML4_INDEX((uint64_t)virtual_address)];
    if(!(pml4_entry.value & PT_ENTRY_PRESENT)) return (void*)UINT64_MAX;

    pt_t* pdpt = (pt_t*)(vmm_pt_entry_get_address(&pml4_entry) + vmm_page_tables_offset);

    /* Getting the PD */
    pt_entry_t pdpt_entry = pdpt->entries[PDPT_INDEX((uint64_t)virtual_address)];
    if(!(pdpt_entry.value & PT_ENTRY_PRESENT)) return (void*)UINT64_MAX;

    // Handling a large page entry at the page directory pointer table (PDPT) level (these have size 1GiB)
    if(pdpt_entry.value & PT_ENTRY_LARGER_PAGES) {
        uint64_t large_page_phys_addr = vmm_pt_entry_get_address(&pdpt_entry);

        uint64_t page_offset = (uint64_t)virtual_address % (uint64_t)(0x1000 * 512 * 512);

        return (void*)(large_page_phys_addr + page_offset);
    }

    pt_t* pd = (pt_t*)(vmm_pt_entry_get_address(&pdpt_entry) + vmm_page_tables_offset);

    /* Getting the PT */
    pt_entry_t pd_entry = pd->entries[PD_INDEX((uint64_t)virtual_address)];
    if(!(pd_entry.value & PT_ENTRY_PRESENT)) return (void*)UINT64_MAX;

    // Handling a large page entry at the page directory (PD) level (these have size 2MiB)
    if(pd_entry.value & PT_ENTRY_LARGER_PAGES) {
        uint64_t large_page_phys_addr = vmm_pt_entry_get_address(&pd_entry);

        uint64_t page_offset = (uint64_t)virtual_address % (uint64_t)(0x1000 * 512);

        return (void*)(large_page_phys_addr + page_offset);
    }

    pt_t* pt = (pt_t*)(vmm_pt_entry_get_address(&pd_entry) + vmm_page_tables_offset);
    
    /* Getting the specific page and returning its physical address */
    pt_entry_t pt_entry = pt->entries[PT_INDEX((uint64_t)virtual_address)];
    if(!(pt_entry.value & PT_ENTRY_PRESENT)) return (void*)UINT64_MAX;

    uint64_t phys_addr = vmm_pt_entry_get_address(&pt_entry);
    return (void*)phys_addr;
}

void vmm_map_page(void* physical_address, void* virtual_address) {
    vmm_map_page_pml4(pml4, physical_address, virtual_address, PT_ENTRY_READ_WRITE);
}

void vmm_map_page_flags(void* physical_address, void* virtual_address, uint8_t flags) {
    vmm_map_page_pml4(pml4, physical_address, virtual_address, flags);
}

void vmm_map_page_pml4(pt_t* p_pml4, void* physical_address, void* virtual_address, uint8_t flags) {

    // We do this just in case the user has un-knowingly given us a virtual address
    void* converted_to_phys = vmm_virt_to_phys(physical_address);
    if(converted_to_phys != (void*)UINT64_MAX) physical_address = converted_to_phys;

    /* Getting the PDPT */
    pt_entry_t* pml4_entry = &p_pml4->entries[PML4_INDEX((uint64_t)virtual_address)];
    if(!(pml4_entry->value & PT_ENTRY_PRESENT)) {
        vmm_init_pt_entry(pml4_entry);
        pml4_entry->value |= PT_ENTRY_READ_WRITE;
        pml4_entry->value |= PT_ENTRY_PRESENT;
    }

    pt_t* pdpt = (pt_t*)(vmm_pt_entry_get_address(pml4_entry) + vmm_page_tables_offset);

    /* Getting the PD */
    pt_entry_t* pdpt_entry = &pdpt->entries[PDPT_INDEX((uint64_t)virtual_address)];
    if(!(pdpt_entry->value & PT_ENTRY_PRESENT)) {
        vmm_init_pt_entry(pdpt_entry);
        pdpt_entry->value |= PT_ENTRY_READ_WRITE;
        pdpt_entry->value |= PT_ENTRY_PRESENT;
    }

    /* If the PDPT entry has the larger pages flag set, then create a new page table, initialise all of its entries and remove the larger pages flag */
    if(pdpt_entry->value & PT_ENTRY_LARGER_PAGES) {
        uint64_t base_phys_addr = vmm_pt_entry_get_address(pdpt_entry);

        /* We have to create the new pdpt_entry in a different variable as editing the value in real-time is not thread safe
           and would likely affect where we use newly-allocated pages (as they would not be mapped to anything, causing a page fault) */
        uint64_t new_pdpt_entry_address = (uint64_t)pmm_allocz() - vmm_page_tables_offset;
        uint64_t new_pdpt_entry_value = 0;
        vmm_pt_entry_set_address((pt_entry_t*)&new_pdpt_entry_value, new_pdpt_entry_address);
        new_pdpt_entry_value |= PT_ENTRY_READ_WRITE;
        new_pdpt_entry_value |= PT_ENTRY_PRESENT;

        pt_t* new_pd = (pt_t*)(new_pdpt_entry_address + vmm_page_tables_offset);
        
        for(uint64_t i = 0; i < 512; i++) {
            pt_entry_t* next_entry = &new_pd->entries[i];
            vmm_pt_entry_set_address(next_entry, base_phys_addr + (i * 0x1000 * 512));
            next_entry->value |= PT_ENTRY_READ_WRITE;
            next_entry->value |= PT_ENTRY_PRESENT;
        }

        // Now that everything has been set-up the same as it was before, we can set the actual pdpt_entry to our new, 'non-larger-pages' value
        pdpt_entry->value = new_pdpt_entry_value;
    }

    pt_t* pd = (pt_t*)(vmm_pt_entry_get_address(pdpt_entry) + vmm_page_tables_offset);

    /* Getting the PT */
    pt_entry_t* pd_entry = &pd->entries[PD_INDEX((uint64_t)virtual_address)];
    if(!(pd_entry->value & PT_ENTRY_PRESENT)) {
        vmm_init_pt_entry(pd_entry);
        pd_entry->value |= PT_ENTRY_READ_WRITE;
        pd_entry->value |= PT_ENTRY_PRESENT;
    }

    /* If the PD entry has the larger pages flag set, then create a new page table, initialise all of its entries and remove the larger pages flag */
    if(pd_entry->value & PT_ENTRY_LARGER_PAGES) {
        uint64_t base_phys_addr = vmm_pt_entry_get_address(pd_entry);

        /* We have to create the new pd_entry in a different variable as editing the value in real-time is not thread safe
           and would likely affect where we use newly-allocated pages (as they would not be mapped to anything, causing a page fault) */
        uint64_t new_pd_entry_address = (uint64_t)pmm_allocz() - vmm_page_tables_offset;
        uint64_t new_pd_entry_value = 0;
        vmm_pt_entry_set_address((pt_entry_t*)&new_pd_entry_value, new_pd_entry_address);
        new_pd_entry_value |= PT_ENTRY_READ_WRITE;
        new_pd_entry_value |= PT_ENTRY_PRESENT;

        pt_t* new_pt = (pt_t*)(new_pd_entry_address + vmm_page_tables_offset);
        
        for(uint64_t i = 0; i < 512; i++) {
            pt_entry_t* next_entry = &new_pt->entries[i];
            vmm_pt_entry_set_address(next_entry, base_phys_addr + (i * 0x1000));
            next_entry->value |= PT_ENTRY_READ_WRITE;
            next_entry->value |= PT_ENTRY_PRESENT;
        }

        // Now that everything has been set-up the same as it was before, we can set the actual pd_entry to our new, 'non-larger-pages' value
        pd_entry->value = new_pd_entry_value;
    }

    pt_t* pt = (pt_t*)(vmm_pt_entry_get_address(pd_entry) + vmm_page_tables_offset);
    
    /* Getting the specific page and setting its physical address */
    pt_entry_t* pt_entry = &pt->entries[PT_INDEX((uint64_t)virtual_address)];

    /* As done above with the larger-pages flag, we create a new value and modify that, then write that to the actual entry.
       This is much more thread-safe. */
    uint64_t new_pt_entry_value = 0;
    vmm_pt_entry_set_address((pt_entry_t*)&new_pt_entry_value, (uint64_t)physical_address);
    new_pt_entry_value |= flags;
    new_pt_entry_value |= PT_ENTRY_PRESENT;

    pt_entry->value = new_pt_entry_value;

    /* Invalidating any possibly pre-existing entries in the TLB */
    vmm_invlpg((uint64_t)physical_address); // This not being here gave me a headache.
    vmm_invlpg((uint64_t)virtual_address);
}

void vmm_map_pages_pml4(pt_t* p_pml4, void* physical_address, void* virtual_address, uint64_t num_pages) {
    uint64_t pml4_i = PML4_INDEX((uint64_t)virtual_address);
    uint64_t pdpt_i = PDPT_INDEX((uint64_t)virtual_address);
    uint64_t pd_i = PD_INDEX((uint64_t)virtual_address);
    uint64_t pt_i = PT_INDEX((uint64_t)virtual_address);

    // We do this just in case the user has un-knowingly given us a virtual address
    void* converted_to_phys = vmm_virt_to_phys(physical_address);
    if(converted_to_phys != (void*)UINT64_MAX) physical_address = converted_to_phys;

    /* Getting the PDPT */
    pt_entry_t* pml4_entry = &p_pml4->entries[pml4_i];
    if(!(pml4_entry->value & PT_ENTRY_PRESENT)) {
        vmm_init_pt_entry(pml4_entry);
        pml4_entry->value |= PT_ENTRY_READ_WRITE;
        pml4_entry->value |= PT_ENTRY_PRESENT;
    }

    pt_t* pdpt = (pt_t*)(vmm_pt_entry_get_address(pml4_entry) + vmm_page_tables_offset);

    /* Getting the PD */
    pt_entry_t* pdpt_entry = &pdpt->entries[pdpt_i];
    if(!(pdpt_entry->value & PT_ENTRY_PRESENT)) {
        vmm_init_pt_entry(pdpt_entry);
        pdpt_entry->value |= PT_ENTRY_READ_WRITE;
        pdpt_entry->value |= PT_ENTRY_PRESENT;
    }

    pt_t* pd = (pt_t*)(vmm_pt_entry_get_address(pdpt_entry) + vmm_page_tables_offset);

    /* Getting the PT */
    pt_entry_t* pd_entry = &pd->entries[pd_i];
    if(!(pd_entry->value & PT_ENTRY_PRESENT)) {
        vmm_init_pt_entry(pd_entry);
        pd_entry->value |= PT_ENTRY_READ_WRITE;
        pd_entry->value |= PT_ENTRY_PRESENT;
    }

    pt_t* pt = (pt_t*)(vmm_pt_entry_get_address(pd_entry) + vmm_page_tables_offset);
    
    /* Getting the specific page */
    pt_entry_t* pt_entry = &pt->entries[pt_i];
    if(!(pt_entry->value & PT_ENTRY_PRESENT)) {
        vmm_init_pt_entry(pt_entry);
        pt_entry->value |= PT_ENTRY_READ_WRITE;
        pt_entry->value |= PT_ENTRY_PRESENT;
    }

    uint64_t current_phys = (uint64_t)physical_address;

    /* Pretty much a depth-first search */
    for(uint64_t i = 0; i < num_pages; i++) {

        /* This allows for the creation of PDPT level 'larger pages' (1GiB size) */
        if(num_pages - 1 - i > 512 * 512 && pd_i == 0 && pt_i == 0) {
            // First, we free the memory allocated during the last cycle
            pmm_free((void*)pt);
            pmm_free((void*)pd);

            // Then, we set the pdpt_entry to store the address and the larger pages flag
            pdpt_entry->value |= PT_ENTRY_LARGER_PAGES;
            vmm_pt_entry_set_address(pdpt_entry, current_phys);
            pt_i += 512;
            pd_i += 512;
            i += 512 * 512 - 1;
            current_phys += 0x1000 * 512 * 512;
        } else if(num_pages - 1 - i > 512 && pt_i == 0) { /* This allows for the creation of PD level 'larger pages' (2MiB size) */
            // First, we free the memory allocated during the last cycle
            pmm_free((void*)pt);

            // Then, we set the pd_entry to store the address and the larger pages flag
            pd_entry->value |= PT_ENTRY_LARGER_PAGES;
            vmm_pt_entry_set_address(pd_entry, current_phys);
            pt_i += 512;
            i += 512 - 1;
            current_phys += 0x1000 * 512;
        } else {
            vmm_pt_entry_set_address(pt_entry, current_phys);
            pt_i++;
            current_phys += 0x1000;
        }

        // Skip creating the next entry when we are done - this leads to an extra entry
        if(i == num_pages - 1) {
            return;
        }

        // Get the next indices and updating the other variables as necessary
        if(pt_i > 511) {
            pt_i = 0;
            pd_i++;

            if(pd_i > 511) {
                pd_i = 0;
                pdpt_i++;

                if(pdpt_i > 511) {
                    pdpt_i = 0;
                    pml4_i++;

                    if(pml4_i > 511) {
                        return;
                    }

                    pml4_entry = &p_pml4->entries[pml4_i];
                    if(!(pml4_entry->value & PT_ENTRY_PRESENT)) {
                        vmm_init_pt_entry(pml4_entry);
                        pml4_entry->value |= PT_ENTRY_READ_WRITE;
                        pml4_entry->value |= PT_ENTRY_PRESENT;
                    }

                    pdpt = (pt_t*)(vmm_pt_entry_get_address(pml4_entry) + vmm_page_tables_offset);
                }

                pdpt_entry = &pdpt->entries[pdpt_i];
                if(!(pdpt_entry->value & PT_ENTRY_PRESENT)) {
                    vmm_init_pt_entry(pdpt_entry);
                    pdpt_entry->value |= PT_ENTRY_READ_WRITE;
                    pdpt_entry->value |= PT_ENTRY_PRESENT;
                }

                pd = (pt_t*)(vmm_pt_entry_get_address(pdpt_entry) + vmm_page_tables_offset);
            }

            pd_entry = &pd->entries[pd_i];
            if(!(pd_entry->value & PT_ENTRY_PRESENT)) {
                vmm_init_pt_entry(pd_entry);
                pd_entry->value |= PT_ENTRY_READ_WRITE;
                pd_entry->value |= PT_ENTRY_PRESENT;
            }

            pt = (pt_t*)(vmm_pt_entry_get_address(pd_entry) + vmm_page_tables_offset);
        }

            
        /* Getting the specific page */
        pt_entry = &pt->entries[pt_i];
        if(!(pt_entry->value & PT_ENTRY_PRESENT)) {
            vmm_init_pt_entry(pt_entry);
            pt_entry->value |= PT_ENTRY_READ_WRITE;
            pt_entry->value |= PT_ENTRY_PRESENT;
        }
    }
}


void vmm_unmap_page(void* virtual_address) {
    /* Getting the PDPT */
    pt_entry_t* pml4_entry = &pml4->entries[PML4_INDEX((uint64_t)virtual_address)];
    if(!(pml4_entry->value & PT_ENTRY_PRESENT)) {
        vmm_init_pt_entry(pml4_entry);
        pml4_entry->value |= PT_ENTRY_READ_WRITE;
        pml4_entry->value |= PT_ENTRY_PRESENT;
    }

    pt_t* pdpt = (pt_t*)(vmm_pt_entry_get_address(pml4_entry) + vmm_page_tables_offset);

    /* Getting the PD */
    pt_entry_t* pdpt_entry = &pdpt->entries[PDPT_INDEX((uint64_t)virtual_address)];
    if(!(pdpt_entry->value & PT_ENTRY_PRESENT)) {
        vmm_init_pt_entry(pdpt_entry);
        pdpt_entry->value |= PT_ENTRY_READ_WRITE;
        pdpt_entry->value |= PT_ENTRY_PRESENT;
    }

    /* If the PDPT entry has the larger pages flag set, then create a new page table, initialise all of its entries and remove the larger pages flag */
    if(pdpt_entry->value & PT_ENTRY_LARGER_PAGES) {
        uint64_t base_phys_addr = vmm_pt_entry_get_address(pdpt_entry);

        /* We have to create the new pdpt_entry in a different variable as editing the value in real-time is not thread safe
           and would likely affect where we use newly-allocated pages (as they would not be mapped to anything, causing a page fault) */
        uint64_t new_pdpt_entry_address = (uint64_t)pmm_allocz() - vmm_page_tables_offset;
        uint64_t new_pdpt_entry_value = 0;
        vmm_pt_entry_set_address((pt_entry_t*)&new_pdpt_entry_value, new_pdpt_entry_address);
        new_pdpt_entry_value |= PT_ENTRY_READ_WRITE;
        new_pdpt_entry_value |= PT_ENTRY_PRESENT;

        pt_t* new_pd = (pt_t*)(new_pdpt_entry_address + vmm_page_tables_offset);
        
        for(uint64_t i = 0; i < 512; i++) {
            pt_entry_t* next_entry = &new_pd->entries[i];
            vmm_pt_entry_set_address(next_entry, base_phys_addr + (i * 0x1000 * 512));
            next_entry->value |= PT_ENTRY_READ_WRITE;
            next_entry->value |= PT_ENTRY_PRESENT;
        }

        // Now that everything has been set-up the same as it was before, we can set the actual pdpt_entry to our new, 'non-larger-pages' value
        pdpt_entry->value = new_pdpt_entry_value;
    }

    pt_t* pd = (pt_t*)(vmm_pt_entry_get_address(pdpt_entry) + vmm_page_tables_offset);

    /* Getting the PT */
    pt_entry_t* pd_entry = &pd->entries[PD_INDEX((uint64_t)virtual_address)];
    if(!(pd_entry->value & PT_ENTRY_PRESENT)) {
        vmm_init_pt_entry(pd_entry);
        pd_entry->value |= PT_ENTRY_READ_WRITE;
        pd_entry->value |= PT_ENTRY_PRESENT;
    }

    /* If the PD entry has the larger pages flag set, then create a new page table, initialise all of its entries and remove the larger pages flag */
    if(pd_entry->value & PT_ENTRY_LARGER_PAGES) {
        uint64_t base_phys_addr = vmm_pt_entry_get_address(pd_entry);

        /* We have to create the new pd_entry in a different variable as editing the value in real-time is not thread safe
           and would likely affect where we use newly-allocated pages (as they would not be mapped to anything, causing a page fault) */
        uint64_t new_pd_entry_address = (uint64_t)pmm_allocz() - vmm_page_tables_offset;
        uint64_t new_pd_entry_value = 0;
        vmm_pt_entry_set_address((pt_entry_t*)&new_pd_entry_value, new_pd_entry_address);
        new_pd_entry_value |= PT_ENTRY_READ_WRITE;
        new_pd_entry_value |= PT_ENTRY_PRESENT;

        pt_t* new_pt = (pt_t*)(new_pd_entry_address + vmm_page_tables_offset);
        
        for(uint64_t i = 0; i < 512; i++) {
            pt_entry_t* next_entry = &new_pt->entries[i];
            vmm_pt_entry_set_address(next_entry, base_phys_addr + (i * 0x1000));
            next_entry->value |= PT_ENTRY_READ_WRITE;
            next_entry->value |= PT_ENTRY_PRESENT;
        }

        // Now that everything has been set-up the same as it was before, we can set the actual pd_entry to our new, 'non-larger-pages' value
        pd_entry->value = new_pd_entry_value;
    }

    pt_t* pt = (pt_t*)(vmm_pt_entry_get_address(pd_entry) + vmm_page_tables_offset);
    
    /* Getting the specific page and deleting it */
    pt_entry_t* pt_entry = &pt->entries[PT_INDEX((uint64_t)virtual_address)];
    vmm_invlpg(vmm_pt_entry_get_address(pt_entry));
    pt_entry->value = 0;

    /* Invalidating that entry in the TLB */
    vmm_invlpg((uint64_t)virtual_address);
}

void vmm_init_pt_entry(pt_entry_t* entry) {
    vmm_pt_entry_set_address(entry, (uint64_t)pmm_allocz() - vmm_page_tables_offset);
}

void vmm_pt_entry_set_address(pt_entry_t* entry, uint64_t address) {
    address &= 0x000FFFFFFFFFFFFF;
    entry->value &= 0xFFF0000000000FFF;
    entry->value |= address & 0x000FFFFFFFFFF000;
}

uint64_t vmm_pt_entry_get_address(pt_entry_t* entry) {
    return entry->value & 0x000FFFFFFFFFF000;
}

void vmm_invlpg(uint64_t address) {
    asm volatile("invlpg (%0)" : : "r" (address) : "memory");
}

void vmm_set_pml4(pt_t* new_pml4) {
    asm volatile ("mov %0, %%cr3" : : "r" ((vmm_virt_to_phys((void*)new_pml4))));
    pml4 = new_pml4;
}

bool vmm_has_initialised() {
    return vmm_initialised;
}
