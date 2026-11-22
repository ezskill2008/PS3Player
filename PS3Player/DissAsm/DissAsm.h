#pragma once
#include <PS3Player/Elf/Elf.h>
#include <PS3Player/Emu/PPUInstructions.h>

#define LC_INSTRUCTION_LENGTH 64

struct DissassemblyOptions {
	bool		includePc{ false };
	bool		followThrough{ false };
	uint32_t	nonFollowOps{ 256 };
};

void PrintInstruction(
	uint32_t raw,
	uint32_t pc);

bool OutputDissassemblyToStdOut(
	const Elf64_Program& prg,
	const GuestMem& mem,
	const DissassemblyOptions& options = {});
