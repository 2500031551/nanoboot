# NanoBoot

NanoBoot is a small RV64 RISC-V teaching kernel for QEMU's `virt` machine. It
boots with OpenSBI, enters supervisor mode, enables Sv39, handles traps and SBI
timer interrupts, schedules a fixed process table, and exposes a UART shell.

## Build and run in Ubuntu / WSL

Install the RISC-V bare-metal compiler and emulator in Ubuntu:

```sh
sudo apt update
sudo apt install gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf qemu-system-misc
cd ~/nanoboot
make clean && make
make qemu
```

At the U-mode `nanoboot>` prompt, try `help`, `prasanthi`, `stage`, `about`,
`ps`, `getpid`, `ticks`, `uptime`, `pwd`, `ls`, `cat about.txt`, and
`isolation`. Exit QEMU with **Ctrl+A**, then **X**.

The QEMU command expects OpenSBI at
`/usr/share/qemu/opensbi-riscv64-generic-fw_dynamic.bin`.

## What the demo currently demonstrates

- Boot banner on UART and transition into U mode.
- `ecall` syscall dispatch for read/write, getpid, fork, exit, wait, and sleep.
- Fixed four-slot process table with round-robin timer preemption, child exit,
  blocking wait, wakeup, and process-slot reuse.
- Per-process Sv39 roots and private user stack pages; kernel and UART mappings
  are supervisor-only. `isolation` audits U permission across every stack page.
- U-mode UART shell using read/write ecalls, plus process/timer inspection and
  a read-only RAM FS.

## Current limits

This is an instructional prototype, not a complete POSIX OS. Fork clones the
saved register frame and one stack page with stack-pointer relocation, but not
a heap or writable global memory. There is no ELF loader/exec, persistent disk
filesystem, general user-pointer copying, pipe/dup2, copy-on-write, or
multi-hart scheduler locking. `isolation` audits PTE permissions but is not an
adversarial fault-containment test. The ELF currently links an RWX load segment.

## Project notes

- [Design](docs/DESIGN.md): boot, traps, scheduling, and virtual memory.
- [Week log](docs/WEEK_LOG.md): mapping to the 12-week course plan.
- [Demo checklist](docs/DEMO.md): reproducible acceptance walkthrough.
- [Three-person plan](docs/TEAM_PLAN.md): suggested ownership split.
