#pragma once
#include <PS3Player/Core/GuestMem.h>
#include <PS3Player/Emu/PPUInstructions.h>
#include <libPs3Core/Include/CoreTypes.h>

#include <bit>

struct PPURegisters {
	uint64_t	gpr[32]{};
	uint64_t	fpr[32]{};
	__m128		vmx[32]{};
	uint32_t	cr{ 0 };
	uint64_t	lr{ 0 };
	uint64_t	ctr{ 0 };
	uint64_t	xer{ 0 };
	uint32_t	pc{ 0 };

	inline double GetFprDouble(int i) const { return std::bit_cast<double>(fpr[i]); }
	inline void SetFprDouble(int i, double v) { fpr[i] = std::bit_cast<uint64_t>(v); }
};

struct PPU {
	PPURegisters	reg{};
	InstField		inst{};
	InstData		instData{};
	GuestMem*		guestMem{};
	uint64_t		operands[5]{};
};

void PPUCycle(
	PPU& ppu);
