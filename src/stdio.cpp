#define va_start(ap, last) __builtin_va_start((ap), (last))
#define va_arg(ap, type)   __builtin_va_arg((ap), type)
#define va_end(ap)         __builtin_va_end((ap))
#include <stdio.h>
#include <stdint.h>
typedef __builtin_va_list va_list;
static void write_syscall(const char* buf, uint64_t len) {
    __asm__ __volatile__(
        "int 0x80"
        :
    : "a"(1),                // rax=1 ("write" syscall)
        "D"(1),                // rdi=1 (stdout)
        "S"(buf),              // rsi=buf
        "d"(len)               // rdx=len
        : "rcx", "r11", "memory"
        );
}

static void putchar(char c) {
    write_syscall(&c, 1);
}

static void print_uint(uint64_t val, int base, bool upper) {
    char buf[32];
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;
    if (val == 0) {
        putchar('0');
        return;
    }
    while (val > 0 && i < (int)sizeof(buf)) {
        buf[i++] = digits[val % base];
        val /= base;
    }
    for (int j = i - 1; j >= 0; --j) putchar(buf[j]);
}

static void print_int(int64_t val) {
    if (val < 0) {
        putchar('-');
        print_uint((uint64_t)(-val), 10, false);
    }
    else {
        print_uint((uint64_t)val, 10, false);
    }
}

static void print_ptr(const void* p) {
    uintptr_t v = (uintptr_t)p;
    putchar('0'); putchar('x');
    print_uint((uint64_t)v, 16, false);
}

/* A small printf supporting: %d %u %x %X %c %s %p %% and width/zero-pad (simple) */
int printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int out_count = 0;

    while (*fmt) {
        if (*fmt != '%') {
            putchar(*fmt++);
            ++out_count;
            continue;
        }
        ++fmt; // skip '%'
        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            ++fmt;
        }

        char spec = *fmt++;
        switch (spec) {
        case 'd': {
            int v = va_arg(ap, int);
            /* naive padding: print into small buffer then pad */
			print_int((int64_t)v);
            break;
        }
        case 'u': {
            unsigned v = va_arg(ap, unsigned);
            /* reuse print_uint but capture output length naive way */
            /* simple implementation: no width support */
            print_uint((uint64_t)v, 10, false);
            break;
        }
        case 'x': {
            unsigned v = va_arg(ap, unsigned);
            print_uint((uint64_t)v, 16, false);
            break;
        }
        case 'X': {
            unsigned v = va_arg(ap, unsigned);
            print_uint((uint64_t)v, 16, true);
            break;
        }
        case 'c': {
            int c = va_arg(ap, int);
            putchar((char)c);
            ++out_count;
            break;
        }
        case 's': {
            const char* s = va_arg(ap, const char*);
            if (!s) s = "(null)";
            while (*s) { putchar(*s++); ++out_count; }
            break;
        }
        case 'p': {
            void* p = va_arg(ap, void*);
            print_ptr(p);
            break;
        }
        case '%': {
            putchar('%');
            ++out_count;
            break;
        }
        default: {
            /* unknown: print literally */
            putchar('%'); putchar(spec);
            out_count += 2;
            break;
        }
        }
    }

    va_end(ap);
    return out_count;
}