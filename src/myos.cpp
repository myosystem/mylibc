#include <myos>
#include <stdlib.h>
extern "C" __attribute__((naked, section(".entry"))) void _start() {
	__asm__ __volatile__(
		"sub rsp, 32\n\t"
		"call _before_main\n\t"
		"call main\n\t"
	);
}
uint64_t send_msg(uint64_t dest_pid, const msg_t* msg) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x04), "D"(0), "S"((uint64_t)msg), "d"(dest_pid)
		: "rcx", "r11", "memory"
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

void* operator new(size_t size) {
	// 단순히 malloc으로 구현 (실제 구현에서는 더 복잡한 메모리 관리 필요)
	return malloc(size);
}
void operator delete(void* ptr) {
	// 단순히 free로 구현 (실제 구현에서는 더 복잡한 메모리 관리 필요)
	free(ptr);
}
void* operator new[](size_t size) {
	return malloc(size);
}
void operator delete[](void* ptr) {
	free(ptr);
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
Window::Window(uint32_t width, uint32_t height) : Window({0,0,width,height}) {
}
Window::Window(RECT rect) : rect(rect) {
	call_once(ginfo_once_flag, [this]() {
		get_ginfo(&ginfo);
		});
	uint64_t bytesPerPixel;

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
	gbuf_addr = gbuf.create(rect.width * rect.height * bytesPerPixel);
	msg_t msg{
	.sender_pid = 1,
	.type = MSG_MAKE_WINDOW,
	.status = 0,
	.payload{ {gbuf.get_id(),pack_u32(rect.x,rect.y), pack_u32(rect.width, rect.height)}},
	.timestamp = 0
	};
	uint64_t result = send_msg(0, &msg);
}
extern "C" void _before_main() {

}