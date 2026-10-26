#pragma once
#include <libPs3Core/Include/LibCore.h>

#define NOMINMAX
#include <Windows.h>

HANDLE openForRead(
	const char* path);

bool readAt(
	HANDLE h, 
	uint64_t offset, 
	void* dst, 
	size_t size);

inline int hexVal(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}
