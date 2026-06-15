#include <fcntl.h>
#include <stdint.h>
#include "stdarg.h"
__attribute__((noinline))
int open(const char* path, int flags, ...) {
    va_list ap;
    va_start(ap, flags);
    int mode = va_arg(ap, int);
    va_end(ap);
    int result;
    __asm__ __volatile__(
        "int 0x80;"
        : "=a"(result)
    : "a"(8), "D"(0), "S"(path),"d"(flags),"b"(mode)                // rax=8 ("open" syscall)
        : "rcx", "r11", "memory" // rdi는 함수 인자로 전달되므로 클로버시 필요 없음
        );
    return result;
}