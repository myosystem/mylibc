#ifndef __STDLIB_H__
#define __STDLIB_H__
#include "stddef.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
void* malloc(size_t size);
void free(void* ptr);
void exit(int status);
int atexit(void (*func)(void));
int __cxa_atexit(void (*func)(void*), void* arg, void* dso);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __STDLIB_H__