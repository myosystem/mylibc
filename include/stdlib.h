#ifndef __STDLIB_H__
#define __STDLIB_H__
#include "stdint.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
void* malloc(size_t size);
void free(void* ptr);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __STDLIB_H__