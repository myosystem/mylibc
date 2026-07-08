#include <pipe>
Pipe::Pipe() {
	int fds[2];
    __asm__ __volatile__(
        "int 0x80;"
        :
        : "a"(33), "D"(fds)
        : "rcx", "r11", "memory"
    );
    read_fd = fds[0];
    write_fd = fds[1];
}
Pipe::~Pipe() {
    close_write();
    close_read();
}
void Pipe::close_read() {
    close(read_fd);
}
void Pipe::close_write() {
    close(write_fd);
}
int Pipe::read(void* buf, uint32_t len) {
    return ::read(read_fd, buf, len);
}
int Pipe::write(const void* buf, uint32_t len) {

}