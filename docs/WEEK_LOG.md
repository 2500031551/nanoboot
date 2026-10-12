# NanoBoot week-by-week build log

This log maps the current source to the supplied syllabus. It describes the
current implementation, not a claim that every course topic is complete.

| Week | Focus | NanoBoot status |
|---|---|---|
| 1 | User/kernel boundary and boot | OpenSBI/QEMU boot, UART banner, U-mode `sret`, `ecall` syscall path. |
| 2 | Linker, stacks, memory | Linker script, BSS, boot stack, static kernel/process stacks. |
| 3 | Console and process table | UART input/output and four-slot process table. |
| 4 | Processes and scheduling | Saved trap-frame and stack clone with child return value zero, stack-pointer relocation, and round-robin preemption. Heap/global writable memory is not cloned. |
| 5 | Syscalls and shell commands | read/write/getpid/fork/exit/wait/sleep dispatch; user-mode command loop. No ELF loader/exec. |
| 6 | Timer traps | SBI TIME re-arm, tick counter, trap save/restore and preemption. |
| 7 | Pipes and blocking I/O | Sleep/wakeup exists. UART shell input is polling; pipes, descriptors, and `dup2` are not implemented. |
| 8 | Virtual memory | Per-process Sv39 roots and separate user stack mappings; `isolation` audits U access bits. No adversarial fault containment or COW. |
| 9 | Files | Small read-only in-memory file table with `ls` and `cat`; no disk filesystem. |
| 10 | Threads and lifecycle | Fork/exit/wait and slot reuse demo. No threads or parallel execution. |
| 11 | Synchronization | Single-hart sleep/wakeup. No spinlock or second hart. |
| 12 | Integration | QEMU target, design notes, and demo checklist. Run and capture the checklist on the target Ubuntu installation. |
