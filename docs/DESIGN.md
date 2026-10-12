# NanoBoot design

## Boot and address space

QEMU loads the ELF at `0x80200000` and starts OpenSBI. The assembly entry
establishes the boot stack and transfers to `kmain` in supervisor mode. The
kernel initializes the fixed process table, page tables, trap vector, and SBI
TIME timer, then uses `sret` to enter the first user program. Sv39 uses a
separate root page table per process. Kernel memory and UART are identity
mapped supervisor-only; user text and each process's stack carry U permission
only in that process's root. SUM is enabled because supervisor trap code saves
frames on user stacks.

The page-table allocator is static and sized for this small kernel. It is not a
general physical page allocator. The mapping range is identity-based and
limited to the QEMU layout; changing the kernel size or platform requires
reviewing those bounds.

## Traps and syscalls

`stvec` points to the assembly trap entry. It saves the integer register frame
and `sepc`, `sstatus`, and `scause`, then calls C. Synchronous environment calls
decode `a7`, take the first argument from `a0`, and return the result in `a0`.
The handler advances `sepc` by four bytes before resuming after `ecall`. Timer
interrupts re-arm the SBI TIME event, increment ticks, wake expired sleepers,
and may select another saved user frame. Unsupported traps stop with diagnostic
CSR values.

The present `write` syscall prints a fixed demonstration string instead of
copying an arbitrary user buffer. A production syscall must validate and copy
user pointers through the current process address space.

## Process lifecycle and scheduling

The process table has four static slots, private kernel stacks, trap frames,
and user stacks. `fork` clones the saved register frame and the one user stack
page, relocates register/stack words that point into that page, returns the
child PID to the parent, and resumes the child after `ecall` with zero. It does
not clone a heap or writable global data. Timer preemption rotates READY
processes. `exit` marks a child EXITED and wakes its waiting parent;
`wait` blocks until a child exits and reclaims the slot. `sleep(n)` blocks for
`n` timer ticks, at 100 ticks per second. There is one hart, so no scheduler
spinlock is required or implemented.

## Shell and RAM filesystem

The command loop runs in U mode. UART input and bounded output use read/write
ecalls; write accepts buffers only from the current process's stack. Commands
needing kernel-owned state (`ps`, timer status, `ls`, `cat`, and the page-table
audit) use a bounded command syscall that copies the command into a kernel
buffer before parsing. The RAM filesystem is read-only; there is no disk image,
redirection, or general user-pointer copying API.

## Security / correctness boundaries

Separate roots and U permission bits establish the intended user-stack access
boundary. `isolation` checks the leaf U permission for every process stack in
every root. It audits page-table configuration; it is not an adversarial fault
containment test. Kernel pages are mapped RWX and the linker emits an RWX
segment warning. These limitations should be fixed before treating this kernel
as secure or production-ready.
