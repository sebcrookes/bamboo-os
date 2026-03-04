#pragma once

#include "../../limine/limine.h"

#define PSF1_FONT_MAGIC 0x0436
#define PSF2_FONT_MAGIC 0x864ab572

typedef struct __attribute__((packed)) {
    uint16_t magic;
    uint8_t font_mode;
    uint8_t character_size;
} psf1_header_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t version;
    uint32_t header_size;
    uint32_t flags;
    uint32_t num_glyphs;
    uint32_t bytes_per_glyph;
    uint32_t height;
    uint32_t width;
} psf2_header_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t num_glyphs;
    uint32_t bytes_per_glyph;
} font_info_t;

typedef struct {
    font_info_t info;
    uint32_t mappings[128];
    uint8_t* glyphs;
} bamboo_font_t;

void font_load(bamboo_font_t* font, char* path, struct limine_module_request module_request);
