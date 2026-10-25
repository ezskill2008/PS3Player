#include <libPs3Decrypt/Include/DecryptISO.h>

#include <Windows.h>
#include <filesystem>

#pragma comment(lib, "libPs3Core.lib")

struct DiskRegion {
	uint32_t			start{};
	uint32_t			end{};
	bool				encrypted{};
};

struct IsoDisk_T {
	CryptKey			cryptKey{};
	uint32_t			regionCount{};
	size_t				totalSize{};
	uint8_t				sector0[ISO_SECTOR_SIZE]{};
	DiskRegion			regions[256]{};
};

void
f() {
	OVERLAPPED overlapped{};
	//ReadFile();
}

HANDLE
openFileForRead(const char* path) {
	const int n = MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0);
	if (n <= 0) return INVALID_HANDLE_VALUE;
	wchar_t* wide = new (std::nothrow) wchar_t[n];
	if (!wide) return INVALID_HANDLE_VALUE;
	MultiByteToWideChar(CP_UTF8, 0, path, -1, wide, n);

	HANDLE h{ CreateFileW(wide,
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
		nullptr) };
	delete[] wide;

	return h;
}

bool
LpsReadKey(
	const char* keyFile,
	CryptKey& key) {
	if (!keyFile) { DUMP_ERROR return false; }

	std::unique_ptr<uint8_t[]> fileData{};
	size_t fileSize{ readEntireFile(keyFile, fileData) };
	if (fileSize != ISO_KEY_SIZE) { DUMP_ERROR return false; }

	memcpy(&key.bytes[0], fileData.get(), ISO_KEY_SIZE);

	return true;
}

bool
LpsLoadDisk(
	const char* path,
	IsoDisk& disk) {
	if (!path) { DUMP_ERROR return false; }
	if (disk) {
		LpsReleaseDisk(disk);
	}

	IsoDisk_T* temp{ new IsoDisk_T() };
	if (!temp) { DUMP_ERROR return false; }

	std::ifstream iFile{ path, std::ios::binary };
	
	temp->totalSize = std::filesystem::file_size(path);
	temp->regionCount = BSwap32(LcRead32(&temp->sector0[0]));

	disk = temp;

	return true;
}

void
LpsReleaseDisk(IsoDisk& disk) {
	if (!disk) { DUMP_ERROR return; }

	delete disk;
}
