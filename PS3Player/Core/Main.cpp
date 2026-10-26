#include <libPs3Decrypt/Include/DecryptISO.h>
#include <libPs3Decrypt/Include/DecryptSELF.h>

#pragma comment(lib, "libPs3Core.lib")
#pragma comment(lib, "libPs3Decrypt.lib")

#include <Windows.h>
#include <vector>
#include <string>

void
PrintTree(IsoDisk disk, const std::string& dir) {
	struct Ctx { IsoDisk disk; const std::string* dir; };
	Ctx ctx{ disk, &dir };

	LpsListDir(disk, dir.c_str(), [](const char* name, const IsoFileInfo* info, void* user) {
		const Ctx& c{ *static_cast<Ctx*>(user) };
		const std::string full{ *c.dir + "/" + name };

		if (info->isDirectory) {
			printf("%s/\n", full.c_str());
			PrintTree(c.disk, full);
		}
		else {
			printf("%s  (%llu bytes)\n", full.c_str(), (unsigned long long)info->size);
		}
		return true;
		}, &ctx);
}

int main() {
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	CryptKey isoKey{};
	if (!LpsReadKey(
		"D:\\code\\PS3Content\\Minerva_Myrient\\Redump\\Sony - PlayStation 3 - Disc Keys\\Brain Challenge (Europe) (En,Fr,De,Es,It,Nl).key",
		isoKey))
		return -1;

	IsoDisk disk{};
	if (!LpsLoadDisk(
		"D:\\code\\PS3Content\\Minerva_Myrient\\Redump\\Sony - PlayStation 3\\Brain Challenge (Europe) (En,Fr,De,Es,It,Nl).iso",
		disk))
		return -1;;

	if (!(LpsSetDiskKey(disk, isoKey)
		&& LpsMountFileSys(disk))) {
		LpsReleaseDisk(disk);
		return -1;
	}

	PrintTree(disk, "");

	LpsListDir(disk, "/", [](const char* name, const IsoFileInfo* info, void*) {
		printf("%s %s %llu\n", info->isDirectory ? "<DIR>" : "     ", name,
			(unsigned long long)info->size);
		return true;
		}, nullptr);

	// read EBOOT.BIN fully
	IsoFile f{ nullptr };
	if (!LpsOpenFile(disk, "PS3_GAME/USRDIR/EBOOT.BIN", f)) {
		LpsReleaseDisk(disk);
		return -1;
	}

	std::vector<uint8_t> eboot(size_t(LpsFileSize(f)));
	const size_t got{ LpsReadFile(f, 0, eboot.data(), eboot.size()) };
	// got == eboot.size(), and eboot[0..3] should be 53 43 45 00 ("SCE\0")
	LpsCloseFile(f);

	SelfKeys keys{};

	if (!LpsLoadSelfKeys("D:\\code\\PS3Content\\keys.txt", keys))
		return -1;

	size_t elfSize{ 0 };
	if (!LpsSelfToElf(keys, eboot.data(), eboot.size(), nullptr, 0, elfSize))
		return -1;

	std::vector<uint8_t> elf(elfSize);
	if (!LpsSelfToElf(keys, eboot.data(), eboot.size(), elf.data(), elf.size(), elfSize))
		return -1;

	LpsReleaseDisk(disk);

	return 0;
}
