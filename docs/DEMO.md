# NanoBoot demo checklist

From Ubuntu in the repository:

```sh
make clean && make
make qemu
```

Expected boot output includes the NanoBoot banner, Sv39 enabled, and entry to
user mode. The scripted user program fills the child slots, exercises fork
failure, waits/reaps children, reuses a slot, calls sleep, and enters the shell.
Look for `Timer preempt`, `Wait returned child PID`, and `Timer woke a sleeping
process` in the console output.

The prompt is a U-mode process. Enter each command separately:

```text
help
prasanthi
whoami
stage
about
pwd
ps
getpid
ticks
uptime
uname
ls
cat about.txt
cat README.md
cat missing.txt
hello
clear
isolation
```

Expected `ls` entries include `welcome.txt`, `about.txt`, `roadmap.txt`, and
`README.md`. The last
`cat` should report that the file was not found. Quit with Ctrl+A, X.

Stage 9 adds `prasanthi`/`whoami` identity output, `stage` and `about` project
information, and `pwd` for the RAMFS root. `ls` and `cat` exercise the read-only
in-memory files.

## Acceptance evidence and known gaps

The `isolation` command audits the U permission on every user stack in every
process root. It verifies page-table configuration but does not yet trigger and
contain an intentional illegal user load. Fork copies the saved registers and
the one user stack page; this demo has no heap or writable global data to clone.
