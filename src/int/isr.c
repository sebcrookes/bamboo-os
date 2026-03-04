#include "isr.h"

#include "../inc/stdio.h"
#include "../inc/faults.h"

__attribute__((interrupt)) void isr_default_handler(interrupt_state_t* state) {
    faults_panic("Unhandled interrupt");
    while(1) {}
}

__attribute__((interrupt)) void isr_div_by_zero_handler(interrupt_state_t* state) {
    faults_panic("Division by zero");
    while(1) {}
}

__attribute__((interrupt)) void isr_gp_fault_handler(interrupt_state_t* state) {
    faults_panic("General Protection fault");
    while(1) {}
}

__attribute__((interrupt)) void isr_page_fault_handler(interrupt_state_t* state) {
    faults_panic("Page fault");
    while(1) {}
}
