#include <Src/TarLoader.h>

#include <iostream>

typedef struct PosixHeader {					 // byte offsets
	char				name[100];               //   0
	char				mode[8];                 // 100
	char				uid[8];                  // 108
	char				gid[8];                  // 116
	char				size[12];                // 124
	char				mtime[12];               // 136
	char				chksum[8];               // 148
	char				typeflag;                // 156
	char				linkname[100];           // 157
	char				magic[6];                // 257
	char				version[2];              // 263
	char				uname[32];               // 265
	char				gname[32];               // 297
	char				devmajor[8];             // 329
	char				devminor[8];             // 337
	char				prefix[155];             // 345
	uint8_t				_pad[12];
} PosixHeader;

static_assert(sizeof(PosixHeader) == 512);

uint64_t
parseOctal(const char* str, size_t length) {
	size_t i{ 0 };

	while (i < length && str[i] == ' ')
		++i;

	if (i == length || str[i] < '0' || str[i] > '7')
		return ~static_cast<uint64_t>(0);

	uint64_t result{ 0 };

	for (; i < length; ++i) {
		const char c{ str[i] };

		if (c == '\0' || c == ' ')
			break;

		if (c < '0' || c > '7')
			return ~static_cast<uint64_t>(0);

		result = (result << 3) | static_cast<uint64_t>(c - '0');
	}

	return result;
}

bool
parseTarHeader(
	const uint8_t* const tar, 
	TarHeader& hdr) {
	if (!tar)
		return false;

	PosixHeader rawHdr{};

	memcpy(&rawHdr, tar, sizeof(PosixHeader));

	if (memcmp(rawHdr.magic, TMAGIC, TMAGLEN) != 0)
		return false;

	memcpy(hdr.name, rawHdr.name, 100);
	hdr.mode = static_cast<uint32_t>(parseOctal(rawHdr.mode, 8));
	hdr.uid = static_cast<uint32_t>(parseOctal(rawHdr.uid, 8));
	hdr.gid = static_cast<uint32_t>(parseOctal(rawHdr.gid, 8));
	hdr.size = parseOctal(rawHdr.size, 12);
	hdr.mtime = parseOctal(rawHdr.mtime, 12);
	hdr.chksum = static_cast<uint32_t>(parseOctal(rawHdr.chksum, 8));
	memcpy(hdr.linkname, rawHdr.linkname, 100);
	memcpy(hdr.magic, rawHdr.magic, 6);
	hdr.version = static_cast<uint8_t>(parseOctal(rawHdr.version, 1));
	memcpy(hdr.uname, rawHdr.uname, 32);
	memcpy(hdr.gname, rawHdr.gname, 32);
	hdr.devmajor = parseOctal(rawHdr.devmajor, 32);
	hdr.devminor = parseOctal(rawHdr.devminor, 32);
	memcpy(hdr.prefix, rawHdr.prefix, 155);

	return true;
}

bool
ParseTarFile(
	const std::vector<uint8_t>& tar,
	std::vector<TarEntry>& entries) {
	if (!tar.data() || tar.size() < sizeof(PosixHeader))
		return false;

	uint64_t offset{ 0 };
	const uint8_t* const base{ tar.data() };
	
	while (offset + 512 <= tar.size()) {
		const uint8_t* const block{ base + offset };
		bool isZero{ true };

		for (size_t i{ 0 }; i < 512; ++i) {
			if (block[i] != 0) {
				isZero = false;
				break;
			}
		}
		if (isZero)
			break;

		TarEntry entry{};

		if (!parseTarHeader(block, entry.header))
			return false;

		const uint64_t size{ entry.header.size };
		const uint64_t remaining{ tar.size() - offset - 512 };

		if (size == ~static_cast<uint64_t>(0) || size > remaining)
			return false;

		entry.offset = offset;
		entries.push_back(entry);

		offset += 512 + ((size + 511) & ~static_cast<uint64_t>(511));
	}

	return true;
}

void
LogTarEntries(
	const std::vector<TarEntry>& entries) {
	printf("Loaded %llu entries in .tar\n", entries.size());

	for (size_t i{ 0 };i < entries.size();++i) {
		const TarEntry& e{ entries[i] };
		printf("%s\n", e.header.name);
		printf("\tOffset=%llu\n", e.offset);
		printf("\tSize=%llu\n", e.header.size);
		printf("\n");
	}
}