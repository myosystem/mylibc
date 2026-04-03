#ifndef __STRING_H__
#define __STRING_H__
#ifdef __cplusplus
extern "C" {
#endif
void* memcpy(void* dest, const void* src, unsigned long long size);

void* memset(void* dest, int value, unsigned long long size);

int strcmp(const char* s1, const char* s2);

int strncmp(const char* s1, const char* s2, unsigned long long n);

int strcasecmp(const char* s1, const char* s2);

void* strncpy(char* dest, const char* src, unsigned long long n);
#ifdef __cplusplus
}
#endif
#endif // __STRING_H__