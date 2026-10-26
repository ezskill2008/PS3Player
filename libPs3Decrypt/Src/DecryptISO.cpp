#include <libPs3Decrypt/Include/DecryptISO.h>

#include <Windows.h>
#include <filesystem>
#include <bcrypt.h>
#include <unordered_map>

#pragma comment(lib, "libPs3Core.lib")
#pragma comment(lib, "bcrypt.lib")

struct DiskRegion {
	uint32_t			start{};
	uint32_t			end{};
	bool				encrypted{};
};

struct DirEntry {
	uint32_t			lba;
	uint32_t			size;
	bool				isDir;
	char				name[256];
};

struct DirNode {
	std::vector<DirEntry>					entries;
	std::unordered_map<std::string, size_t>	byName;
};

struct IsoDisk_T {
	HANDLE				file{};
	size_t				fileSize{};
	uint32_t			sectorCount{};
	uint32_t			regionCount{};
	bool				hasKey{ false };
	uint32_t			rootLba{ 0 };
	uint32_t			rootSize{ 0 };
	DiskRegion			regions[256]{};
	BCRYPT_ALG_HANDLE	alg{ nullptr };
	BCRYPT_KEY_HANDLE	key{ nullptr };
	SRWLOCK				cacheLock{ SRWLOCK_INIT };
	std::unordered_map<uint32_t, std::shared_ptr<DirNode>>	dirCache;
};

struct IsoFile_T {
	IsoDisk				disk{ nullptr };
	uint32_t			lba{ 0 };
	uint64_t			size{ 0 };
};

uint32_t
be32(const uint8_t* s) {
	return (uint32_t(s[0]) << 24) | (uint32_t(s[1]) << 16) |
		(uint32_t(s[2]) << 8) | uint32_t(s[3]);
}

uint32_t
le32(const uint8_t* p) {
	return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

std::string
upperCase(const char* s) {
	std::string out{ s };
	for (char& c : out) {
		if (c >= 'a' && c <= 'z')
			c = char(c - 'a' + 'A');
	}

	return out;
}

bool
readAt(HANDLE h, uint64_t offset, void* dst, size_t size) {
	uint8_t* p = static_cast<uint8_t*>(dst);
	while (size > 0) {
		const DWORD chunk{ size > 0x40000000u ? 0x40000000u : static_cast<DWORD>(size) };
		OVERLAPPED ov{};
		ov.Offset = static_cast<DWORD>(offset & 0xFFFFFFFFu);
		ov.OffsetHigh = static_cast<DWORD>(offset >> 32);

		DWORD got{ 0 };
		if (!ReadFile(h, p, chunk, &got, &ov) || got == 0)
			return false;

		p += got;
		offset += got;
		size -= got;
	}
	
	return true;
}

HANDLE
openForRead(const char* path) {
	const int n{ MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0) };
	if (n <= 0) return INVALID_HANDLE_VALUE;
	wchar_t* wide{ new NOTHROW wchar_t[n] };
	if (!wide) return INVALID_HANDLE_VALUE;
	MultiByteToWideChar(CP_UTF8, 0, path, -1, wide, n);

	HANDLE h{ CreateFileW(wide, GENERIC_READ, FILE_SHARE_READ, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, nullptr) };
	delete[] wide;

	return h;
}

