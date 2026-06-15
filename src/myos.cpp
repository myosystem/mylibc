#include <myos>
#include <stdlib.h>
#include <stdio.h>
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
extern "C" __attribute__((naked, section(".entry"))) void _start() {
	__asm__ __volatile__(
		"sub rsp, 32\n\t"
		"call _before_main\n\t"
		"call main\n\t"
		"mov rdi, rax\n\t"
		"call _after_main\n\t"
	);
}
uint64_t send_msg(uint64_t dest_pid, const msg_t* msg, bool is_block) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x04), "D"(0), "S"((uint64_t)msg), "d"(dest_pid), "c"((uint64_t)is_block)
		: "r11", "memory"
	);
	return ret; // return value in rax
}
uint64_t receive_msg(msg_t* msg) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x04), "D"(1), "S"((uint64_t)msg)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax (메시지 수신 성공 여부)
}
uint64_t get_tsc() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(0)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax
}
uint64_t get_memory_size() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(2)
		: "rcx", "r11", "memory"
	);
	return ret * 4096; // return value in rax (B 단위)
}
uint64_t get_used_memory_size() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(3)
		: "rcx", "r11", "memory"
	);
	return ret * 4096; // return value in rax (B 단위)
}
uint64_t get_free_memory_size() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(4)
		: "rcx", "r11", "memory"
	);
	return ret * 4096; // return value in rax (B 단위)
}
uint64_t get_process_count() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(5)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax (B 단위)
}
uint64_t get_max_process_count() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(6)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax (B 단위)
}
void shutdown() {
	__asm__ __volatile__(
		"int 0x80"
		:
	: "a"(-1ull)
		: "rcx", "r11", "memory"
		);
}
void wait_for_msg() {
	__asm__ __volatile__(
		"int 0x81"
		:
		: "a"(0x04)
		: "rcx", "r11", "memory"
	);
}
void sleep(uint64_t ms) {
	__asm__ __volatile__(
		"int 0x81"
		:
	: "a"(32), "D"(ms)
		:"memory"
	);
}
uint64_t set_timer(uint64_t timeout_ms, uint64_t interval_ms) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x06), "D"(0x35), "S"(timeout_ms), "d"(interval_ms)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax (타이머 ID)
}
void cancel_timer(uint64_t timer_id) {
	__asm__ __volatile__(
		"int 0x80"
		:
	: "a"(0x06), "D"(0x0), "S"(timer_id)
		:"memory"
	);
}

SharedMem::SharedMem() : id(0), size(0) {}

void* SharedMem::create(uint64_t size) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(15), "D"(size), "S"(this)
		: "rcx", "r11", "memory"
	);
	if (ret == 0) return nullptr; // 실패
	return (void*)(uintptr_t)ret; // 반환값은 공유 메모리의 가상 주소
}

void* SharedMem::accept(uint64_t id) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(16), "D"(id), "S"(this)
		: "rcx", "r11", "memory"
	);
	if (ret == 0) return nullptr; // 실패
	return (void*)(uintptr_t)ret; // 반환값은 공유 메모리의 가상 주소
}
uint64_t get_ginfo(Ginfo* ginfo) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x05), "D"(0), "S"((uint64_t)ginfo)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax
}
uint64_t display_frame(void* frame_buffer) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x05), "D"(2), "S"((uint64_t)frame_buffer)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax
}
uint64_t SharedMem::get_id() const {
	return id;
}
uint64_t SharedMem::get_size() const {
	return size;
}
once_flag Window::ginfo_once_flag;
Ginfo Window::ginfo;
uint64_t Window::bytesPerPixel;
Window::Window(RECT rect, uint32_t style, uint32_t ex_style) : rect(rect) {
	call_once(ginfo_once_flag, [this]() {
		get_ginfo(&ginfo);
		switch (ginfo.format) { // Updated to use ginfo.format instead of ModeInfo->PixelFormat
		case GOP_PIXEL_FORMAT_RGBR:
		case GOP_PIXEL_FORMAT_BGRR:
			bytesPerPixel = 4;
			break;

		case GOP_PIXEL_FORMAT_BITMASK: {
			uint32_t mask = (uint32_t)ginfo.format;
			mask = (uint32_t)(ginfo.width | ginfo.height);
			uint32_t highest = 31;
			while (highest && ((mask >> highest) & 1) == 0)
				highest--;
			bytesPerPixel = ((highest + 1) + 7) / 8;
			break;
		}
		case GOP_PIXEL_FORMAT_BLT_ONLY:
		default:
			bytesPerPixel = 0;
			break;
		}
		});
	gbuf = gshm.create(rect.width * rect.height * bytesPerPixel);
	msg_t msg{
	.sender_pid = 1,
	.type = MSG_MAKE_WINDOW,
	.status = 0,
	.payload{ {gshm.get_id(),pack_u32(rect.x,rect.y), pack_u32(rect.width, rect.height), pack_u32(ex_style, style),0}},
	.timestamp = 0
	};
	uint64_t result = send_msg(0, &msg);
	printf("Code %d: Sent MSG_MAKE_WINDOW with shared memory id %d, result=%d\n", (int)msg.type, (int)gshm.get_id(), (int)result);
	wait_for_msg();
	msg_t response;
	receive_msg(&response);

}
void Window::draw_frame(RECT rect) {
	if (gbuf == nullptr) return;
	msg_t msg{
		.sender_pid = 1,
		.type = MSG_DRAW_FRAME,
		.status = 0,
		.payload{ {gshm.get_id(),pack_u32(rect.x,rect.y), pack_u32(rect.width, rect.height)}},
		.timestamp = 0

	};
	uint64_t result = send_msg(0, &msg);
}
FILE* stdout;
FILE* stdin;
FILE* stderr;
extern "C" void _before_main() {
	stdout = (FILE*)malloc(sizeof(FILE));
	stdin = (FILE*)malloc(sizeof(FILE));
	stderr = (FILE*)malloc(sizeof(FILE));
	stdout->fd = 1; // 표준 출력
	stdout->buffer = (uint8_t*)malloc(1024); // 버퍼 할당
	stdout->bufsize = 1024;
	stdout->cursor = 0;
	stdout->flags = _IOLBF;
	stdout->dirty = 0;
	stdout->ungetc_buf = 0;
	stdout->unget_count = 0;
	stdout->limit = 1024;

	stdin->fd = 0; // 표준 입력
	stdin->buffer = (uint8_t*)malloc(1024); // 버퍼 할당
	stdin->bufsize = 1024;
	stdin->cursor = 0;
	stdin->flags = _IOFBF; // 버퍼링 없음
	stdin->dirty = 0;
	stdin->ungetc_buf = 0;
	stdin->unget_count = 0;
	stdin->limit = 0; // 아직 읽을게 없음

	stderr->fd = 2; // 표준 오류
	stderr->buffer = nullptr; // 버퍼 할당
	stderr->bufsize = 0;
	stderr->cursor = 0;
	stderr->flags = _IONBF; // 라인 버퍼링
	stderr->dirty = 0;
	stderr->ungetc_buf = 0;
	stderr->unget_count = 0;
	stderr->limit = 0;
}
extern "C" void _after_main(int exit_code) {
	fclose(stdout);
	fclose(stdin);
	fclose(stderr);
	exit(exit_code);
}
extern "C" void* __dso_handle = nullptr;