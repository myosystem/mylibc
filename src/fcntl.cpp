#include <fcntl.h>
#include <stdint.h>
__attribute__((naked, noinline))
int open(const char* path, int flags, ...) {
    __asm__ __volatile__(
        "int 0x80;"
        "ret;"
        :
    : "a"(8)                // rax=8 ("open" syscall)
        : "rcx", "r11", "memory" // rdi는 함수 인자로 전달되므로 클로버시 필요 없음
        );
}