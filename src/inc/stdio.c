#include "stdio.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "../rendering/renderer.h"

uint32_t print_colour = 0xFFFFFF;

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    uint32_t print_colour_start = print_colour;

    for(int i = 0; format[i] != 0; i++) {
        if(format[i] == '%') {
            switch(format[i + 1]) {
                case 'c': {
                    char val = (char) va_arg(args, int); // Char is promoted to int when passed through '...'
                    renderer_draw_char(val, print_colour);
                } break;
                case 's': {
                    char* val = va_arg(args,  char*);
                    renderer_draw_string(val, print_colour);
                } break;
                case 'u': {
                    uint64_t val = va_arg(args, uint64_t);
                    renderer_draw_number(val, 10, false, true, print_colour);
                } break;
                case 'd': {
                    int32_t val = va_arg(args, int32_t);
                    renderer_draw_number((uint64_t)val, 10, true, true, print_colour);
                } break;
                case 'X': {
                    uint64_t val = va_arg(args, uint64_t);
                    renderer_draw_number(val, 16, false, true, print_colour);
                } break;
                case 'x': {
                    uint64_t val = va_arg(args, uint64_t);
                    renderer_draw_number(val, 16, false, false, print_colour); // TODO: support lower-case hex printing
                } break;

                case 'C': { // Change print colour
                    uint32_t col = va_arg(args, uint32_t);
                    print_colour = col;
                } break;
            }

            i++;
        } else {
            if(format[i] == ' ' || i == 0) {
                uint64_t chars_left = renderer_chars_left_on_line();

                if(chars_left < renderer_chars_per_line()) {
                    bool is_space = false;
                    for(uint64_t j = 1; j < chars_left + 1; j++) {
                        if(format[i + j] == '\0') {
                            is_space = true;
                            break;
                        }
                        if(format[i + j] == ' ') {
                            is_space = true;
                        }
                    }

                    if(!is_space) {
                        renderer_draw_char('\n', 0);
                        if(i == 0) renderer_draw_char(' ', 0);
                    }
                }
            }
            renderer_draw_char(format[i], print_colour);
        }
    }

    print_colour = print_colour_start; // Return to the print colour from before we printed

    va_end(args);
}
