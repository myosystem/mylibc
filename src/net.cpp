#include <net>

// syscall 37 = netdevice : rdi=서브커맨드, rsi=장치인덱스, rdx=값
// syscall 38 = route     : rdi=서브커맨드, rsi=장치인덱스, r8=dest, r9=netmask, r10=gateway

namespace Net {
namespace Link {

int count() {
    uint64_t ret;
    __asm__ __volatile__(
        "int 0x80"
        : "=a"(ret)
        : "a"(37ull), "D"(0ull)
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}

bool mac(int dev, uint8_t out[6]) {
    uint64_t ret;
    __asm__ __volatile__(
        "int 0x80"
        : "=a"(ret)
        : "a"(37ull), "D"(3ull), "S"((uint64_t)dev), "d"((uint64_t)out)
        : "rcx", "r11", "memory"
    );
    return (int64_t)ret >= 0;
}

} // namespace Link

namespace Addr {

uint32_t get(int dev) {
    uint64_t ret;
    __asm__ __volatile__(
        "int 0x80"
        : "=a"(ret)
        : "a"(37ull), "D"(1ull), "S"((uint64_t)dev)
        : "rcx", "r11", "memory"
    );
    return (uint32_t)ret;
}

bool set(int dev, uint32_t ip) {
    uint64_t ret;
    __asm__ __volatile__(
        "int 0x80"
        : "=a"(ret)
        : "a"(37ull), "D"(2ull), "S"((uint64_t)dev), "d"((uint64_t)ip)
        : "rcx", "r11", "memory"
    );
    return (int64_t)ret >= 0;
}

} // namespace Addr

namespace Route {

bool add(uint32_t dest, uint32_t nm, uint32_t gateway, int dev) {
    uint64_t ret;
    // r8/r9/r10은 제약문자가 없어서 레지스터 변수로 고정한다.
    // 입력으로 쓰므로 클로버 목록에 넣으면 안 된다(GCC가 거부).
    register uint64_t r8v  __asm__("r8")  = dest;
    register uint64_t r9v  __asm__("r9")  = nm;
    register uint64_t r10v __asm__("r10") = gateway;
    __asm__ __volatile__(
        "int 0x80"
        : "=a"(ret)
        : "a"(38ull), "D"(0ull), "S"((uint64_t)dev),
          "r"(r8v), "r"(r9v), "r"(r10v)
        : "rcx", "r11", "memory"
    );
    return (int64_t)ret >= 0;
}

bool del(int dev) {
    uint64_t ret;
    __asm__ __volatile__(
        "int 0x80"
        : "=a"(ret)
        : "a"(38ull), "D"(1ull), "S"((uint64_t)dev)
        : "rcx", "r11", "memory"
    );
    return (int64_t)ret >= 0;
}

} // namespace Route
} // namespace Net
