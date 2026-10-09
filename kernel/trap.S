.section .text
.globl trap_entry

trap_entry:
    # Read the address of the instruction that caused the trap
    csrr t0, sepc

    # Save it in memory
    la t1, saved_sepc
    sd t0, 0(t1)

    # Call the C trap handler
    call trap_handler

    # Load the saved program counter
    la t1, saved_sepc
    ld t0, 0(t1)

    # Skip the ECALL instruction (4 bytes)
    addi t0, t0, 4
    csrw sepc, t0

    # Return from the trap
    sret


.section .bss
.align 3

.globl saved_sepc
saved_sepc:
    .space 8
