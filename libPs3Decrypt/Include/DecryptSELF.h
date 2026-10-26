#pragma once
#include <libPs3Core/Include/LibCore.h>

typedef struct SelfKeys_T* SelfKeys;

LPS_API bool LpsLoadSelfKeys(
	const char* keyFile,
	SelfKeys& keys);

LPS_API void LpsReleaseSelfKeys(
	SelfKeys& keys);

LPS_API bool LpsSelfToElf(
	SelfKeys keys,
	const void* selfIn,
	size_t selfSize,
	void* elfOut,
	size_t elfOutCapacity,
	size_t& elfSize);
