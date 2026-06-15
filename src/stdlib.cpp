#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#define PAGE_SIZE 4096
#define MMAP_THRESHOLD (128 * 1024) // 128KiB 이상이면 mmap 사용
#define MIN_SLOT_PER_CHUNK 16
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

static inline uint64_t syscall_munmap(void* addr, size_t size) {
    uint64_t ret;
    __asm__ __volatile__(
        "int 0x80"
        : "=a"(ret)
        : "a"(10ull),          // rax: munmap syscall 번호
        "D"(addr),          // rdi: 요청 주소
        "S"(size)          // rsi: 요청 크기
        : "rcx", "r11", "memory"
    );
    return ret; // munmap은 0을 반환
}
static uint8_t* heap_curr = 0;

void* _malloc(size_t size) {
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

// chunk_header 크기: 포인터 2(16) + uint32_t 4(16) + uint64_t 8(64) = 96바이트 → 16배수 OK
struct chunk_header {
    chunk_header* next, * prev;
    uint32_t level;
    uint32_t total_slots;
    uint32_t used_count;
    uint32_t page_count;
    uint64_t bitmap[8];
};

// 슬롯 헤더: 16바이트
// [0..7]  : chunk_header 주소 (또는 ~0ULL for mmap)
// [8..15] : 예약 (추후 활용 가능)
struct slot_header {
    uint64_t chunk_ptr;
    uint64_t reserved;
};

// mmap 대형 블록 헤더: 16바이트
struct MMapHeader {
    uint64_t marker;  // ~0ULL — mmap 식별자
    size_t size;      // 8바이트
};

// level 0: 최대 16바이트, level 1: 32바이트, ..., level N: (1ULL << (level+4))바이트
// 최대 level 9: 8192바이트 > 2048 → 실제로는 level 0~7 정도 사용 (size <= 2048)
chunk_header* chunk_heads[10] = { nullptr, };

static chunk_header* alloc_new_chunk(int level) {
    size_t slot_size = (1ULL << (level + 4)) + sizeof(slot_header); // 슬롯 헤더 16바이트
    int count = 1;
    size_t s = (PAGE_SIZE - sizeof(chunk_header)) / slot_size;

    while (s < MIN_SLOT_PER_CHUNK) {
        count++;
        s = (PAGE_SIZE * count - sizeof(chunk_header)) / slot_size;
    }
    if (s > 512) s = 512;

    chunk_header* head = (chunk_header*)syscall_mmap(PAGE_SIZE * count, 0);
    if (head == (chunk_header*)~0ULL) return nullptr;

    head->level = level;
    head->total_slots = (uint32_t)s;
    head->used_count = 0;
    head->page_count = count;
    head->next = head->prev = nullptr;

    for (int i = 0; i < 8; i++) {
        int start_bit = i * 64;
        if (start_bit >= (int)s)       head->bitmap[i] = ~0ULL;
        else if (start_bit + 64 > (int)s) head->bitmap[i] = ~((1ULL << (s - start_bit)) - 1ULL);
        else                            head->bitmap[i] = 0;
    }
    return head;
}

void* malloc(size_t size) {
    if (size == 0) return nullptr;

    // 1. 대형 블록 처리 (2KiB 초과)
    if (size > 2048) {
        size_t total = size + sizeof(MMapHeader);
        MMapHeader* h = (MMapHeader*)syscall_mmap(total, 0);
        if (h == (MMapHeader*)~0ULL) return nullptr;
        h->size = total;
        h->marker = ~0ULL;
        // 반환 포인터는 MMapHeader 바로 뒤 → 16바이트 정렬 보장
        return (void*)(h + 1);
    }

    // 2. 레벨 결정: size <= (1ULL << (level + 4))
    int level = 0;
    for (; level < 10; level++) if (size <= (1ULL << (level + 4))) break;

    // 3. 기존 청크에서 빈 슬롯 탐색
    chunk_header* head = chunk_heads[level];
    while (head) {
        for (int i = 0; i < 8; i++) {
            if (head->bitmap[i] != ~0ULL) {
                for (int bit = 0; bit < 64; bit++) {
                    if (!(head->bitmap[i] & (1ULL << bit))) {
                        head->bitmap[i] |= (1ULL << bit);
                        head->used_count++;
                        size_t slot_size = (1ULL << (head->level + 4)) + sizeof(slot_header);
                        slot_header* sh = (slot_header*)((uint8_t*)(head + 1) + (i * 64 + bit) * slot_size);
                        sh->chunk_ptr = (uint64_t)head;
                        sh->reserved = 0;
                        return (void*)(sh + 1);
                    }
                }
            }
        }
        head = head->next;
    }

    // 4. 새 청크 할당 및 0번 슬롯 리턴
    chunk_header* new_h = alloc_new_chunk(level);
    if (!new_h) return nullptr;

    new_h->next = chunk_heads[level];
    if (chunk_heads[level]) chunk_heads[level]->prev = new_h;
    chunk_heads[level] = new_h;

    new_h->bitmap[0] |= 1ULL;
    new_h->used_count = 1;

    slot_header* sh = (slot_header*)(new_h + 1);
    sh->chunk_ptr = (uint64_t)new_h;
    sh->reserved = 0;
    return (void*)(sh + 1);
}

void free(void* ptr) {
    if (!ptr) return;

    slot_header* sh = (slot_header*)ptr - 1;

    // mmap 대형 블록 판별
    if (sh->chunk_ptr == ~0ULL) {
        MMapHeader* h = (MMapHeader*)ptr - 1;
        syscall_munmap(h, h->size);
        return;
    }

    chunk_header* head = (chunk_header*)sh->chunk_ptr;
    size_t slot_size = (1ULL << (head->level + 4)) + sizeof(slot_header);
    size_t index = ((uint8_t*)sh - (uint8_t*)(head + 1)) / slot_size;

    head->bitmap[index / 64] &= ~(1ULL << (index % 64));
    head->used_count--;

    // 빈 청크 반납 (리스트에 다른 청크가 있을 때만)
    if (head->used_count == 0 && (head->next || head->prev)) {
        if (head->prev) head->prev->next = head->next;
        if (head->next) head->next->prev = head->prev;
        if (chunk_heads[head->level] == head) chunk_heads[head->level] = head->next;
        syscall_munmap(head, head->page_count * PAGE_SIZE);
    }
}
void* realloc(void* ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) { free(ptr); return nullptr; }

    slot_header* sh = (slot_header*)ptr - 1;
    size_t old_size;
    if (sh->chunk_ptr == ~0ULL) {
        // mmap block: reserved == total (including MMapHeader)
        old_size = sh->reserved - sizeof(MMapHeader);
    } else {
        chunk_header* head = (chunk_header*)sh->chunk_ptr;
        old_size = 1ULL << (head->level + 4);
    }

    if (size <= old_size) return ptr;

    void* newptr = malloc(size);
    if (!newptr) return nullptr;
    memcpy(newptr, ptr, old_size);
    free(ptr);
    return newptr;
}
static std::vector<void(*)()> atexit_handlers;

int atexit(void (*func)(void)) {
    atexit_handlers.push_back(func);
    return 0;
}
struct cxa_atexit_entry {
    void (*func)(void*);
    void* arg;
    void* dso;
};

static std::vector<cxa_atexit_entry> cxa_handlers;

int __cxa_atexit(void (*func)(void*), void* arg, void* dso) {
    cxa_atexit_entry entry = { func, arg, dso };
    cxa_handlers.push_back(entry);
    return 0;
}
void exit(int status) {
    for (int i = cxa_handlers.size() - 1; i >= 0; i--) {
        cxa_handlers[i].func(cxa_handlers[i].arg);
    }
    for (int i = atexit_handlers.size() - 1; i >= 0; i--) {
        atexit_handlers[i]();
    }
    fflush(stdout);
    fflush(stderr);
    _exit(status);
}