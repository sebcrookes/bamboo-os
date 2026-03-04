#pragma once

#include "../bootdata.h"

void pmm_init(boot_data_t* bootdata);

uint64_t pmm_phys_to_virt(uint64_t phys);
uint64_t pmm_virt_to_phys(uint64_t virt);

uint64_t pmm_get_address_by_index(uint64_t index);
uint64_t pmm_get_index_by_address(uint64_t address);

bool pmm_is_usable_page_allocated_by_index(uint64_t index);
void pmm_set_usable_page_allocated_by_index(uint64_t index, bool allocated);

void* pmm_alloc_phys();
void* pmm_alloc();
void* pmm_allocz();
void pmm_free(void* page);

uint64_t pmm_get_total_usable_memory();
uint64_t pmm_get_current_free_memory();
