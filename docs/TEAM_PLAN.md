# Three-person NanoBoot work split

Use three feature branches and integrate through reviewed pull requests. Keep
the existing QEMU demo passing after each merge. Replace Member 1/3 with the
actual names agreed by the team; the known contributor for Member 2 is
Prasanthi Lakshmi (`2500030676`).

| Member | Main ownership | Work mapped to the draft | Deliverables |
|---|---|---|---|
| Member 1 — existing project owner | Boot and low-level interface | Weeks 1–6: reset/entry, linker and stacks, UART/trap setup, timer interrupts, syscall ABI, review integration with the original Stage 1–6 work. | Boot path and trap/syscall notes; stable `make qemu` baseline. |
| Member 2 — Prasanthi | Processes and memory | Weeks 7–9: sleep/wakeup, process lifecycle and slot reuse, round-robin scheduling, Sv39 user stacks, RAMFS shell commands. | Process/VM/RAMFS changes, command demo, tests for `wait`, `sleep`, and stack permissions. |
| Member 3 — integration lead | Final integration and submission | Weeks 10–12: acceptance run, test/demo script, design/build log, clean README, capture demo output, check pull requests and resolve integration issues. Coordinate optional pipes/exec work with the relevant owner. | Reproducible demo, final documentation, tagged release/PR after team review. |

## Suggested sequence

1. Member 1 publishes the known-good boot/syscall baseline and documents the
   syscall numbers and trap-frame layout.
2. Member 2 builds process, VM, and RAMFS features on a branch based on that
   baseline; do not edit generated `.o`/`.elf` files into commits.
3. Member 3 runs `make clean && make` and `bash tests/demo.sh` on the combined
   branch, checks each acceptance point, and writes down any remaining gaps.
4. Merge one reviewed PR at a time. The repository owner merges to the shared
   default branch after all three members agree; do not force-push shared work.

## Keep ownership clear

Agree on names and exact task ownership before presenting the split. One
person owns each code area, while another reviews it. Everyone contributes to
the final demo and viva preparation. The optional COW and multi-hart stretch
goals should be assigned only after the core integration is stable.
