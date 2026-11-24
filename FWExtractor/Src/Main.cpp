#include <Src/Common.h>

#pragma comment(lib, "libPs3Core.lib")

struct PupPackage {
	std::ifstream					pupFile{};
	size_t							pupSize{};
	ScePupHeader					hdr{};
	std::vector<ScePupSegmentEntry>	entries{};
};

bool
loadPackageFile(
	const char* file,
	PupPackage& pup) {
	if (!(file && std::filesystem::exists(file)))
		return false;

	pup.pupSize = std::filesystem::file_size(file);
	pup.pupFile.open(file, std::ios::in | std::ios::binary);

	return pup.pupFile.is_open();
}

bool
parsePup(
	PupPackage& pup) {
	if (!pup.pupFile.is_open())
		return false;

	constexpr const char* kMagic{ "SCEUF\0\0\0" };
	pup.pupFile.read(reinterpret_cast<char*>(&pup.hdr), sizeof(ScePupHeader));

	for (uint32_t i{ 0 }; i < 0x7; ++i)
		if (pup.hdr.magic[i] != kMagic[i])
			return false;

	pup.hdr.packageVersion = LcBSwap64(pup.hdr.packageVersion);
	pup.hdr.imageVersion = LcBSwap64(pup.hdr.imageVersion);
	pup.hdr.segmentNum = LcBSwap64(pup.hdr.segmentNum);
	pup.hdr.fileOffset = LcBSwap64(pup.hdr.fileOffset);
	pup.hdr.fileSize = LcBSwap64(pup.hdr.fileSize);

	const uint32_t segNum{ static_cast<uint32_t>(pup.hdr.segmentNum) };

	pup.entries.resize(segNum);
	
	for (uint32_t i{ 0 };i < segNum;++i) {
		ScePupSegmentEntry& entry{ pup.entries[i] };
		
		pup.pupFile.read(reinterpret_cast<char*>(&entry), sizeof(ScePupSegmentEntry));

		entry.id = LcBSwap64(entry.id);
		entry.offset = LcBSwap64(entry.offset);
		entry.size = LcBSwap64(entry.size);
		entry.signAlgorithm = LcBSwap32(entry.signAlgorithm);
		entry.padding = LcBSwap32(entry.padding);
	}

	return true;
}

int main(int argc, char** argv) {
	PupPackage pup{};

	if (!loadPackageFile("D:\\PS3UPDAT.PUP", pup)) {
		printf("Failed to load PS3UPDAT.PUP!\n");
		return -1;
	}

	if (!parsePup(pup)) {
		printf("Failed to parse pup!\n");
		return -1;
	}


}
