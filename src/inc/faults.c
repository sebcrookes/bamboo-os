#include "faults.h"

#include <stdbool.h>

#include "../rendering/renderer.h"
#include "stdio.h"

#define FAULTS_PANIC_INLINE 1

/**
 * Causes the current CPU to hang indefinitely.
 */
__attribute__((noreturn)) void faults_hang() {
    while(true) {
        asm("hlt");
    }
}

/**
 * Displays an error message, and then causes all CPUs to hang indefinitely.
 * 
 * @param message the error message to display
 */
__attribute__((noreturn)) void faults_panic(char* message) {
    if(FAULTS_PANIC_INLINE) {
        printf("\n%C[Faults]%C - An unexpected fatal error occurred: %s\n", COLOUR_PRINT_ERROR, COLOUR_PRINT, message);
        faults_hang();
    }

    renderer_clear_screen(0xFF0000);
    renderer_draw_string(message, 0xFFFFFF);

    faults_hang();
}

/**
 * Checks that the given condition is met - if not, a kernel panic is triggered with the given message.
 * 
 * @param condition the condition to check
 * @param failure_message the message to display during the kernel panic if the condition was not met
 */
void faults_assert(bool condition, char* failure_message) {
    if(condition) return;

    faults_panic(failure_message);
}
