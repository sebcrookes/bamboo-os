#include "font.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "../inc/string.h"
#include "../inc/faults.h"

void font_load(bamboo_font_t* font, char* path, struct limine_module_request module_request) {
    /* Checking if module_request was successful */
    struct limine_module_response* module_response = module_request.response;
    if(module_response == NULL) {
        faults_hang();
    }

    /* Getting the limine_file for our font */
    struct limine_file* font_file = NULL;

    for(size_t i = 0; i < module_response->module_count; i++) {
        struct limine_file* f = module_response->modules[i];
        if(does_string_end_with(f->path, path)) {
            font_file = f;
            break;
        }
    }

    if(font_file == NULL) faults_hang();

    /* Loading the header and checking magic value */

    psf1_header_t* psf1_hdr = (psf1_header_t*) font_file->address;

    if(psf1_hdr->magic != PSF1_FONT_MAGIC) {
        
        psf2_header_t* psf2_hdr = (psf2_header_t*) font_file->address;

        if(psf2_hdr->magic != PSF2_FONT_MAGIC) {
            faults_hang();
        }

        /* Load the PSF2 font */
        font->info.width = psf2_hdr->width;
        font->info.height = psf2_hdr->height;
        font->info.num_glyphs = psf2_hdr->num_glyphs;
        font->info.bytes_per_glyph = psf2_hdr->bytes_per_glyph;

        uint64_t unicode_table_offset = sizeof(psf2_header_t) + psf2_hdr->num_glyphs * psf2_hdr->bytes_per_glyph;
        uint8_t* unicode_table = (uint8_t*)(psf2_hdr) + unicode_table_offset;

        // Building a rudimentary lookup table from the provided unicode table (we don't have malloc, so it is fixed size for now)
        if(psf2_hdr->flags & 0b1) {
            for(int ascii = 0; ascii < 128; ascii++) {                
                int line = 0; // The line of the table is equivalent to the glyph number of those unicode codepoints
                int i = 0;
                bool found = false;
                do {
                    if(unicode_table[i] == ascii) {
                        font->mappings[ascii] = line;
                        found = true;
                        break;
                    }

                    if(unicode_table[i] == 0xFF) { // The end of the line has been reached
                        line++;
                    }

                    // Skipping all characters which aren't one byte in UTF-8
                    int charlen = 1;

                    if((unicode_table[i] & 0xE0) == 0xC0) {
                        charlen = 2;
                    } else if((unicode_table[i] & 0xF0) == 0xE0) {
                        charlen = 3;
                    } else if((unicode_table[i] & 0xF8) == 0xF0) {
                        charlen = 4;
                    }

                    i += charlen;
                } while(!found && line < 128 && font_file->size > unicode_table_offset + i);
            }
        }

        font->glyphs = (uint8_t*)(psf2_hdr + 1); // The glyphs are just after the header
    } else {

        /* Load the PSF1 font */
        font->info.width = 8;
        font->info.height = psf1_hdr->character_size;
        font->info.num_glyphs = 128;
        font->info.bytes_per_glyph = font->info.height;
    
        font->glyphs = (uint8_t*)(psf1_hdr + 1);

        // Mappings in PSF1 are fixed - there is no table
        for(int i = 0; i < 128; i++) {
            font->mappings[i] = i;
        }
    }
}
