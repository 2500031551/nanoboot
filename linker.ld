OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
    . = 0x80200000;

    .text : {
        *(.text.entry)
        *(.text)
        *(.text.*)
    }

    . = ALIGN(4096);

    __usertext_start = .;
    .usertext : {
        *(.usertext)
    }
    __usertext_end = .;
    . = ALIGN(4096);
    __userrodata_start = .;
    .rodata : {
        *(.rodata .rodata.*)
    }
    __userrodata_end = .;

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
