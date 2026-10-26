#pragma once
#include <libPs3Core/Include/CoreTypes.h>

#include <memory>
#include <stdlib.h>

#define DUMP_ERROR LcLogError(__FILE__, __FUNCTION__, __LINE__);

CORE_API void LcLogError(
	const char* file,
	const char* function,
	size_t line);

inline uint16_t
BSwap16(uint16_t b) {
	return _byteswap_ushort(b);
}

inline uint32_t
BSwap32(uint32_t b) {
	return _byteswap_ulong(b);
}

inline uint64_t
BSwap64(uint64_t b) {
	return _byteswap_uint64(b);
}

inline uint16_t
LcRead16(uint8_t* stream) {
	uint16_t value{ 0 };
	memcpy(&value, stream, sizeof(uint16_t));
	return value;
}

inline uint32_t
LcRead32(uint8_t* stream) {
	uint32_t value{ 0 };
	memcpy(&value, stream, sizeof(uint32_t));
	return value;
}

inline uint64_t
LcRead64(uint8_t* stream) {
	uint64_t value{ 0 };
	memcpy(&value, stream, sizeof(uint64_t));
	return value;
}

inline uint16_t
LcRead16BE(uint8_t* stream) {
	uint16_t value{ 0 };
	memcpy(&value, stream, sizeof(uint16_t));
	return BSwap16(value);
}

inline uint32_t
LcRead32BE(uint8_t* stream) {
	uint32_t value{ 0 };
	memcpy(&value, stream, sizeof(uint32_t));
	return BSwap32(value);
}

inline uint64_t
LcRead64BE(uint8_t* stream) {
	uint64_t value{ 0 };
	memcpy(&value, stream, sizeof(uint64_t));
	return BSwap64(value);
}

inline uint16_t
Be16(const uint8_t* p) {
	return uint16_t((p[0] << 8) | p[1]); 
}

inline uint32_t
Be32(const uint8_t* p) { 
	return (uint32_t{ p[0] } << 24) | (uint32_t{ p[1] } << 16) | 
		(uint32_t{ p[2] } << 8) | uint32_t{ p[3] }; 
}

inline uint64_t
Be64(const uint8_t* p) { 
	return (uint64_t{ Be32(p) } << 32) | Be32(p + 4); 
}

inline uint16_t
Le16(const uint8_t* p) {
	return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}

inline uint32_t
Le32(const uint8_t* p) {
	return uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
		(uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

inline uint64_t
Le64(const uint8_t* p) {
	return uint64_t(p[0]) | (uint64_t(p[1]) << 8) |
		(uint64_t(p[2]) << 16) | (uint64_t(p[3]) << 24) |
		(uint64_t(p[4]) << 32) | (uint64_t(p[5]) << 40) |
		(uint64_t(p[6]) << 48) | (uint64_t(p[7]) << 56);
}
