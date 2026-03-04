OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kentry)

PHDRS {
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS {
    . = 0xFFFFFFFF80000000;
    PROVIDE(__KERNEL_START__ = .);

    . = ALIGN(CONSTANT(MAXPAGESIZE));
    .text : {
        *(.text)
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));
    .rodata : {
        *(.rodata)
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));
    .data : {
        *(.data)
        KEEP (*(.reqs_start))
        KEEP(*(.reqs))
        KEEP(*(.reqs_end))
    } :data

    .bss : {
        *(.bss)
    } :data

    PROVIDE(__KERNEL_END__ = .);
}
