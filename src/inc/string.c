#include "string.h"

void* memcpy(void* dest, const void* src, size_t n) {
    for(uint64_t i = 0; i < n; i++) {
        ((uint8_t*) dest)[i] = ((uint8_t*) src)[i];
    }

    return dest;
}

void* memset(void* s, int c, size_t n) {
    for(uint64_t i = 0; i < n; i++) {
        ((uint8_t*) s)[i] = (uint8_t) c;
    }

    return s;
}

void* memmove(void* s1, const void* s2, size_t n) {
    if(s1 > s2) { // The destination is at a higher memory address than the source
        for(uint64_t i = n - 1; i >= 0; i--) {
            ((uint8_t*) s1)[i] = ((uint8_t*) s2)[i];
        }
    } else { // The source is at a higher memory address than the destination
        for(uint64_t i = 0; i < n; i++) {
            ((uint8_t*) s1)[i] = ((uint8_t*) s2)[i];
        }
    }
}

int memcmp(const void* s1, const void* s2, size_t n) {
    for(uint64_t i = 0; i < n; i++) {
        uint8_t b1 = ((uint8_t*) s1)[i];
        uint8_t b2 = ((uint8_t*) s2)[i];

        if(b1 > b2) return 1;
        if(b1 < b2) return -1;
    }

    return 0;
}

/* Other string-related functions */

uint64_t strlen(char* str) {
    uint64_t i = 0;

    while(str[i] != 0) {
        i++;
    }

    return i;
}

bool does_string_end_with(char* str, char* end) {
    uint64_t len_str = strlen(str);
    uint64_t len_end = strlen(end);
    
    if(len_str < len_end) return false;

    for(uint64_t i = 0; i < len_end; i++) {
        if(str[len_str - 1 - i] != end[len_end - 1 - i]) {
            return false;
        }
    }

    return true;
}