int
hexVal(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

bool
isEncrypted(const IsoDisk_T* d, uint32_t lba) {
	for (uint32_t i{ 0 }; i < d->regionCount; ++i) {
		const DiskRegion& r{ d->regions[i] };
		if (lba >= r.start && lba < r.end)
			return r.encrypted;
	}

	return false;
}

bool
decryptSector(IsoDisk_T* d, uint32_t lba, uint8_t* sector) {
	uint8_t iv[16]{};
	iv[12] = uint8_t(lba >> 24);
	iv[13] = uint8_t(lba >> 16);
	iv[14] = uint8_t(lba >> 8);
	iv[15] = uint8_t(lba);

	ULONG outLen{ 0 };
	const NTSTATUS st{ BCryptDecrypt(d->key, sector, ISO_SECTOR_SIZE, nullptr,
		iv, sizeof(iv), sector, ISO_SECTOR_SIZE,
		&outLen, 0) };

	return BCRYPT_SUCCESS(st) && outLen == ISO_SECTOR_SIZE;
}

bool
validatePVD(uint8_t lba[ISO_SECTOR_SIZE]) {
	if (lba[0] != 1) return false;
	if (lba[1] != 'C') return false;
	if (lba[2] != 'D') return false;
	if (lba[3] != '0') return false;
	if (lba[4] != '0') return false;
	if (lba[5] != '1') return false;
	if (lba[6] != 1) return false;
	if (lba[128] != 0x00) return false;
	if (lba[129] != 0x08) return false;

	return true;
}

template <class F>
bool
walkDir(const uint8_t* buf, uint32_t size, F cb) {
	uint32_t pos{ 0 };
	
	while (pos < size) {
		const uint8_t len{ buf[pos] };
		
		if (len == 0) {                                  // padding to end of sector
			pos = (pos / ISO_SECTOR_SIZE + 1) * ISO_SECTOR_SIZE;
			continue;
		}

		if (pos + len > size || len < 34) return false;  // corrupt

		const uint8_t* r{ buf + pos };

		DirEntry e{};
		e.lba = le32(r + 2);
		e.size = le32(r + 10);
		e.isDir = (r[25] & 0x02) != 0;

		const uint8_t n{ r[32] };

		if (pos + 33 + n > size) return false;
		if (n == 1 && r[33] == 0) { e.name[0] = '.'; }
		else if (n == 1 && r[33] == 1) { e.name[0] = '.'; e.name[1] = '.'; }
		else {
			uint32_t o{ 0 };
			for (uint8_t i = 0; i < n && o < 255; ++i) {
				if (r[33 + i] == ';') break;             // strip ";1" version suffix
				e.name[o++] = char(r[33 + i]);
			}

			if (o > 0 && e.name[o - 1] == '.') --o;      // "FILE." -> "FILE"
			e.name[o] = 0;
		}

		if (!cb(e)) return true;
		
		pos += len;
	}

	return true;
}

std::shared_ptr<DirNode>
getDir(IsoDisk d, uint32_t lba, uint32_t size) {
	AcquireSRWLockShared(&d->cacheLock);
	auto it{ d->dirCache.find(lba) };
	
	if (it != d->dirCache.end()) {
		auto node{ it->second };
		ReleaseSRWLockShared(&d->cacheLock);
		return node;
	}

	ReleaseSRWLockShared(&d->cacheLock);

	const uint32_t sectors{ (size + ISO_SECTOR_SIZE - 1) / ISO_SECTOR_SIZE };
	std::vector<uint8_t> buf(size_t(sectors) * ISO_SECTOR_SIZE);
	if (!LpsReadSectors(d, lba, sectors, buf.data())) return nullptr;

	auto node{ std::make_shared<DirNode>() };
	if (!walkDir(buf.data(), size, [&](const DirEntry& e) {
		node->byName[upperCase(e.name)] = node->entries.size();
		node->entries.push_back(e);
		return true;
		})) return nullptr;

	AcquireSRWLockExclusive(&d->cacheLock);
	auto [pos, inserted] { d->dirCache.emplace(lba, node) };
	auto result{ pos->second };
	ReleaseSRWLockExclusive(&d->cacheLock);
	return result;
}

bool
lookup(IsoDisk d, const char* path, DirEntry& out) {
	DirEntry cur{};
	cur.lba = d->rootLba;
	cur.size = d->rootSize;
	cur.isDir = true;

	const char* p{ path };

	while (*p) {
		while (*p == '/') ++p;
		if (!*p) break;

		char   comp[256]{};
		size_t n{ 0 };
		while (*p && *p != '/' && n < 255) comp[n++] = *p++;
		if (*p && *p != '/') return false;

		if (!cur.isDir) return false;
		auto dir{ getDir(d, cur.lba, cur.size) };
		if (!dir) return false;

		auto it{ dir->byName.find(upperCase(comp)) };
		if (it == dir->byName.end()) return false;
		cur = dir->entries[it->second];
	}
	
	out = cur;

	return true;
}

bool
LpsReadKey(
	const char* keyFile,
	CryptKey& key) {
	HANDLE h{ openForRead(keyFile) };
	if (h == INVALID_HANDLE_VALUE) return false;

	LARGE_INTEGER sz{};
	uint8_t buf[64]{};
	bool ok = false;

	if (GetFileSizeEx(h, &sz) && sz.QuadPart >= ISO_KEY_SIZE && sz.QuadPart <= sizeof(buf)) {
		const DWORD len = static_cast<DWORD>(sz.QuadPart);
		DWORD got{ 0 };
		if (ReadFile(h, buf, len, &got, nullptr) && got == len) {
			if (len == ISO_KEY_SIZE) {
				memcpy(key.bytes, buf, ISO_KEY_SIZE);
				ok = true;
			}
			else {
				ok = true;
				for (int i = 0; i < ISO_KEY_SIZE && ok; ++i) {
					const int hi{ hexVal(char(buf[i * 2])) };
					const int lo{ hexVal(char(buf[i * 2 + 1])) };
					if (hi < 0 || lo < 0) ok = false;
					else key.bytes[i] = uint8_t((hi << 4) | lo);
				}
			}
		}
	}

	CloseHandle(h);
	
	return ok;
}

bool
LpsLoadDisk(
	const char* path,
	IsoDisk& disk) {
	disk = nullptr;

	IsoDisk_T* d{ new NOTHROW IsoDisk_T() };
	if (!d) return false;

	d->file = openForRead(path);
	if (d->file == INVALID_HANDLE_VALUE) { delete d; return false; }

	LARGE_INTEGER sz{};
	if (!GetFileSizeEx(d->file, &sz) || sz.QuadPart < ISO_SECTOR_SIZE) {
		CloseHandle(d->file); delete d; return false;
	}

	d->fileSize = static_cast<uint64_t>(sz.QuadPart);
	d->sectorCount = static_cast<uint32_t>(d->fileSize / ISO_SECTOR_SIZE);

	uint8_t sector0[ISO_SECTOR_SIZE];
	if (!readAt(d->file, 0, sector0, sizeof(sector0))) {
		CloseHandle(d->file); delete d; return false;
	}

	// All header fields are big-endian. A count of 0 (or an implausible one)
	// means sector 0 is the ISO9660 system area, i.e. an already decrypted image.
	const uint32_t count{ be32(sector0) };

	if (count >= 1 && count <= 255) {
		uint32_t e[512]{};
		for (uint32_t j{ 0 }; j < count * 2; ++j)
			e[j] = be32(&sector0[8 + j * 4]);

		uint32_t n{ 0 };
		bool valid{ true };
		for (uint32_t i{ 0 }; i < count && valid; ++i) {
			const uint32_t s{ e[2 * i] };
			const uint32_t en{ e[2 * i + 1] };
			if (en < s) { valid = false; break; }

			d->regions[n++] = { s, en + 1, false };

			if (i + 1 < count) {
				const uint32_t nextStart{ e[2 * i + 2] };
				if (nextStart <= en + 1) { valid = false; break; }
				d->regions[n++] = { en + 1, nextStart, true };
			}
		}
		d->regionCount = valid ? n : 0;
	}

	disk = d;

#if _DEBUG
	for (uint32_t i = 0; i < disk->regionCount; ++i)
		printf("region %u: %u..%u enc=%d\n", i,
			disk->regions[i].start, disk->regions[i].end,
			disk->regions[i].encrypted);

	uint8_t hdr[48];
	readAt(disk->file, 0, hdr, sizeof hdr);
	for (int i = 0; i < 48; i += 4)
		printf("%02x: %02x%02x%02x%02x\n", i, hdr[i], hdr[i + 1], hdr[i + 2], hdr[i + 3]);
#endif

	return true;
}

void
LpsReleaseDisk(
	IsoDisk& disk) {
	if (!disk) return;

	if (disk->file != INVALID_HANDLE_VALUE)
		CloseHandle(disk->file);
	if (disk->key) BCryptDestroyKey(disk->key);
	if (disk->alg) BCryptCloseAlgorithmProvider(disk->alg, 0);

	delete disk;
	disk = nullptr;
}

bool
LpsReadSectors(
	IsoDisk disk, 
	uint32_t lba, 
	uint32_t count, 
	void* out) {
	if (!disk || !out || uint64_t(lba) + count > disk->sectorCount) return false;

	if (!readAt(disk->file, uint64_t(lba) * ISO_SECTOR_SIZE, out,
		size_t(count) * ISO_SECTOR_SIZE))
		return false;

	if (disk->regionCount == 0) return true;

	uint8_t* p = static_cast<uint8_t*>(out);
	for (uint32_t i = 0; i < count; ++i, p += ISO_SECTOR_SIZE) {
		if (!isEncrypted(disk, lba + i)) continue;
		if (!disk->key) return false;
		if (!decryptSector(disk, lba + i, p)) return false;
	}
	return true;
}

bool
LpsSetDiskKey(
	IsoDisk disk,
	const CryptKey& key) {
	if (!disk) return false;

	if (disk->key) { BCryptDestroyKey(disk->key); disk->key = nullptr; }

	if (!disk->alg) {
		if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&disk->alg, BCRYPT_AES_ALGORITHM, nullptr, 0)))
			return false;
		if (!BCRYPT_SUCCESS(BCryptSetProperty(disk->alg, BCRYPT_CHAINING_MODE,
			(PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0)))
			return false;
	}

	disk->hasKey = true;

	return BCRYPT_SUCCESS(BCryptGenerateSymmetricKey(disk->alg, &disk->key, nullptr, 0,
		(PUCHAR)key.bytes, ISO_KEY_SIZE, 0));
}

