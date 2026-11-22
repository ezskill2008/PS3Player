#pragma once

#define NOMINMAX
#include <stdint.h>
#include <Windows.h>
#include <immintrin.h>

#include <type_traits>

// libPs3Core.dll
// The core interface on which every submodule may depend
// Functions in this dll should be prefixed with Lc
// Functions in other submodules should be prefixed with Lps

#define CORE_API __declspec(dllexport)

#if LPS3_BUILDING_DLL
#define LPS_API __declspec(dllexport)
#else
#define LPS_API __declspec(dllimport)
#endif

#define NOTHROW (std::nothrow)

struct SPEState {
	__m128		ugp[32]{};
	uint64_t	pc{};
};
