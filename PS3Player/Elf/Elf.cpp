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

	hdr.type = Be16(&elf[offsetof(Elf64_Ehdr, type)]);
	hdr.machine = Be16(&elf[offsetof(Elf64_Ehdr, machine)]);
	hdr.version = Be32(&elf[offsetof(Elf64_Ehdr, version)]);
	hdr.entry = Be64(&elf[offsetof(Elf64_Ehdr, entry)]);
	hdr.phoff = Be64(&elf[offsetof(Elf64_Ehdr, phoff)]);
	hdr.shoff = Be64(&elf[offsetof(Elf64_Ehdr, shoff)]);
	hdr.flags = Be32(&elf[offsetof(Elf64_Ehdr, flags)]);
	hdr.ehsize = Be16(&elf[offsetof(Elf64_Ehdr, ehsize)]);
	hdr.phentsize = Be16(&elf[offsetof(Elf64_Ehdr, phentsize)]);
	hdr.phnum = Be16(&elf[offsetof(Elf64_Ehdr, phnum)]);
	hdr.shentsize = Be16(&elf[offsetof(Elf64_Ehdr, shentsize)]);
	hdr.shnum = Be16(&elf[offsetof(Elf64_Ehdr, shnum)]);
	hdr.shstrndx = Be16(&elf[offsetof(Elf64_Ehdr, shstrndx)]);

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
	hdr.type = BSwap32(hdr.type);
	hdr.flags = BSwap32(hdr.flags);
	hdr.offset = BSwap64(hdr.offset);
	hdr.vaddr = BSwap64(hdr.vaddr);
	hdr.paddr = BSwap64(hdr.paddr);
	hdr.filesz = BSwap64(hdr.filesz);
	hdr.memsz = BSwap64(hdr.memsz);
	hdr.align = BSwap64(hdr.align);

	return true;
}

Elf64_Entry
parsEntry() {
	return {};
}

bool
LoadElfProgram(
	const std::vector<uint8_t>& elf,
	Elf64_Program& prg) {
	if (prg.mem.base) prg.mem.Release();
	prg = {};
	if (!prg.mem.Init()) return false;
	if (!prg.mem.base) return false;//To step intellisense crying

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

		if (p.vaddr + p.memsz > GuestMem::kSize || p.offset + p.filesz > elf.size()) return false;

		if (!prg.mem.Map(
			(uint32_t)p.vaddr,
			p.memsz,
			(p.flags & PF_EXEC)))
			return false;

		memcpy(prg.mem.base + p.vaddr, elf.data() + p.offset, (size_t)p.filesz);
	}

	return true;
}
