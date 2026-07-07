#ifndef __DIRENT_H__
#define __DIRENT_H__
#include <stdint.h>

#define DT_REG 0
#define DT_DIR 1

// POSIX-compatible directory entry.
// Returned by readdir().
struct dirent {
    unsigned char d_type;    // DT_REG or DT_DIR
    uint32_t      d_size;    // file size (extension)
    char          d_name[256];
};

// Opaque directory stream (use DIR* only)
typedef struct _DIR DIR;

DIR*           opendir(const char* path);
struct dirent* readdir(DIR* dir);
int            closedir(DIR* dir);

// Low-level raw format from kernel read():
// [uint64_t reclen][uint8_t type][uint32_t size][char name[]\0]
// Use with: fd = open(path, O_DIRECTORY); read(fd, &reclen, 8); ...

#endif
