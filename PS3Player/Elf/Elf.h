#pragma once
#include <libPs3Core/Include/LibCore.h>
#include <PS3Player/Core/GuestMem.h>

#include <vector>

#define PT_LOAD 1
#define PT_PROCPARAMS 0x60000001
#define PT_PRXPARAMS 0x60000002
#define PT_TLS 7

enum ElfType : uint16_t {
    ET_NONE = 0,
    ET_REL = 1,
    ET_EXEC = 2,
    ET_DYN = 3,
    ET_CORE = 4,
    ET_LOOS = 0xfe00,
    ET_HIOS = 0xfeff,
    ET_LOPROC = 0xff00,
    ET_HIPROC = 0xffff,
};

enum ElfMachine : uint16_t {
    EM_PPC64 = 21,
};

enum ElfPermFlags : uint32_t {
    PF_NONE = 0,
    PF_EXEC = 0x1,             // Execute
    PF_WRITE = 0x2,             // Write
    PF_READ = 0x4,             // Read
};

struct Elf64_Ehdr {
    uint8_t         ident[16];
    uint16_t        type;
    uint16_t        machine;
    uint32_t        version;
    uint64_t        entry;
    uint64_t        phoff;
    uint64_t        shoff;
    uint32_t        flags;
    uint16_t        ehsize;
    uint16_t        phentsize;
    uint16_t        phnum;
    uint16_t        shentsize;
    uint16_t        shnum;
    uint16_t        shstrndx;
};

struct Elf64_Phdr {
    uint32_t	    type;
    uint32_t	    flags;
    uint64_t	    offset;
    uint64_t	    vaddr;
    uint64_t	    paddr;
    uint64_t	    filesz;
    uint64_t	    memsz;
    uint64_t	    align;
};

struct Elf64_Entry {
    uint32_t        entryPc;
    uint32_t        toc;
};

struct Elf64_Program {
    Elf64_Ehdr              ehdr{};
    std::vector<Elf64_Phdr> phdrs{};
    GuestMem                mem{};
    Elf64_Entry             entry{};
};

bool LoadElfProgram(
    const std::vector<uint8_t>& elf,
    Elf64_Program& prg);
