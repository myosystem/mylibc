#ifndef __FCNTL_H__
#define __FCNTL_H__

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define O_CREAT     0x40
#define O_DIRECTORY 0x10000

int open(const char* path, int flags, ...);
#endif /*__FCNTL_H__*/