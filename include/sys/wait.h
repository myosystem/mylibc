#ifndef __SYS_WAIT_H__
#define __SYS_WAIT_H__
#include <stdint.h>

static inline uint64_t wait() {
    uint64_t result;
    __asm__ __volatile__(
        "mov rax, 32\n"
        "int 0x80\n"
        : "=a"(result)
        :
        : "memory"
    );
    return result;
}
#endif /*__SYS_WAIT_H__*/