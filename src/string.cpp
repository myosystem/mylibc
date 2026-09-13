#include <string>
#include <string.h>
#include <stdlib.h>
#include <new>
#include <bits>
using namespace std;

string::string() {
	data = new char[1];
	data[0] = '\0';
	length = 0;
	cap = 1;
}

string::string(const char* s) {
	length = 0;
	while (s[length]) length++;
	cap = bit_ceil(length + 1);
	data = new char[cap];
	memcpy(data, s, length);
	data[length] = '\0';
}

string::string(const string& other) : length(other.length), cap(other.cap) {
	data = new char[cap];
	memcpy(data, other.data, length + 1);
}

string::string(string&& other) noexcept : data(other.data), length(other.length), cap(other.cap) {
	other.data = new char[1];
	other.data[0] = '\0';
	other.length = 0;
	other.cap = 1;
}

string::~string() {
	if (data) {
		delete[] data;
		data = nullptr;
	}
	length = 0;
	cap = 0;
}
string& string::operator=(const string& other) {
	if (this == &other) return *this;
	if (other.length >= cap) {
		char* new_data = new char[other.cap];
		memcpy(new_data, other.data, other.length + 1);
		delete[] data;
		data = new_data;
		cap = other.cap;
	}
	else {
		memcpy(data, other.data, other.length + 1);
	}
	length = other.length;
	return *this;
}
string& string::operator=(string&& other) noexcept {
	if (this == &other) return *this;
	if (data) delete[] data;
	data = other.data;
	length = other.length;
	cap = other.cap;
	other.data = nullptr;
	other.length = 0;
	other.cap = 0;
	return *this;
}
const char* string::c_str() const {
	return data;
}
size_t string::size() const {
	return length;
}
void string::shrink_to_fit() {
	auto new_cap = bit_ceil(length + 1);
	if (new_cap == cap) return;
	cap = new_cap;
	char* new_data = new char[cap];
	memcpy(new_data, data, length);
	delete[] data;
	data = new_data;
	data[length] = '\0';
}
void string::resize(size_t n, char ch) {
	if (n == length) return;
	size_t old_length = length;
	length = n;
	if (length >= cap) {
		shrink_to_fit();          // 여기선 늘리는 역할 (cap = bit_ceil(length+1))
	}
	if (n > old_length) {
		memset(data + old_length, ch, n - old_length);
	}
	data[length] = '\0';
}
void string::push_back(char ch) {
	length++;
	if (length >= cap) {
		shrink_to_fit();
	}
	data[length - 1] = ch;
	data[length] = '\0';
}
string& string::append(const char* s) {
	return append(s, strlen(s));
}
string& string::append(const char* s, size_t n) {
	size_t old_length = length;
	length += n;
	if (length >= cap) {
		shrink_to_fit();
	}
	memcpy(data + old_length, s, n);
	data[length] = '\0';
	return *this;
}
string& string::operator+=(string& str) {
	append(str.c_str(), str.size());
	return *this;
}