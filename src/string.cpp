#include <string>
#include <string.h>
#include <stdlib.h>
#include <new>
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
	cap = 1;
	while (length >= cap) cap *= 2;
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