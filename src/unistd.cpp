#include <unistd.h>

__attribute__((naked, noinline))
ssize_t read(int fd, void* buf, size_t count) {
    __asm__ __volatile__(
        "int 0x80;"
        "ret;"
        :
    : "a"(2)                // rax=2 ("read" syscall)
        : "rcx", "r11", "memory" // rdi rsi rdx는 함수 인자로 전달되므로 클로버시 필요 없음
        );
}
__attribute__((naked, noinline))
ssize_t write(int fd, const void* buf, size_t count) {
    __asm__ __volatile__(
        "int 0x80;"
        "ret;"
        :
    : "a"(1)                // rax=1 ("write" syscall)
        : "rcx", "r11", "memory" // rdi rsi rdx는 함수 인자로 전달되므로 클로버시 필요 없음
        );
}
int close(int fd) {
    return 0;
}
__attribute__((naked, noinline))
pid_t fork(void) {
    __asm__ __volatile__(
        "int 0x80;"
        "ret;"
        :
    : "a"(30)
        : "rcx", "r11", "memory"
        );
}

__attribute__((naked, noinline))
void _exit(int status) {
    __asm__ __volatile__(
        "int 0x80;"
        "ret;"
        :
    : "a"(50)
        : "rcx", "r11", "memory"
        );
}