#define va_start(ap, last) __builtin_va_start((ap), (last))
#define va_arg(ap, type)   __builtin_va_arg((ap), type)
#define va_end(ap)         __builtin_va_end((ap))
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

struct _FILE {
	uint8_t* buffer;
	uint64_t bufsize;
	uint64_t cursor;
    uint64_t fd;
	uint64_t flags;
    uint64_t limit;
	uint32_t ungetc_buf;
	uint32_t unget_count;
    uint8_t dirty;
};
int fflush(FILE* stream) {
    if (!stream->dirty || stream->cursor == 0) {
        return 0;
    }
    if (stream->fd == (uint64_t)-2) {
        return 0; // string sink: nothing to flush to fd
    }
    int64_t written = write(stream->fd, (const char*)stream->buffer, stream->cursor);

    if (written < 0) {
        return -1;
    }

    stream->cursor = 0;
    stream->dirty = 0;
    return 0;
}
int fputc(int c, FILE* stream) {
    if (stream->cursor >= stream->bufsize) {
        if (stream->fd == (uint64_t)-2) return -1; // string buffer full
        if (fflush(stream) != 0) {
            return -1;
        }
    }
    stream->buffer[stream->cursor++] = (uint8_t)c;
    stream->dirty = 1;
    if ((stream->flags & _IOLBF) && (char)c == '\n') {
        fflush(stream);
    }

    return (unsigned char)c;
}
static void fprint_uint(uint64_t val, int base, bool upper, FILE* stream) {
    char buf[32];
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;
    if (val == 0) {
        fputc('0', stream);
        return;
    }
    while (val > 0 && i < (int)sizeof(buf)) {
        buf[i++] = digits[val % base];
        val /= base;
    }
    for (int j = i - 1; j >= 0; --j) fputc(buf[j], stream);
}
static void fprint_int(int64_t val, FILE* stream) {
    if (val < 0) {
        fputc('-', stream);
        fprint_uint((uint64_t)(-val), 10, false, stream);
    }
    else {
        fprint_uint((uint64_t)val, 10, false, stream);
    }
}
static void fprint_ptr(const void* p, FILE* stream) {
    uintptr_t v = (uintptr_t)p;
    fputc('0', stream); fputc('x', stream);
    fprint_uint((uint64_t)v, 16, false, stream);
}

