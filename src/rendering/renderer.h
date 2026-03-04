#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "framebuffer.h"
#include "font.h"

/* Provides a simple way of drawing pixels and characters to the screen */

void renderer_init(bamboo_fb_t* fb, bamboo_font_t* font);

/* Rendering functions */

void renderer_set_pixel(uint64_t x, uint64_t y, uint32_t colour);
void renderer_draw_char_at(char c, uint64_t x, uint64_t y, uint32_t colour);
void renderer_draw_char(char c, uint32_t colour);
void renderer_draw_string(char* string, uint32_t colour);
void renderer_draw_number(uint64_t val, uint8_t base, bool is_signed, bool capitalise, uint32_t colour);

void renderer_clear_screen(uint32_t colour);

uint64_t renderer_chars_left_on_line();
uint64_t renderer_chars_per_line();