bool
LpsMountFileSys(
	IsoDisk disk) {
	if (!disk) return false;

	uint8_t pvd[ISO_SECTOR_SIZE]{};
	if (!LpsReadSectors(disk, 16, 1, pvd)) return false;
	if (!validatePVD(pvd)) return false;

	disk->rootLba = le32(pvd + 156 + 2);
	disk->rootSize = le32(pvd + 156 + 10);
	
	return true;
}

bool
LpsStat(
	IsoDisk d, 
	const char* path, 
	IsoFileInfo& info) {
	DirEntry e{};
	if (!d || !path || !lookup(d, path, e)) return false;

	info = { e.size, e.lba, e.isDir };

	return true;
}

bool
LpsOpenFile(
	IsoDisk d, 
	const char* path, 
	IsoFile& file) {
	file = nullptr;
	DirEntry e{};
	if (!d || !path || !lookup(d, path, e) || e.isDir) return false;
	
	file = new NOTHROW IsoFile_T{ d, e.lba, e.size };
	
	return file != nullptr;
}

void
LpsCloseFile(
	IsoFile& file) {
	if(file) delete file;
	file = nullptr;
}

uint64_t
LpsFileSize(
	IsoFile file) {
	return file ? file->size : 0;
}

size_t
LpsReadFile(
	IsoFile f, 
	uint64_t offset, 
	void* dst, 
	size_t size) {
	if (!f || !dst || offset >= f->size) return 0;
	if (size > f->size - offset) size = size_t(f->size - offset);

	constexpr uint32_t kChunk{ 256 };
	std::vector<uint8_t> tmp(size_t(kChunk) * ISO_SECTOR_SIZE);
	uint8_t* out{ static_cast<uint8_t*>(dst) };
	size_t done{ 0 };

	while (done < size) {
		const uint64_t abs{ offset + done };
		const uint32_t sec{ uint32_t(abs / ISO_SECTOR_SIZE) };
		const uint32_t inSec{ uint32_t(abs % ISO_SECTOR_SIZE) };
		const size_t   want{ size - done };

		uint32_t count{ uint32_t((inSec + want + ISO_SECTOR_SIZE - 1) / ISO_SECTOR_SIZE) };
		if (count > kChunk) count = kChunk;

		if (!LpsReadSectors(f->disk, f->lba + sec, count, tmp.data())) break;

		const size_t avail{ size_t(count) * ISO_SECTOR_SIZE - inSec };
		const size_t take{ want < avail ? want : avail };
		memcpy(out + done, tmp.data() + inSec, take);
		done += take;
	}

	return done;
}

bool
LpsListDir(
	IsoDisk d, 
	const char* path, 
	LpsDirCallback cb, 
	void* user) {
	DirEntry e{};
	if (!d || !cb || !lookup(d, path, e) || !e.isDir) return false;
	auto dir{ getDir(d, e.lba, e.size) };
	if (!dir) return false;

	for (const DirEntry& c : dir->entries) {
		if (strcmp(c.name, ".") == 0 || strcmp(c.name, "..") == 0) continue;
		const IsoFileInfo info{ c.size, c.lba, c.isDir };
		if (!cb(c.name, &info, user)) break;
	}

	return true;
}
