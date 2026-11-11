#pragma once

#define NOMINMAX
#include <stdint.h>
#include <Windows.h>
#include <immintrin.h>

#define CORE_API __declspec(dllexport)

#if LPS3_BUILDING_DLL
#define LPS_API __declspec(dllexport)
#else
#define LPS_API __declspec(dllimport)
#endif

#define NOTHROW (std::nothrow)

struct PPUState {
	uint64_t	gpr[32]{};
	uint64_t	fpr[32]{};
	__m128		vmx[32]{};
	uint32_t	cr{ 0 };
	uint64_t	lr{ 0 };
	uint64_t	ctr{ 0 };
	uint64_t	xer{ 0 };
	uint32_t	pc{ 0 };
};

struct SPEState {
	__m128		ugp[32]{};
	uint64_t	pc{};
};
