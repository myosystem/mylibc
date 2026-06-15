#ifndef __UNISTD_H__
#define __UNISTD_H__
#include <sys/types.h>
#include <stddef.h>
#ifndef __OFF_T_DEFINED
#define __OFF_T_DEFINED
typedef signed long long off_t;
#endif
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
int close(int fd);
off_t lseek(int fd, off_t offset, int whence);
pid_t fork(void);
void _exit(int status);
int dup2(int oldfd, int newfd);
int dup3(int oldfd, int newfd, int flags);
#endif /*__UNISTD_H__*/