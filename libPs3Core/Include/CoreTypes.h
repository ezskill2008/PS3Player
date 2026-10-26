#pragma once
#include <stdint.h>

#define CORE_API __declspec(dllexport)

#if LPS3_BUILDING_DLL
#define LPS_API __declspec(dllexport)
#else
#define LPS_API __declspec(dllimport)
#endif

#define NOTHROW (std::nothrow)
