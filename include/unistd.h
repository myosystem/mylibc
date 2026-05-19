#ifndef __UNISTD_H__
#define __UNISTD_H__
#include <sys/types.h>
#include <stddef.h>
ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
int close(int fd);
pid_t fork(void);
void _exit(int status);
#endif /*__UNISTD_H__*/