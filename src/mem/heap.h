#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct __attribute__((packed)) {
    void* prev; // Points to the segment header of the previous segment
    void* next; // Points to the segment header of the next segment
    uint32_t size; // Holds actual size of segment, without header
    bool free;
} heap_segment_hdr_t;

void heap_init(void* p_base_address);

void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);

void heap_split_segment(heap_segment_hdr_t* curr, uint64_t size);
void heap_merge_forward(heap_segment_hdr_t* curr);
void heap_extend(heap_segment_hdr_t* last, uint64_t num_pages);
