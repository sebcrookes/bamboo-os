#pragma once

#include <stddef.h>
#include <stdint.h>
#include "../../limine/limine.h"

typedef struct {
    uint32_t* buffer;
    uint64_t width;
    uint64_t height;
    uint64_t pitch; // Number of bytes between start of one scanline to the next
} bamboo_fb_t;

void framebuffer_init(bamboo_fb_t* fb, struct limine_framebuffer_request framebuffer_request);
