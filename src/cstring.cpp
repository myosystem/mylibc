#include <string.h>
#include <stdlib.h>
#include <stdint.h>
void* memcpy(void* dest, const void* src, size_t size) {
    // [Intel Syntax]
    // 1. RCX = size / 8 (QWORD 개수)
    // 2. rep movsq (8바이트씩 고속 복사)
    // 3. RCX = size % 8 (나머지 바이트)
    // 4. rep movsb (나머지 처리)
    __asm__ __volatile__(
        "mov rcx, rdx      \n\t"  // rdx(size)를 rcx에 복사
        "shr rcx, 3        \n\t"  // size / 8 (Shift Right 3)
        "rep movsq         \n\t"  // 8바이트 단위 복사
        "and rdx, 7        \n\t"  // size % 8 (나머지 구하기)
        "mov rcx, rdx      \n\t"  // 나머지를 rcx에 넣음
        "rep movsb         \n\t"  // 나머지 1바이트 단위 복사
        :
    : "D"(dest), "S"(src), "d"(size) // RDI=dest, RSI=src, RDX=size
        : "rcx", "memory" // RCX 파괴됨, 메모리 변경됨
        );
    return dest;
}

void* memset(void* dest, int value, size_t size) {
    // [Intel Syntax]
    // 1. AL(value)를 RAX 전체에 복사 (0xAB -> 0xABAB...ABAB)
    // 2. 8바이트씩 쓰기 (rep stosq)
    // 3. 나머지 바이트 쓰기 (rep stosb)
    __asm__ __volatile__(
        "movzx eax, sil    \n\t"  // value(esi의 하위 8비트)를 eax에 로드
        "mov ah, al        \n\t"  // ax = 0xABAB
        "mov rcx, rax      \n\t"  // rcx = 0x000000000000ABAB
        "shl rcx, 16       \n\t"  // rcx = 0x00000000ABAB0000
        "or  rax, rcx      \n\t"  // rax = 0x00000000ABABABAB
        "mov rcx, rax      \n\t"
        "shl rcx, 32       \n\t"
        "or  rax, rcx      \n\t"  // rax = 0xABABABABABABABAB (완성!)

        "mov rcx, rdx      \n\t"  // size를 rcx로
        "shr rcx, 3        \n\t"  // size / 8
        "rep stosq         \n\t"  // 8바이트씩 한 번에 씀 (메모리 대역폭 풀가동)

        "and rdx, 7        \n\t"  // 나머지
        "mov rcx, rdx      \n\t"
        "rep stosb         \n\t"  // 찌꺼기 처리
        :
    : "D"(dest), "S"(value), "d"(size) // RDI=dest, RSI=value(int지만 sil씀), RDX=size
        : "rax", "rcx", "memory"
        );
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    int result;
    // [Intel Syntax - Optimized by User]
    // 1. SUB AL, CL을 먼저 해버림 (결과 저장)
    // 2. 0이 아니면(다르면) 바로 루프 탈출 -> AL에 이미 정답 들어있음!
    // 3. 0이면(같으면) CL이 NULL인지 확인 -> 둘 다 끝났으면 종료
    __asm__ __volatile__(
        "xor rax, rax       \n\t" // 반환값 32비트 확장을 위해 상위 비트 클리어
        "xor rcx, rcx       \n\t"

        "1:                     \n\t"
        "mov al, [rdi]      \n\t" // s1 로드
        "mov cl, [rsi]      \n\t" // s2 로드

        "sub al, cl         \n\t" // [핵심] 그냥 빼버림! (CMP 안 씀)
        "jnz 2f             \n\t" // 0이 아니면(다르면) 즉시 종료 (AL에 답 있음)

        "test cl, cl        \n\t" // 둘 다 같아서 AL=0인 상태. 근데 CL이 0(NULL)인가?
        "jz 2f              \n\t" // 그렇다면 문자열 끝. 종료. (AL=0인 상태로 리턴)

        "inc rdi            \n\t" // 포인터 증가
        "inc rsi            \n\t"
        "jmp 1b             \n\t" // 루프

        "2:                     \n\t"
        "movsx eax, al      \n\t" // AL(8bit)을 int(32bit)로 부호 확장
        : "=a"(result)
        : "D"(s1), "S"(s2)
        : "rcx", "memory"
    );
    return result;
}
int strncmp(const char* s1, const char* s2, size_t n) {
    int result;
    __asm__ __volatile__(
        "xor rax, rax       \n\t" // 반환값 32비트 확장을 위해 상위 비트 클리어
        "xor rcx, rcx       \n\t"
        "1:                     \n\t"
        "test rdx, rdx      \n\t" // n이 0인가?
        "jz 2f              \n\t" // 0이면 종료 (AL=0인 상태로 리턴)
        "mov al, [rdi]      \n\t" // s1 로드
        "mov cl, [rsi]      \n\t" // s2 로드

        "sub al, cl         \n\t" // [핵심] 그냥 빼버림! (CMP 안 씀)
        "jnz 2f             \n\t" // 0이 아니면(다르면) 즉시 종료 (AL에 답 있음)
        "test cl, cl        \n\t" // 둘 다 같아서 AL=0인 상태. 근데 CL이 0(NULL)인가?
        "jz 2f              \n\t" // 그렇다면 문자열 끝. 종료. (AL=0인 상태로 리턴)
        "inc rdi            \n\t" // 포인터 증가
        "inc rsi            \n\t"
        "dec rdx            \n\t" // n 감소
        "jmp 1b             \n\t" // 루프
        "2:                     \n\t"
        "movsx eax, al      \n\t" // AL(8bit)을 int(32bit)로 부호 확장
        : "=a"(result)
        : "D"(s1), "S"(s2), "d"(n)
        : "rcx", "memory"
    );
    return result;
}
char* strncpy(char* dest, const char* src, size_t n) {
    char* d = dest;
    const char* s = src;
    while (n-- && (*d++ = *s++)) {
        // 복사하면서 NULL 문자 만나면 종료
    }
    // 남은 공간을 NULL로 채움
    while (n--) {
        *d++ = '\0';
    }
    return dest;
}
size_t strlen(const char* s) {
    const char* p = s;
    while (*p) ++p;
    return (size_t)(p - s);
}
char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}
char* strcat(char* dest, const char* src) {
    char* d = dest;
    while (*d) ++d;
    while ((*d++ = *src++));
    return dest;
}
char* strncat(char* dest, const char* src, size_t n) {
    char* d = dest;
    while (*d) ++d;
    while (n-- && *src) *d++ = *src++;
    *d = '\0';
    return dest;
}
char* strchr(const char* s, int c) {
    while (*s) {
        if ((unsigned char)*s == (unsigned char)c) return (char*)s;
        ++s;
    }
    if (c == '\0') return (char*)s;
    return nullptr;
}
char* strrchr(const char* s, int c) {
    const char* last = nullptr;
    do {
        if ((unsigned char)*s == (unsigned char)c) last = s;
    } while (*s++);
    return (char*)last;
}
char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; ++haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) { ++h; ++n; }
        if (!*n) return (char*)haystack;
    }
    return nullptr;
}
void* memmove(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    if (d == s || n == 0) return dest;
    if (d < s || d >= s + n) {
        // no overlap or dest is before src: forward copy
        return memcpy(dest, src, n);
    }
    // overlap: copy backwards
    d += n; s += n;
    while (n--) *--d = *--s;
    return dest;
}
int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* a = (const uint8_t*)s1;
    const uint8_t* b = (const uint8_t*)s2;
    while (n--) {
        if (*a != *b) return (int)*a - (int)*b;
        ++a; ++b;
    }
    return 0;
}
char* strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char* p = (char*)malloc(len);
    if (p) memcpy(p, s, len);
    return p;
}