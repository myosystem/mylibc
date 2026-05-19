#ifndef __STDIO_H__
#define __STDIO_H__
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
typedef __builtin_va_list va_list;
#endif
#define _IOFBF  0
#define _IOLBF  1
#define _IONBF  2
#define EOF (-1)
struct _FILE;
typedef struct _FILE FILE;
extern FILE* stdout;
extern FILE* stdin;
extern FILE* stderr;
FILE* fopen(const char* pathname, const char* mode);
int fclose(FILE* stream);
int printf(const char* fmt, ...);
int vprintf(const char* fmt, va_list ap);
int fprintf(FILE* stream, const char* fmt, ...);
int vfprintf(FILE* stream, const char* fmt, va_list ap);

int scanf(const char* fmt, ...);
int vscanf(const char* fmt, va_list ap);
int fscanf(FILE* stream, const char* fmt, ...);
int vfscanf(FILE* stream, const char* fmt, va_list ap);

int fflush(FILE* stream);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __STDIO_H__