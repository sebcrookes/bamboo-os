#include "renderer.h"

#include "../inc/string.h"
#include "../inc/stdio.h"

bamboo_fb_t* r_fb;
bamboo_font_t* r_font;

// Position of terminal cursor
uint64_t r_x;
uint64_t r_y;
bool r_on_second_half;

void renderer_init(bamboo_fb_t* fb, bamboo_font_t* font) {
    r_fb = fb;
    r_font = font;
    r_on_second_half = false;
}

void renderer_set_pixel(uint64_t x, uint64_t y, uint32_t colour) {
    if(x < r_fb->width && y < r_fb->height) {
        r_fb->buffer[(y * r_fb->pitch / sizeof(uint32_t)) + x] = colour;
    }
}

void renderer_draw_char_at(char c, uint64_t x, uint64_t y, uint32_t colour) {
    if((uint32_t) c >= r_font->info.num_glyphs || c >= 128) return;

    uint8_t* glyph = r_font->glyphs + (r_font->mappings[(uint8_t) c] * r_font->info.bytes_per_glyph);
    uint32_t pitch = r_font->info.bytes_per_glyph / r_font->info.height;
    
    for(uint32_t y_off = 0; y_off < r_font->info.height; y_off++) {
        for(uint32_t x_off = 0; x_off < r_font->info.width; x_off++) {
            uint8_t bits = glyph[(y_off * pitch) + (x_off / 8)];
            uint8_t bit = bits >> (7 - x_off % 8) & 1;
            if(bit) renderer_set_pixel(x_off + x, y_off + y, colour);
        }
    }
}

void renderer_draw_char(char c, uint32_t colour) {
    if(c == '\n') {
        r_y += r_font->info.height;
        if(r_on_second_half) {
            r_x = r_fb->width / 2 + r_font->info.width;
        } else {
            r_x = 0;
        }
        return;
    }

    if(!r_on_second_half && r_x >= r_fb->width / 2) {
        r_y += r_font->info.height;
        r_x = 0;
    } else if(r_on_second_half && r_x >= r_fb->width) {
        r_y += r_font->info.height;
        r_x = r_fb->width / 2 + r_font->info.width;
    }

    if(r_y >= r_fb->height) {
        if(!r_on_second_half) {
            r_on_second_half = true;
            r_x = r_fb->width / 2  + r_font->info.width;
            r_y = 0;
        } else {
            r_on_second_half = false;
            r_x = 0;
            r_y = 0;
            renderer_clear_screen(COLOUR_BACKGROUND);
        }
    }

    renderer_draw_char_at(c, r_x, r_y, colour);
    r_x += r_font->info.width;
}

void renderer_draw_string(char* string, uint32_t colour) {
    for(uint64_t i = 0; i < strlen(string); i++) {
        if(string[i] == ' ') {
            uint64_t chars_left = renderer_chars_left_on_line();

            if(chars_left < renderer_chars_per_line()) {
                bool is_space = false;
                for(uint64_t j = 0; j < chars_left; j++) {
                    if(string[i + j] == '\0') {
                        is_space = true;
                        break;
                    }
                    if(string[i + j] == ' ') {
                        is_space = true;
                    }
                }

                if(!is_space) {
                    renderer_draw_char('\n', 0);
                }
            }
        }
        renderer_draw_char(string[i], colour);
    }
}

void renderer_draw_number(uint64_t val, uint8_t base, bool is_signed, bool capitalise, uint32_t colour) {
    if(base > 20) return;

    char buf[30]; // The string, in reverse
    int i = 0;

    uint64_t current_div = val;

    if(is_signed) {
        int64_t signed_val = (int64_t) val;

        if(signed_val >= 0) {
            current_div = val;
        } else {
            renderer_draw_char('-', colour);
            current_div = ~((uint64_t) signed_val) + 1;
        }
    }

    do {
        uint64_t div = current_div / base; // We keep going with this one
        uint64_t mod = current_div % base;

        current_div = div;

        if(mod < 10) {
            buf[i] = (char)(mod + '0');
        } else {
            if(capitalise) {
                buf[i] = (char)(mod - 10 + 'A');
            } else {
                buf[i] = (char)(mod - 10 + 'a');
            }
        }

        i++;
    } while(current_div > 0);

    if(base == 16) renderer_draw_string("0x", colour);
    if(base == 2) renderer_draw_string("0b", colour);

    for(int j = i - 1; j >= 0; j--) {
        renderer_draw_char(buf[j], colour);
    }
}

void renderer_clear_screen(uint32_t colour) {
    for(uint64_t y = 0; y < r_fb->height; y++) {
        for(uint64_t x = 0; x < r_fb->width; x++) {
            r_fb->buffer[(y * r_fb->pitch / sizeof(uint32_t)) + x] = colour;
        }
    }

    r_x = 0;
    r_y = 0;
}

uint64_t renderer_chars_left_on_line() {
    if(!r_on_second_half) {
        return ((r_fb->width / 2) - r_x) / r_font->info.width;
    } else {
        return (r_fb->width - r_x) / r_font->info.width;
    }
}

uint64_t renderer_chars_per_line() {
    return (r_fb->width / 2) / r_font->info.width;
}
