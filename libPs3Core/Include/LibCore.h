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
