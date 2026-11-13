#include <PS3Player/Elf/Elf.h>

static const uint8_t kElfMagic[4]{ 0x7F, 'E', 'L', 'F' };

bool
readElfEhdr(
	const std::vector<uint8_t>& elf,
	Elf64_Ehdr& hdr) {
	if (!(elf.data() && elf.size() >= sizeof(Elf64_Ehdr)))
		return false;

	hdr = {};
	memcpy(&hdr.ident[0], elf.data(), 4);
	for (uint32_t i{ 0 }; i < 4; ++i)
		if (hdr.ident[i] != kElfMagic[i]) return false;

	hdr.type = LcBe16(&elf[offsetof(Elf64_Ehdr, type)]);
	hdr.machine = LcBe16(&elf[offsetof(Elf64_Ehdr, machine)]);
	hdr.version = LcBe32(&elf[offsetof(Elf64_Ehdr, version)]);
	hdr.entry = LcBe64(&elf[offsetof(Elf64_Ehdr, entry)]);
	hdr.phoff = LcBe64(&elf[offsetof(Elf64_Ehdr, phoff)]);
	hdr.shoff = LcBe64(&elf[offsetof(Elf64_Ehdr, shoff)]);
	hdr.flags = LcBe32(&elf[offsetof(Elf64_Ehdr, flags)]);
	hdr.ehsize = LcBe16(&elf[offsetof(Elf64_Ehdr, ehsize)]);
	hdr.phentsize = LcBe16(&elf[offsetof(Elf64_Ehdr, phentsize)]);
	hdr.phnum = LcBe16(&elf[offsetof(Elf64_Ehdr, phnum)]);
	hdr.shentsize = LcBe16(&elf[offsetof(Elf64_Ehdr, shentsize)]);
	hdr.shnum = LcBe16(&elf[offsetof(Elf64_Ehdr, shnum)]);
	hdr.shstrndx = LcBe16(&elf[offsetof(Elf64_Ehdr, shstrndx)]);

	if (hdr.machine != EM_PPC64) return false;
	if (!hdr.version) return false;

	return true;
}

bool
readElfPhdr(
	const std::vector<uint8_t>& elf,
	const Elf64_Ehdr& ehdr,
	uint16_t index,
	Elf64_Phdr& hdr) {
	uint64_t off{ ehdr.phoff + (uint64_t)index * ehdr.phentsize };

	if (index >= ehdr.phnum) return false;
	if (!(elf.data() && off + sizeof(Elf64_Phdr) <= elf.size())) return false;

	memcpy(&hdr, elf.data() + off, sizeof(Elf64_Phdr));
	hdr.type = LcBSwap32(hdr.type);
	hdr.flags = LcBSwap32(hdr.flags);
	hdr.offset = LcBSwap64(hdr.offset);
	hdr.vaddr = LcBSwap64(hdr.vaddr);
	hdr.paddr = LcBSwap64(hdr.paddr);
	hdr.filesz = LcBSwap64(hdr.filesz);
	hdr.memsz = LcBSwap64(hdr.memsz);
	hdr.align = LcBSwap64(hdr.align);

	return true;
}

bool
readOpd(const GuestMem& mem, uint32_t addr, Elf64_Opd& opd) {
	if (!mem.IsMapped(addr, sizeof(Elf64_Opd))) return false;

	opd.entryPc = mem.Read32(addr);
	opd.toc = mem.Read32(addr + 4);

	return true;
}

