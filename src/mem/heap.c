#include "heap.h"

#include "../inc/stdio.h"
#include "../inc/string.h"

#include "pmm.h"
#include "vmm.h"

/* 
 * KernelHeap:
 * The heap consists of a linked list of segments. You can tell if a segment is the first / last if:
 * First: the 'prev' variable contains value 0
 * Last: the 'next' variable contains value 0
 */

uint64_t heap_base_address = 0;
uint64_t heap_size = 0; // In pages

void heap_init(void* p_base_address) {
    heap_base_address = (uint64_t) p_base_address;

    /* Allocate + map the first page */
    void* first_page = pmm_allocz();
    vmm_map_page(first_page, (void*)heap_base_address);
    heap_size = 1;

    /* Set up first segment */
    heap_segment_hdr_t* hdr = (heap_segment_hdr_t*) heap_base_address;
    hdr->prev = 0;
    hdr->next = 0;
    hdr->free = true;
    hdr->size = 0x1000 - sizeof(heap_segment_hdr_t);

    printf("%C[Heap]%C - Initialised heap allocator at address %X\n", COLOUR_KERNEL_INFO, COLOUR_PRINT, heap_base_address);
}

void* malloc(size_t size) {
    bool searching = true;

    heap_segment_hdr_t* hdr = (heap_segment_hdr_t*) heap_base_address;

    while(searching) {
        if(hdr->free) {
            if(hdr->size == size) {
                hdr->free = false;
                return (void*)(((uint8_t*) hdr) + sizeof(heap_segment_hdr_t));
            }
            if(hdr->size > size) {
                heap_split_segment(hdr, size);
                hdr->free = false;
                return (void*)(((uint8_t*) hdr) + sizeof(heap_segment_hdr_t));
            }
        }

        if(hdr->next == 0) {
            searching = false;
        } else {
            hdr = (heap_segment_hdr_t*) hdr->next;
        }
    }

    // Allocate enough pages to have enough space for a block of given size

    uint64_t bytes_needed = size;
    uint64_t pages_needed = (bytes_needed / 0x1000) + 1;

    heap_extend(hdr, pages_needed);

    if(hdr->size == size) {
        hdr->free = false;
        return (void*)(((uint8_t*) hdr) + sizeof(heap_segment_hdr_t));
    }
    if(hdr->size > size) {
        heap_split_segment(hdr, size);
        hdr->free = false;
        return (void*)(((uint8_t*) hdr) + sizeof(heap_segment_hdr_t));
    }

    return NULL;
}

void free(void* ptr) {
    // Checking if pointer is valid
    heap_segment_hdr_t* hdr = (heap_segment_hdr_t*) heap_base_address;
    void* hdr_addr = (void*)((uint64_t) ptr - sizeof(heap_segment_hdr_t));
    bool valid = false;
    do {
        if(hdr_addr == (void*) hdr) {
            valid = true;
            break;
        } else if(hdr->next == 0) {
            break;
        } else {
            hdr = (heap_segment_hdr_t*) hdr->next;
        }
    } while((void*) hdr <= hdr_addr);

    if(!valid) return;

    // Setting the header to free, and checking segments around it (to try to merge them)

    heap_segment_hdr_t* header = (heap_segment_hdr_t*) hdr_addr;
    header->free = true;

    if(header->prev != NULL) {
        if(((heap_segment_hdr_t*) header->prev)->free) {
            header = (heap_segment_hdr_t*) header->prev;
            heap_merge_forward(header);
        }
    }

    if(((heap_segment_hdr_t*) header->next)->free) {
        heap_merge_forward(header);
    }
}

void* realloc(void* ptr, size_t size) {
    // Checking if pointer is valid
    heap_segment_hdr_t* hdr = (heap_segment_hdr_t*) heap_base_address;
    void* hdr_addr = (void*)(((uint64_t) ptr) - sizeof(heap_segment_hdr_t));
    bool valid = false;
    do {
        if(hdr_addr == (void*) hdr) {
            valid = true;
            break;
        } else if(hdr->next == 0) {
            break;
        } else {
            hdr = (heap_segment_hdr_t*) hdr->next;
        }
    } while((void*) hdr <= hdr_addr);

    if(!valid) return malloc(size);
    
    void* new_ptr = malloc(size);

    heap_segment_hdr_t* old_header = (heap_segment_hdr_t*) hdr_addr;

    if(old_header->size < size) {
        memcpy(new_ptr, ptr, old_header->size);
    } else {
        memcpy(new_ptr, ptr, size);
    }

    return new_ptr;
}

/* ========= Private Functions ========= */

/* Splits a segment defined by 'curr' so that size fits exactly into curr and a
   new segment is created just after it */
void heap_split_segment(heap_segment_hdr_t* curr, uint64_t size) {
    heap_segment_hdr_t* new_hdr = (heap_segment_hdr_t*)(((uint8_t*) curr) + sizeof(heap_segment_hdr_t) + size);

    // If there is not enough space to create the next header, leave it
    if(curr->size <= size + sizeof(heap_segment_hdr_t)) {
        return;
    }
    
    new_hdr->prev = (void*) curr;
    new_hdr->free = true;

    if(curr->next == 0) {
        new_hdr->next = 0;
    } else {
        new_hdr->next = curr->next;
    }

    curr->next = (void*) new_hdr;
    new_hdr->size = curr->size - (size + sizeof(heap_segment_hdr_t));
    curr->size = size;
}

/* Merges the current segment with the one in front */
void heap_merge_forward(heap_segment_hdr_t* curr) {
    curr->size += sizeof(heap_segment_hdr_t) + ((heap_segment_hdr_t*) curr->next)->size;
    curr->next = ((heap_segment_hdr_t*)(curr->next))->next;

    memset((void*) curr->next, 0, sizeof(heap_segment_hdr_t));
}

/* Extends the segment at the end of the heap by a number of pages, given by 'num_pages' */
void heap_extend(heap_segment_hdr_t* last, uint64_t num_pages) {
    for(uint64_t i = 0; i < num_pages; i++) {
        void* new_page = pmm_allocz();
        vmm_map_page(new_page, (void*)((uint64_t) heap_base_address + (heap_size * 0x1000)));
        heap_size++;

        last->size += 0x1000;
    }
}
