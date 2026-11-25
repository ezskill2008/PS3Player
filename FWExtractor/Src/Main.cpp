#include <Src/Common.h>
#include <Src/TarLoader.h>

#include <libPs3Decrypt/Include/DecryptSELF.h>

#pragma comment(lib, "libPs3Core.lib")

struct PupPackage {
	std::ifstream					pupFile{};
	size_t							pupSize{};
	ScePupHeader					hdr{};
	std::vector<ScePupSegmentEntry>	entries{};
};

ScePupSegmentEntry
findSegmentById(
	const PupPackage& pup,
	uint64_t id) {
	for (const auto& e : pup.entries) {
		if (e.id == id)
			return e;
	}

	return {};
}

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

bool
installFirmWare(
	PupPackage& pup,
	const char* targetDir) {
	if (!(pup.pupFile.is_open() && targetDir))
		return false;

	if (!std::filesystem::exists(targetDir)) {
		if (!std::filesystem::create_directories(targetDir))
			return false;
	}

	const ScePupSegmentEntry e{ findSegmentById(pup, static_cast<uint64_t>(SegmentId::UpdateFiles)) };
	if (e.id != static_cast<uint64_t>(SegmentId::UpdateFiles)) return false;

	size_t buffer_size{ static_cast<size_t>(e.size) };
	std::vector<uint8_t> buffer{};
	buffer.resize(buffer_size);

	pup.pupFile.seekg(e.offset);
	pup.pupFile.read(reinterpret_cast<char*>(buffer.data()), buffer_size);
	
	std::vector<TarEntry> entries{};

	if (!ParseTarFile(buffer, entries)) {
		return false;
	}

	LogTarEntries(entries);

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

	if (!installFirmWare(pup, "D:\\CEX4.93\\")) {
		printf("Failed to install pup!\n");
		return -1;
	}
}
