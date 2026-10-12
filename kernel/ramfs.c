#include "ramfs.h"

static const char welcome_text[] =
    "Welcome to NanoBoot. Type help to list shell commands.\n";
static const char about_text[] =
    "NanoBoot is a small RISC-V supervisor-mode teaching kernel.\n"
    "It demonstrates traps, syscalls, scheduling, timers, and Sv39.\n";
static const char roadmap_text[] =
    "Implemented demo: QEMU boot, timer traps, fork/exit/wait/sleep,\n"
    "round-robin preemption, per-process page-table roots, UART shell.\n"
    "Current limits: no ELF exec, disk image, pipes, COW, or multi-hart.\n";
static const char readme_text[] =
    "NanoBoot: RV64 RISC-V teaching kernel for QEMU virt.\n"
    "Commands: help, hello, ps, getpid, ticks, uptime, uname, pwd, ls.\n"
    "Use cat about.txt or cat roadmap.txt to read more.\n"
    "Stage 9 demonstrates a read-only in-memory filesystem.\n";

static const ramfs_file_t files[] = {
    {"welcome.txt", welcome_text, sizeof(welcome_text) - 1},
    {"about.txt", about_text, sizeof(about_text) - 1},
    {"roadmap.txt", roadmap_text, sizeof(roadmap_text) - 1},
    {"README.md", readme_text, sizeof(readme_text) - 1},
};

const ramfs_file_t *ramfs_files(size_t *count)
{
    if (count != 0) {
        *count = sizeof(files) / sizeof(files[0]);
    }
    return files;
}

const ramfs_file_t *ramfs_find(const char *name)
{
    for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
        const char *a = name;
        const char *b = files[i].name;
        while (*a && *b && *a == *b) {
            a++;
            b++;
        }
        if (*a == '\0' && *b == '\0') {
            return &files[i];
        }
    }
    return 0;
}
