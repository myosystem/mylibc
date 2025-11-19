#include <stdlib.h>
#include <stdint.h>
#define PAGE_SIZE 4096
#define MMAP_THRESHOLD (128 * 1024) // 128KiB 이상이면 mmap 사용
static inline uint64_t syscall_brk(uint64_t new_bottom) {
    uint64_t ret;
    __asm__ __volatile__(
        "int 0x80"
        : "=a"(ret)
        : "a"(45ull),         // rax: brk syscall 번호
        "D"(new_bottom)     // rdi: new bottom
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline uint64_t syscall_mmap(uint64_t size, uint64_t flags) {
    uint64_t ret;
    __asm__ __volatile__(
        "int 0x80"
        : "=a"(ret)
        : "a"(9ull),          // rax: mmap syscall 번호
        "D"(size),          // rdi: 요청 크기
        "S"(flags)          // rsi: 플래그
        : "rcx", "r11", "memory"
    );
    return ret; // mmap은 VA를 반환
}

static uint8_t* heap_curr = 0;

void* malloc(size_t size) {
    if (size == 0) return nullptr;

    // 8바이트 정렬
    size = (size + 7) & ~7ULL;

    // 큰 요청은 mmap 사용
    if (size >= MMAP_THRESHOLD) {
        uint64_t addr = syscall_mmap(size, 0);
        if (addr == ~0ULL) return nullptr;
        return (void*)addr;
    }

    // 힙 초기화
    if (!heap_curr) {
        uint64_t curr = syscall_brk(0);
        if (curr == ~0ULL) return nullptr;
        heap_curr = (uint8_t*)curr;
    }

    uint8_t* result = heap_curr;
    uint64_t new_bottom = (uint64_t)(heap_curr + size);

    // 커널에 힙 확장 요청
    uint64_t ret = syscall_brk(new_bottom);
    if (ret == ~0ULL) {
        return nullptr;
    }

    heap_curr = (uint8_t*)new_bottom;
    return result;
}
void free(void* ptr) {
    // 현재 간단한 구현에서는 free가 아무 작업도 하지 않음
    (void)ptr;
}