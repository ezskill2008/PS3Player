#pragma once
#include <PS3Player/Elf/Elf.h>
#include <libPs3Core/Include/PPUInstructions.h>

#define LC_INSTRUCTION_LENGTH 64

bool OutputDissAssemblyToStdOut(
	const Elf64_Program& prg,
	const GuestMem& mem);
