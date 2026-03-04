#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "../bootdata.h"

/* === Paging explanation (for future reference) ===

   The page map is a tree, and there are 4 levels of page tables in 
   4-level paging - PML4, PDPT, PD, and PT. Each page table (pt_t) holds
   512 entries (pt_entry_t) (and thus each pt is a page long). For example, 
   the PML4 holds 512 entries, each pointing to a PDPT. Then, each PDPT 
   holds 512 entries, each pointing to a PD, and so on.
   
   === Virtual address layout ===

   The first 12 bits are the physical offset in the page (0 - 0xFFF), so
   we ignore these to just get the specific page the data is in.
   
   There are then 4 fields, each 9 bits long (as each table has 512 entries).
   These are the indices gathered in the macros below, telling us which tables
   to read/write to (basically which branches of the tree we need to go down). 
   
   The last 16 bits are the sign extension, and we ignore these. */

/* Macros for finding indices of each table */
#define PML4_INDEX(v) ((v >> 39) & 0x1FF) // Page map level 4 table
#define PDPT_INDEX(v) ((v >> 30) & 0x1FF) // Page directory pointer table
#define PD_INDEX(v) ((v >> 21) & 0x1FF) // Page directory
#define PT_INDEX(v) ((v >> 12) & 0x1FF) // Page table

/* Flags for entries in tables */
#define PT_ENTRY_PRESENT 0b1
#define PT_ENTRY_READ_WRITE 0b10
#define PT_ENTRY_USER_ACCESS 0b100
#define PT_ENTRY_WRITE_THROUGH 0b1000
#define PT_ENTRY_CACHE_DISABLED 0b10000
#define PT_ENTRY_ACCESSED 0b100000
#define PT_ENTRY_LARGER_PAGES 0b10000000

/* Every entry in every page table is stored in this format */
typedef struct __attribute__((packed)) {
   uint64_t value;

   /* Structure of 'value':
   1 bit: present
   1 bit: read_write
   1 bit: user_access
   1 bit: write_through
   1 bit: cache_disabled
   1 bit: accessed
   1 bit: reserved0
   1 bit: larger_pages
   1 bit: reserved1
   3 bits: available
   52 bits: address // The addresses stored in the pt_entries are ALWAYS physical, so page tables must be given as physical addresses, not virtual (we MUST use the offset)
   */
} pt_entry_t;

/* Each page table contains 512 entries to the tables below it, and thus is 1 page long */
typedef struct __attribute__((aligned(0x1000))) {
   pt_entry_t entries[512];
} pt_t;

void vmm_init(boot_data_t* bootdata);
void* vmm_virt_to_phys(void* virtual_address);
void vmm_map_page(void* physical_address, void* virtual_address);
void vmm_map_page_flags(void* physical_address, void* virtual_address, uint8_t flags);
void vmm_map_page_pml4(pt_t* p_pml4, void* physical_address, void* virtual_address, uint8_t flags);
void vmm_map_pages_pml4(pt_t* p_pml4, void* physical_address, void* virtual_address, uint64_t num_pages);

void vmm_unmap_page(void* virtual_address);

void vmm_init_pt_entry(pt_entry_t* entry);
void vmm_pt_entry_set_address(pt_entry_t* entry, uint64_t address);
uint64_t vmm_pt_entry_get_address(pt_entry_t* entry);

void vmm_invlpg(uint64_t address);

void vmm_set_pml4(pt_t* new_pml4);

bool vmm_has_initialised();
