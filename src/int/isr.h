#pragma once

#include <stdint.h>

typedef struct {
    uint64_t iret_rip;
    uint64_t iret_cs;
    uint64_t iret_flags;
    uint64_t iret_rsp;
    uint64_t iret_ss;
} interrupt_state_t;

__attribute__((interrupt)) void isr_default_handler(interrupt_state_t* state);

__attribute__((interrupt)) void isr_div_by_zero_handler(interrupt_state_t* state);

__attribute__((interrupt)) void isr_gp_fault_handler(interrupt_state_t* state);
__attribute__((interrupt)) void isr_page_fault_handler(interrupt_state_t* state);
