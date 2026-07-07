#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define MAX_DIRS 8

struct _DIR {
    int          fd;
    int          in_use;
    struct dirent entry;
};

static struct _DIR dir_pool[MAX_DIRS];

DIR* opendir(const char* path) {
    for (int i = 0; i < MAX_DIRS; i++) {
        if (!dir_pool[i].in_use) {
            int fd = open(path, O_RDONLY | O_DIRECTORY);
            if (fd < 0) return nullptr;
            dir_pool[i].fd = fd;
            dir_pool[i].in_use = 1;
            return &dir_pool[i];
        }
    }
    return nullptr; // pool full
}

struct dirent* readdir(DIR* dir) {
    if (!dir || !dir->in_use) return nullptr;

    uint64_t reclen;
    if (read(dir->fd, &reclen, sizeof(reclen)) <= 0) return nullptr;

    unsigned restlen = (unsigned)(reclen - sizeof(reclen));
    char rest[300];
    if (restlen > sizeof(rest)) return nullptr;
    if (read(dir->fd, rest, restlen) <= 0) return nullptr;

    dir->entry.d_type = (unsigned char)rest[0];
    dir->entry.d_size = *(uint32_t*)(rest + 1);
    char* name = rest + 5;
    int i = 0;
    while (name[i] && i < 255) { dir->entry.d_name[i] = name[i]; i++; }
    dir->entry.d_name[i] = 0;

    return &dir->entry;
}

int closedir(DIR* dir) {
    if (!dir || !dir->in_use) return -1;
    int r = close(dir->fd);
    dir->in_use = 0;
    return r;
}
