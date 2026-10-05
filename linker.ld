OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
    . = 0x80200000;

    .text : {
        *(.text.entry)
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss : ALIGN(4096)
    {
        __bss_start = .;

        *(.bss .bss.*)
        *(COMMON)

        __bss_end = .;
    }
}
