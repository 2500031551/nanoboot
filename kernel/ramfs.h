#ifndef RAMFS_H
#define RAMFS_H

#include <stddef.h>

typedef struct {
    const char *name;
    const char *data;
    size_t size;
} ramfs_file_t;

const ramfs_file_t *ramfs_files(size_t *count);
const ramfs_file_t *ramfs_find(const char *name);

#endif
