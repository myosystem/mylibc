#include <sched.h>

int sched_yield(void) {
    __asm__ __volatile__(
        "mov rax, 34\n\t"
        "int 0x80\n\t"
        ::: "rax", "rcx", "r11", "memory"
    );
    return 0;
}