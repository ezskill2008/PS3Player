#include <libPs3Decrypt/Include/DecryptISO.h>

#include <fstream>
#include <filesystem>

#pragma comment(lib, "libPs3Core.lib")

struct IsoDisk_T {
	uint32_t			regionCount{};
	size_t				totalSize{};
	std::ifstream		iFile{};
	uint8_t				sector0[ISO_SECTOR_SIZE]{};
};

size_t
readEntireFile(const char* path,
	std::unique_ptr<uint8_t[]>& data) {
	if (!std::filesystem::exists(path)) {
		DUMP_ERROR return 0;
	}

	size_t size{ std::filesystem::file_size(path) };
	if (size == 0) return 0;

	data = std::make_unique<uint8_t[]>(size);

	std::ifstream ifile{ path, std::ios::in };
	ifile.read((char*)data.get(), size);
	ifile.close();

	return size;
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

	temp->totalSize = std::filesystem::file_size(path);
	temp->iFile.open(path);
	temp->iFile.read((char*)&temp->sector0[0], ISO_SECTOR_SIZE);
	temp->regionCount = BSwap32(LcRead32(&temp->sector0[0]));

	disk = temp;

	return true;
}

void
LpsReleaseDisk(IsoDisk& disk) {
	if (!disk) { DUMP_ERROR return; }
	disk->iFile.close();

	delete disk;
}
