#pragma once
#include <PS3Player/Elf/Elf.h>
#include <libPs3Core/Include/PPUInstructions.h>

#define LC_INSTRUCTION_LENGTH 64

struct DissassemblyOptions {
	bool		includePc{ false };
	bool		followThrough{ false };
	uint32_t	nonFollowOps{ 256 };
};

bool OutputDissassemblyToStdOut(
	const Elf64_Program& prg,
	const GuestMem& mem,
	const DissassemblyOptions& options = {});