bool
LoadElfProgram(
	const std::vector<uint8_t>& elf,
	GuestMem& mem,
	Elf64_Program& prg) {
	if (!(elf.size() && elf.data() && mem.base)) return false;
	if (!readElfEhdr(elf, prg.ehdr)) return false;

	const uint32_t phnum{ prg.ehdr.phnum };
	prg.phdrs.resize(phnum);

	for (uint32_t i{ 0 }; i < phnum; ++i)
		if (!readElfPhdr(
			elf,
			prg.ehdr,
			i,
			prg.phdrs[i]))
			return false;

	for (uint16_t i{ 0 }; i < phnum; ++i) {
		const Elf64_Phdr& p{ prg.phdrs[i] };
		if (p.type != PT_LOAD) continue;
		if (p.memsz == 0) continue;
		if (p.vaddr + p.memsz > GuestMem::kSize || p.offset + p.filesz > elf.size()) return false;

		if (!mem.Map(
			(uint32_t)p.vaddr,
			p.memsz,
			(p.flags & PF_EXEC)))
			return false;

		memcpy(mem.base + p.vaddr, elf.data() + p.offset, (size_t)p.filesz);
	}

	if (!readOpd(mem, (uint32_t)prg.ehdr.entry, prg.opd)) return false;
	if (!mem.IsExecutable(prg.opd.entryPc))		return false;

	return true;
}

bool
LoadProcessParams(
	const GuestMem& mem,
	const Elf64_Program& prg,
	SysProcessParam& params) {
	params = {};
	if (!(mem.base && !prg.phdrs.empty())) return false;

	uint32_t index{ 0 };

	for (; index < prg.phdrs.size(); ++index)
		if (prg.phdrs[index].type == PT_PROCPARAMS)
			break;

	if (index == prg.phdrs.size()) return false;

	const Elf64_Phdr& phdr{ prg.phdrs[index] };

	if (!mem.IsMapped((uint32_t)phdr.vaddr, 0x20)) return false;

	const uint8_t* const base{ mem.base + phdr.vaddr };
	const uint32_t size{ LcBSwap32(LcRead32(base)) };

	if (size < 0x20 || !mem.IsMapped((uint32_t)phdr.vaddr, size)) return false;
	memcpy(&params, base, size);
	
	params.size = LcBSwap32(params.size);
	params.magic = LcBSwap32(params.magic);
	params.version = LcBSwap32(params.version);
	params.sdkVersion = LcBSwap32(params.sdkVersion);
	params.primaryPrio = LcBSwap32(params.primaryPrio);
	params.primaryStackSize = LcBSwap32(params.primaryStackSize);
	params.mallocPageSize = LcBSwap32(params.mallocPageSize);
	params.ppcSeg = LcBSwap32(params.ppcSeg);
	params.crashDumpParamAddr = LcBSwap32(params.crashDumpParamAddr);

	return true;
}

bool
LoadPrxParams(
	const GuestMem& mem,
	const Elf64_Program& prg,
	SysProcPrxParam& params) {
	params = {};
	if (!(mem.base && !prg.phdrs.empty())) return false;

	uint32_t index{ 0 };

	for (; index < prg.phdrs.size(); ++index)
		if (prg.phdrs[index].type == PT_PRXPARAMS)
			break;

	if (index == prg.phdrs.size()) return false;

	const Elf64_Phdr& phdr{ prg.phdrs[index] };

	if (!mem.IsMapped((uint32_t)phdr.vaddr, 0x28)) return false;
	const uint8_t* const base{ mem.base + phdr.vaddr };
	const uint32_t size{ LcBSwap32(LcRead32(base)) };

	if (size < 0x28 || !mem.IsMapped((uint32_t)phdr.vaddr, size)) return false;
	memcpy(&params, base, size);

	params.size = LcBSwap32(params.size);
	params.magic = LcBSwap32(params.magic);
	params.version = LcBSwap32(params.version);
	params.pad0 = LcBSwap32(params.pad0);
	params.libEntStart = LcBSwap32(params.libEntStart);
	params.libEntEnd = LcBSwap32(params.libEntEnd);
	params.libStubStart = LcBSwap32(params.libStubStart);
	params.libStubEnd = LcBSwap32(params.libStubEnd);
	params.ver = LcBSwap16(params.ver);
	params.pad1 = LcBSwap16(params.pad1);
	params.pad2 = LcBSwap32(params.pad2);

	return true;
}