/* A small printf supporting: %d %u %x %X %c %s %p %% and width/zero-pad (simple) */
int printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
	int result = vfprintf(stdout, fmt, ap);
    va_end(ap);
    return result;
}
int vprintf(const char* fmt, va_list ap) {
    return vfprintf(stdout, fmt, ap);
}
int fprintf(FILE* stream, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int result = vfprintf(stream, fmt, ap);
    va_end(ap);
    return result;
}
int vfprintf(FILE* stream, const char* fmt, va_list ap) {
    int out_count = 0;

    while (*fmt) {
        if (*fmt != '%') {
            fputc(*fmt++, stream);
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
            fprint_int((int64_t)v, stream);
            break;
        }
        case 'u': {
            unsigned v = va_arg(ap, unsigned);
            /* reuse fprint_uint but capture output length naive way */
            /* simple implementation: no width support */
            fprint_uint((uint64_t)v, 10, false, stream);
            break;
        }
        case 'l': {
			if (*fmt == 'd') {
				int64_t v = va_arg(ap, int64_t);
				fprint_int(v, stream);
				++fmt;
			}
			else if (*fmt == 'u') {
				uint64_t v = va_arg(ap, uint64_t);
				fprint_uint(v, 10, false, stream);
				++fmt;
			}
			else if (*fmt == 'x') {
				uint64_t v = va_arg(ap, uint64_t);
				fprint_uint(v, 16, false, stream);
				++fmt;
			}
			else if (*fmt == 'X') {
				uint64_t v = va_arg(ap, uint64_t);
				fprint_uint(v, 16, true, stream);
				++fmt;
			}
			else {
				/* unknown: print literally */
				fputc('%', stream); fputc('l', stream); fputc(*fmt, stream);
				out_count += 3;
			}
			break;
        }
        case 'x': {
            unsigned v = va_arg(ap, unsigned);
            fprint_uint((uint64_t)v, 16, false, stream);
            break;
        }
        case 'X': {
            unsigned v = va_arg(ap, unsigned);
            fprint_uint((uint64_t)v, 16, true, stream);
            break;
        }
        case 'c': {
            int c = va_arg(ap, int);
            fputc((char)c, stream);
            ++out_count;
            break;
        }
        case 's': {
            const char* s = va_arg(ap, const char*);
            if (!s) s = "(null)";
            while (*s) { fputc(*s++, stream); ++out_count; }
            break;
        }
        case 'p': {
            void* p = va_arg(ap, void*);
            fprint_ptr(p, stream);
            break;
        }
        case '%': {
            fputc('%', stream);
            ++out_count;
            break;
        }
        default: {
            /* unknown: print literally */
            fputc('%', stream); fputc(spec, stream);
            out_count += 2;
            break;
        }
        }
    }
    return out_count;
}
FILE* fopen(const char* pathname, const char* mode) {
    FILE* f = (FILE*)malloc(sizeof(FILE));
    if (!f) return nullptr;
    f->buffer = (uint8_t*)malloc(1024); // simple fixed buffer size
    if (!f->buffer) {
        free(f);
        return nullptr;
    }
    f->bufsize = 1024;
    f->cursor = 0;
    f->dirty = 0;
    f->fd = open(pathname,0);
    if (f->fd < 0) {
        free(f->buffer);
        free(f);
        return nullptr;
    }
    f->flags = 0; // simple implementation: ignore mode parsing
	return f;
}
int fclose(FILE* stream) {
    if (!stream) return -1;
    fflush(stream);
    close(stream->fd);
    if (stream->buffer)
        free(stream->buffer);
    free(stream);
    return 0;
}
int ungetc(int c, FILE* stream) {
    if (stream->unget_count < 4) {
		stream->ungetc_buf = (stream->ungetc_buf << 8) | (c & 0xFF);
        stream->unget_count++;
		return c;
    }
    else {
        return -1;
    }
}
int fgetc(FILE* stream) {
    if (stream->unget_count > 0) {
        stream->unget_count--;
		char c = (char)stream->ungetc_buf;
		stream->ungetc_buf >>= 8;
        return c;
    }

    if (stream->cursor >= stream->limit) {
        int64_t n = read(stream->fd, (char*)stream->buffer, stream->bufsize);
        if (n <= 0) return -1;

        stream->limit = n;
        stream->cursor = 0;
    }
    return stream->buffer[stream->cursor++];
}
static uint64_t fscan_uint(int base, bool upper, FILE* stream) {
    uint64_t result = 0;
    while (true) {
        int c = fgetc(stream);
        if (c == -1) break;
        if (c >= '0' && c <= '9') {
            result = result * base + (c - '0');
        }
        else if (upper && c >= 'A' && c < 'A' + base - 10) {
            result = result * base + (c - 'A' + 10);
        }
        else if (!upper && c >= 'a' && c < 'a' + base - 10) {
            result = result * base + (c - 'a' + 10);
        }
        else {
			ungetc(c, stream);
            break;
        }
	}
    return result;
}
static int64_t fscan_int(FILE* stream) {
    int64_t result = 0;
    bool negative = false;
    int c = fgetc(stream);
    if (c == '-') {
        negative = true;
		result = fscan_uint(10, false, stream);
    }
    else if (c >= '0' && c <= '9') {
		ungetc(c, stream);
		result = fscan_uint(10, false, stream);
    }
	if (negative) result = -result;
	return result;
}
int scanf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int result = vfscanf(stdin, fmt, ap);
    va_end(ap);
    return result;
}
int fscanf(FILE* stream, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int result = vfscanf(stream, fmt, ap);
    va_end(ap);
    return result;
}
int vfscanf(FILE* stream, const char* fmt, va_list ap) {
    int out_count = 0;

    while (*fmt) {
        if (*fmt != '%') {
            fgetc(stream);
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
            int* v = va_arg(ap, int*);
            /* naive padding: print into small buffer then pad */
			*v = (int)fscan_int(stream);
			out_count++;
            break;
        }
        case 'u': {
            unsigned* v = va_arg(ap, unsigned*);
            /* reuse fprint_uint but capture output length naive way */
            /* simple implementation: no width support */
            *v = (unsigned)fscan_uint(10, false, stream);
            out_count++;
            break;
        }
        case 'x': {
            unsigned* v = va_arg(ap, unsigned*);
            *v = (unsigned)fscan_uint(16, false, stream);
            out_count++;
            break;
        }
        case 'X': {
            unsigned* v = va_arg(ap, unsigned*);
            *v = (unsigned)fscan_uint(16, true, stream);
            out_count++;
            break;
        }
        case 'c': {
            int* v = va_arg(ap, int*);
            *v = fgetc(stream);
            out_count++;
            break;
        }
        case 's': {
            char* s = va_arg(ap, char*);
            if (!s) continue;
            int c;
            while ((c = fgetc(stream)) != -1 && (c == ' ' || c == '\t' || c == '\n' || c == '\r'));
            if (c == -1) break;
            while (c != -1 && c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '\0') {
                *s++ = (char)c;
                c = fgetc(stream);
            }
            if (c != -1) ungetc(c, stream);
            *s = '\0';
            out_count++;
            break;
        }
        case '%': {
			continue; // just skip '%' in input
        }
        default: {
            continue;
        }
        }
    }
    return out_count;
}
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t total_bytes = size * nmemb;
    size_t bytes_to_read = total_bytes;
    uint8_t* out = (uint8_t*)ptr;

    // 1. ungetc 버퍼에 뭐가 있다면 그것부터 처리 (fgetc 호출로 간단히 해결 가능)
    while (stream->unget_count > 0 && bytes_to_read > 0) {
        *out++ = (uint8_t)fgetc(stream);
        bytes_to_read--;
    }

    while (bytes_to_read > 0) {
        // 2. 현재 버퍼에 데이터가 남아있는지 확인
        size_t available = stream->limit - stream->cursor;

        if (available > 0) {
            size_t take = (available < bytes_to_read) ? available : bytes_to_read;
            memcpy(out, &stream->buffer[stream->cursor], take);
            stream->cursor += take;
            out += take;
            bytes_to_read -= take;
        }
        else {
            // 3. 버퍼가 비었을 때
            if (bytes_to_read >= stream->bufsize) {
                // 남은 양이 버퍼 크기보다 크면 버퍼 거치지 말고 직접 유저 버퍼로 꽂아버림 (최적화)
                size_t direct_read_blocks = bytes_to_read / stream->bufsize;
                int64_t n = read(stream->fd, (char*)out, direct_read_blocks * stream->bufsize);
                if (n <= 0) break;

                out += n;
                bytes_to_read -= n;
                // 만약 n이 요청보다 적으면(EOF) 루프 탈출
                if (n < (int64_t)(direct_read_blocks * stream->bufsize)) break;
            }
            else {
                // 남은 양이 버퍼보다 작으면 다시 버퍼를 채움
                int64_t n = read(stream->fd, (char*)stream->buffer, stream->bufsize);
                if (n <= 0) break;

                stream->limit = n;
                stream->cursor = 0;
                // 다음 루프에서 available > 0 조건에 의해 처리됨
            }
        }
    }

    return (total_bytes - bytes_to_read) / size;
}
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
    const uint8_t* src = (const uint8_t*)ptr;
    size_t total = size * nmemb;
    for (size_t i = 0; i < total; i++) {
        if (fputc((int)src[i], stream) < 0) return i / size;
    }
    return nmemb;
}
int fputs(const char* s, FILE* stream) {
    while (*s) {
        if (fputc((unsigned char)*s++, stream) < 0) return -1;
    }
    return 0;
}
char* fgets(char* s, int n, FILE* stream) {
    if (n <= 0) return nullptr;
    int i = 0;
    while (i < n - 1) {
        int c = fgetc(stream);
        if (c < 0) break;
        s[i++] = (char)c;
        if (c == '\n') break;
    }
    if (i == 0) return nullptr;
    s[i] = '\0';
    return s;
}
int fseek(FILE* stream, long offset, int whence) {
    fflush(stream);
    stream->cursor = 0;
    stream->limit = 0;
    stream->unget_count = 0;
    return lseek(stream->fd, (off_t)offset, whence) < 0 ? -1 : 0;
}
long ftell(FILE* stream) {
    if (stream->dirty) fflush(stream);
    off_t pos = lseek(stream->fd, 0, SEEK_CUR);
    if (pos < 0) return -1L;
    // subtract unconsumed bytes in the read-ahead buffer
    pos -= (off_t)(stream->limit - stream->cursor);
    pos -= (off_t)stream->unget_count;
    return (long)pos;
}
void rewind(FILE* stream) {
    fseek(stream, 0L, SEEK_SET);
}
int vsnprintf(char* buf, size_t n, const char* fmt, va_list ap) {
    if (n == 0) return 0;
    FILE sf;
    sf.buffer = (uint8_t*)buf;
    sf.bufsize = n - 1; // reserve 1 byte for null terminator
    sf.cursor = 0;
    sf.fd = (uint64_t)-2; // string sink sentinel
    sf.dirty = 0;
    sf.limit = 0;
    sf.flags = 0;
    sf.unget_count = 0;
    sf.ungetc_buf = 0;
    int ret = vfprintf(&sf, fmt, ap);
    buf[sf.cursor] = '\0';
    return ret;
}
int snprintf(char* buf, size_t n, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vsnprintf(buf, n, fmt, ap);
    va_end(ap);
    return ret;
}
int vsprintf(char* buf, const char* fmt, va_list ap) {
    return vsnprintf(buf, (size_t)-1, fmt, ap);
}
int sprintf(char* buf, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vsprintf(buf, fmt, ap);
    va_end(ap);
    return ret;
}