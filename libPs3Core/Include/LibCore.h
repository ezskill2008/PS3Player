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
LcBSwap16(uint16_t b) {
	return _byteswap_ushort(b);
}

inline uint32_t
LcBSwap32(uint32_t b) {
	return _byteswap_ulong(b);
}

inline uint64_t
LcBSwap64(uint64_t b) {
	return _byteswap_uint64(b);
}

inline uint16_t
LcRead16(const uint8_t* const stream) {
	uint16_t value{ 0 };
	memcpy(&value, stream, sizeof(uint16_t));
	return value;
}

inline uint32_t
LcRead32(const uint8_t* const stream) {
	uint32_t value{ 0 };
	memcpy(&value, stream, sizeof(uint32_t));
	return value;
}

inline uint64_t
LcRead64(const uint8_t* const stream) {
	uint64_t value{ 0 };
	memcpy(&value, stream, sizeof(uint64_t));
	return value;
}

inline uint16_t
LcRead16BE(const uint8_t* const stream) {
	uint16_t value{ 0 };
	memcpy(&value, stream, sizeof(uint16_t));
	return LcBSwap16(value);
}

inline uint32_t
LcRead32BE(const uint8_t* const stream) {
	uint32_t value{ 0 };
	memcpy(&value, stream, sizeof(uint32_t));
	return LcBSwap32(value);
}

inline uint64_t
LcRead64BE(const uint8_t* const stream) {
	uint64_t value{ 0 };
	memcpy(&value, stream, sizeof(uint64_t));
	return LcBSwap64(value);
}

inline uint16_t
LcBe16(const uint8_t* const p) {
	return uint16_t((p[0] << 8) | p[1]);
}

inline uint32_t
LcBe32(const uint8_t* const p) {
	return (uint32_t{ p[0] } << 24) | (uint32_t{ p[1] } << 16) |
		(uint32_t{ p[2] } << 8) | uint32_t{ p[3] };
}

inline uint64_t
LcBe64(const uint8_t* const p) {
	return (uint64_t{ LcBe32(p) } << 32) | LcBe32(p + 4);
}

inline uint16_t
LcLe16(const uint8_t* const p) {
	return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}

inline uint32_t
LcLe32(const uint8_t* const p) {
	return uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
		(uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

inline uint64_t
Le64(const uint8_t* const p) {
	return uint64_t(p[0]) | (uint64_t(p[1]) << 8) |
		(uint64_t(p[2]) << 16) | (uint64_t(p[3]) << 24) |
		(uint64_t(p[4]) << 32) | (uint64_t(p[5]) << 40) |
		(uint64_t(p[6]) << 48) | (uint64_t(p[7]) << 56);
}
