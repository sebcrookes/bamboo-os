[bits 64]

_idt_load:
    lidt [rdi]
    ret

GLOBAL _idt_load