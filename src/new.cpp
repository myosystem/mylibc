#include <new>
#include <stdlib.h>
static std::new_handler __new_handler = nullptr;
void* operator new(size_t size) {
	if (size == 0)
		size = 1;
	for (;;) {
		void* result = malloc(size);
		if (result != nullptr) return result;
		if (__new_handler) 
			__new_handler();
		else
			//throw std::bad_alloc();
			return nullptr;

	}
}
void operator delete(void* ptr) noexcept {
	free(ptr);
}
void* operator new[](size_t size) {
	return operator new(size);
}
void operator delete[](void* ptr) noexcept {
	operator delete(ptr);
}
void operator delete(void* ptr, size_t) noexcept {
	operator delete(ptr);
}
std::new_handler std::set_new_handler(std::new_handler handler) noexcept {
	std::new_handler old_handler = ::__new_handler;
	::__new_handler = handler;
	return old_handler;
}
std::new_handler std::get_new_handler() noexcept {
	return ::__new_handler;
}