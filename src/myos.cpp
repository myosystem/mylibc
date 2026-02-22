#include <myos>
#include <stdlib.h>

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

uint64_t SharedMem::get_id() const {
	return id;
}
uint64_t SharedMem::get_size() const {
	return size;
}