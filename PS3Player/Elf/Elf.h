#pragma once
#include <libPs3Core/Include/LibCore.h>
#include <PS3Player/Core/GuestMem.h>

#include <vector>

#define EM_PPC64            21
#define PT_LOAD             0x1
#define PT_PROCPARAMS       0x60000001
#define PT_PRXPARAMS        0x60000002
#define PT_TLS              0x7
#define SHT_SCE_RELA        0x60000000
#define SHT_SCE_NID         0x61000001
#define SHT_SCE_IOPMOD      0x70000080
#define SHT_SCE_EEMOD       0x70000090
#define SHT_SCE_PSPRELA     0x700000A0
#define SHT_SCE_PPURELA     0x700000A4
#define PROCPARAMS_MAGIC    0x13BCC5F6
#define PRXPARAMS_MAGIC     0x1B434CEC

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

enum ElfPermFlags : uint32_t {
    PF_NONE = 0,
    PF_EXEC = 0x1,             // Execute
    PF_WRITE = 0x2,            // Write
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

struct Elf64_Shdr {//Is this needed anywhere?
    uint32_t        name;
    uint32_t        type;
    uint64_t        flags;
    uint64_t        addr;
    uint64_t        offset;  
    uint64_t        size;
    uint32_t        link;
    uint32_t        info;    
    uint64_t        addralign;
    uint64_t        entsize;
};

struct Elf64_Opd {
    uint32_t        entryPc;
    uint32_t        toc;
};

struct Elf64_Program {
    Elf64_Ehdr              ehdr{};
    std::vector<Elf64_Phdr> phdrs{};
    Elf64_Opd               opd{};
};

struct SysProcessParam {
    uint32_t                size{ 0 };               // 0x24 in most games, 0x20 in older ones
    uint32_t                magic{ 0 };              // 0x13BCC5F6
    uint32_t                version{ 0 };            // 0x00000001 up to 0x003x0001 range
    uint32_t                sdkVersion{ 0 };         // 0xFFFFFFFF if unknown
    int32_t                 primaryPrio{ 0 };        // main thread priority, e.g. 1001
    uint32_t                primaryStackSize{ 0 };   // e.g. 0x100000 (1 MB)
    uint32_t                mallocPageSize{ 0 };     // e.g. 0x100000 (1 MB)
    uint32_t                ppcSeg{ 0 };             // 0 = default, 1 = OVLM, 2 = fixed addr PRX
    uint32_t                crashDumpParamAddr{ 0 }; // only present in newer versions (size 0x40)
};

struct SysProcPrxParam {
    uint32_t                size{ 0 };          // 0x28
    uint32_t                magic{ 0 };         // 0x1B434CEC
    uint32_t                version{ 0 };       // 1
    uint32_t                pad0{ 0 };
    uint32_t                libEntStart{ 0 };   // exported library table range
    uint32_t                libEntEnd{ 0 };
    uint32_t                libStubStart{ 0 };  // imported library stub table range
    uint32_t                libStubEnd{ 0 };
    uint16_t                ver{ 0 };           // 0x0101
    uint16_t                pad1{ 0 };
    uint32_t                pad2{ 0 };
};

bool LoadElfProgram(
    const std::vector<uint8_t>& elf,
    GuestMem& mem,
    Elf64_Program& prg);

bool LoadProcessParams(
    const GuestMem& mem,
    const Elf64_Program& prg,
    SysProcessParam& params);

bool LoadPrxParams(
    const GuestMem& mem,
    const Elf64_Program& prg,
    SysProcPrxParam& params);
