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
__attribute__((noinline))
int dup2(int oldfd, int newfd) {
    int result;
    __asm__ __volatile__(
        "int 0x80;"
        : "=a"(result)
        : "a"(8), "D"(2), "S"(oldfd), "d"(newfd)                // rax=8 ("dup2" syscall)
        : "rcx", "r11", "memory" // rdi는 함수 인자로 전달되므로 클로버시 필요 없음
    );
    return result;
}
int dup3(int oldfd, int newfd, int flags) {
    if (oldfd == newfd) return -1;
    return dup2(oldfd, newfd); // 플래그구현 아직 없음
}
__attribute__((noinline))
int close(int fd) {
    int result;
    __asm__ __volatile__(
        "int 0x80;"
        : "=a"(result)
        : "a"(8), "D"(1), "S"(fd)                // rax=8 ("close" syscall)
        : "rcx", "r11", "memory" // rdi는 함수 인자로 전달되므로 클로버시 필요 없음
    );
    return result;
}
__attribute__((naked, noinline))
off_t lseek(int fd, off_t offset, int whence) {
    __asm__ __volatile__(
        "int 0x80;"
        "ret;"
        :
        : "a"(62)
        : "rcx", "r11", "memory"
    );
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
__attribute__((naked, noinline))
int execv(const char* path, char* const argv[]) {
    __asm__ __volatile__(
        "int 0x80;"
        "ret;"
        :
        : "a"(31)
        : "rcx", "r11", "memory"
    );
}
__attribute__((naked, noinline))
int chdir(const char* path) {
    __asm__ __volatile__(
        "int 0x80;"
        "ret;"
        :
        : "a"(35)
        : "rcx", "r11", "memory"
    );
}