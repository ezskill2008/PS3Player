#include <libPs3Decrypt/Include/DecryptSELF.h>
#include <libPs3Decrypt/Src/ReadFile.h>

#include <synchapi.h>
#include <vector>
#include <string>
#include <sstream>
#include <zlib.h>

static constexpr uint32_t kSectTypePhdr{ 2 };
static constexpr uint32_t kSectEncrypted{ 3 };
static constexpr uint32_t kSectCompressed{ 2 };
static constexpr uint32_t kEhdrSize{ 64 };
static constexpr uint32_t kPhdrSize{ 56 };
static constexpr uint64_t kMaxElf{ 1ull << 31 };
static const uint8_t kElfMagic[4]{ 0x7F, 'E', 'L', 'F' };

struct SelfKeyEntry {
	uint32_t	selfType{ 0 };
	uint16_t	keyRevision{ 0 };
	uint8_t		erk[32]{};
	uint8_t		riv[16]{};
};

struct SectionHdr {
	uint64_t	offset{ 0 };
	uint64_t	size{ 0 };
	uint32_t	type{ 0 };
	uint32_t	index{ 0 };
	uint32_t	encrypted{ 0 };
	uint32_t	keyIndex{ 0 };
	uint32_t	ivIndex{ 0 };
	uint32_t	compressed{ 0 };
};

struct PhdrInfo {
	uint64_t	offset{ 0 };
	uint64_t	filesz{ 0 };
};

struct SelfPlan {
	std::vector<SectionHdr> sections;
	std::vector<PhdrInfo>	phdrs;
	std::vector<uint8_t>	keys;
	uint64_t				elfOffset{ 0 };
	uint64_t				phdrOffset{ 0 };
	uint64_t				shdrOffset{ 0 };
	uint64_t				ePhoff{ 0 };
	uint64_t				eShoff{ 0 };
	uint16_t				ePhnum{ 0 };
	uint16_t				eShnum{ 0 };
	uint16_t				eShentsize{ 0 };
	uint64_t				outSize{ 0 };
};

struct SelfKeys_T {
	SRWLOCK                   keyLock{ SRWLOCK_INIT };
	std::vector<SelfKeyEntry> keys;
};

bool
inRange(uint64_t off, uint64_t len, uint64_t total) {
	return off <= total && len <= total - off;
}

bool
isZero(const uint8_t* p, size_t n) {
	for (size_t i{ 0 }; i < n; ++i) if (p[i] != 0) return false;
	return true;
}

bool
parseHex(const std::string& s, uint8_t* out, size_t bytes) {
	if (s.size() != bytes * 2) return false;
	for (size_t i{ 0 }; i < bytes; ++i) {
		const int hi{ hexVal(s[i * 2]) };
		const int lo{ hexVal(s[i * 2 + 1]) };
		if (hi < 0 || lo < 0) return false;
		out[i] = uint8_t((hi << 4) | lo);
	}

	return true;
}

struct aesProv {
	BCRYPT_ALG_HANDLE cbc{ nullptr };
	BCRYPT_ALG_HANDLE ecb{ nullptr };

	aesProv() = default;
	aesProv(const aesProv&) = delete;
	aesProv& operator=(const aesProv&) = delete;
	~aesProv() {
		if (cbc) BCryptCloseAlgorithmProvider(cbc, 0);
		if (ecb) BCryptCloseAlgorithmProvider(ecb, 0);
	}

	bool open() {
		if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&cbc, BCRYPT_AES_ALGORITHM, nullptr, 0))) return false;
		if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&ecb, BCRYPT_AES_ALGORITHM, nullptr, 0))) return false;
		if (!BCRYPT_SUCCESS(BCryptSetProperty(cbc, BCRYPT_CHAINING_MODE,
			(PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0))) return false;
		if (!BCRYPT_SUCCESS(BCryptSetProperty(ecb, BCRYPT_CHAINING_MODE,
			(PUCHAR)BCRYPT_CHAIN_MODE_ECB, sizeof(BCRYPT_CHAIN_MODE_ECB), 0))) return false;
		return true;
	}
};

bool
aesCbcDecrypt(const aesProv& p, const uint8_t* key, ULONG keyLen,
	const uint8_t* iv, const uint8_t* in, uint8_t* out, ULONG len) {
	BCRYPT_KEY_HANDLE k{ nullptr };
	if (!BCRYPT_SUCCESS(BCryptGenerateSymmetricKey(p.cbc, &k, nullptr, 0,
		const_cast<PUCHAR>(key), keyLen, 0))) return false;

	uint8_t ivCopy[16]{};
	memcpy(ivCopy, iv, 16);
	ULONG got{ 0 };
	const NTSTATUS st{ BCryptDecrypt(k, const_cast<PUCHAR>(in), len, nullptr,
		ivCopy, 16, out, len, &got, 0) };
	BCryptDestroyKey(k);

	return BCRYPT_SUCCESS(st) && got == len;
}

bool
aesCtr(const aesProv& p, const uint8_t* key, const uint8_t* iv, uint8_t* data, size_t size) {
	BCRYPT_KEY_HANDLE k{ nullptr };
	if (!BCRYPT_SUCCESS(BCryptGenerateSymmetricKey(p.ecb, &k, nullptr, 0,
		const_cast<PUCHAR>(key), 16, 0))) return false;

	constexpr size_t kBlocks{ 4096 };
	std::vector<uint8_t> ctrBuf(kBlocks * 16);
	std::vector<uint8_t> stream(kBlocks * 16);

	uint8_t counter[16]{};
	memcpy(counter, iv, 16);

	bool ok{ true };
	size_t pos{ 0 };
	while (pos < size && ok) {
		const size_t remain{ size - pos };
		const size_t blocks{ (std::min)((remain + 15) / 16, kBlocks) };

		for (size_t b{ 0 }; b < blocks; ++b) {
			memcpy(&ctrBuf[b * 16], counter, 16);
			for (int i{ 15 }; i >= 0; --i)
				if (++counter[i] != 0) break;
		}

		ULONG got{ 0 };
		if (!BCRYPT_SUCCESS(BCryptEncrypt(k, ctrBuf.data(), ULONG(blocks * 16), nullptr,
			nullptr, 0, stream.data(), ULONG(blocks * 16), &got, 0))) {
			ok = false; break;
		}

		const size_t n{ (std::min)(remain, blocks * 16) };
		for (size_t i{ 0 }; i < n; ++i) data[pos + i] ^= stream[i];
		pos += n;
	}

	BCryptDestroyKey(k);
	
	return ok;
}

bool
parseSelf(SelfKeys keys, const uint8_t* s, size_t size, const aesProv& aes, SelfPlan& plan) {
	// --- SCE header (big-endian)
	if (size < 0x70) return false;
	if (Be32(s) != 0x53434500u) return false;
	const uint16_t keyRev{ Be16(s + 0x08) };
	const uint16_t category{ Be16(s + 0x0A) };
	const uint32_t metaOff{ Be32(s + 0x0C) };
	const uint64_t headerLen{ Be64(s + 0x10) };
	if (category != 1) return false;
	if (headerLen > size) return false;

	// --- SELF extended header at 0x20
	const uint64_t appInfoOff{ Be64(s + 0x28) };
	plan.elfOffset = Be64(s + 0x30);
	plan.phdrOffset = Be64(s + 0x38);
	plan.shdrOffset = Be64(s + 0x40);

	if (!inRange(appInfoOff, 0x20, size)) return false;
	const uint32_t selfType{ Be32(s + appInfoOff + 12) };

	// --- metadata info: 64 bytes, AES-256-CBC with the per-type/revision erk + riv
	const uint64_t infoPos{ 0x20 + uint64_t(metaOff) };
	if (!inRange(infoPos, 0x40, headerLen)) return false;

	uint8_t metaKey[16]{};
	uint8_t metaIv[16]{};
	bool    keyFound{ false };
	bool    rowSeen{ false };

	AcquireSRWLockShared(&keys->keyLock);
	for (const SelfKeyEntry& e : keys->keys) {
		if (e.selfType != selfType || e.keyRevision != keyRev) continue;
		rowSeen = true;

		uint8_t info[0x40]{};
		if (!aesCbcDecrypt(aes, e.erk, 32, e.riv, s + infoPos, info, 0x40)) continue;

		// layout: key[16], key_pad[16], iv[16], iv_pad[16]; both pads must decrypt to zero
		if (isZero(info + 0x10, 0x10) && isZero(info + 0x30, 0x10)) {
			memcpy(metaKey, info, 16);
			memcpy(metaIv, info + 0x20, 16);
			keyFound = true;
			break;
		}
	}
	ReleaseSRWLockShared(&keys->keyLock);
	if (!keyFound)
		return false;

	// --- metadata header + section headers + key array: AES-128-CTR
	const uint64_t regionPos{ infoPos + 0x40 };
	if (regionPos + 0x20 > headerLen) return false;
	std::vector<uint8_t> meta(size_t(headerLen - regionPos));
	memcpy(meta.data(), s + regionPos, meta.size());
	if (!aesCtr(aes, metaKey, metaIv, meta.data(), meta.size())) return false;

	const uint32_t sectionCount{ Be32(meta.data() + 0x0C) };
	const uint32_t keyCount{ Be32(meta.data() + 0x10) };
	if (sectionCount > 0x1000 || keyCount > 0x1000) return false;
	const size_t need{ 0x20 + size_t(sectionCount) * 0x30 + size_t(keyCount) * 16 };
	if (need > meta.size()) return false;

	const uint8_t* keyBase{ meta.data() + 0x20 + size_t(sectionCount) * 0x30 };
	plan.keys.assign(keyBase, keyBase + size_t(keyCount) * 16);

	// --- embedded ELF header, program headers, section headers (plain data in the SELF)
	if (!inRange(plan.elfOffset, kEhdrSize, size)) return false;
	const uint8_t* eh{ s + plan.elfOffset };
	if (memcmp(eh, kElfMagic, 4) != 0 || eh[4] != 2 || eh[5] != 2)
		return false;

	plan.ePhoff = Be64(eh + 0x20);
	plan.eShoff = Be64(eh + 0x28);
	const uint16_t phentsize{ Be16(eh + 0x36) };
	plan.ePhnum = Be16(eh + 0x38);
	plan.eShentsize = Be16(eh + 0x3A);
	plan.eShnum = Be16(eh + 0x3C);

	if (phentsize != kPhdrSize) return false;
	if (!inRange(plan.phdrOffset, uint64_t(plan.ePhnum) * kPhdrSize, size)) return false;
	const bool hasShdrs{ plan.eShnum != 0 && plan.shdrOffset != 0 };
	if (hasShdrs && !inRange(plan.shdrOffset, uint64_t(plan.eShnum) * plan.eShentsize, size))
		return false;

	for (uint16_t i{ 0 }; i < plan.ePhnum; ++i) {
		const uint8_t* ph{ s + plan.phdrOffset + size_t(i) * kPhdrSize };
		PhdrInfo info{};
		info.offset = Be64(ph + 0x08);          // p_offset
		info.filesz = Be64(ph + 0x20);          // p_filesz
		plan.phdrs.push_back(info);
	}

	// --- output size: headers plus every program header's data
	uint64_t out{ kEhdrSize };
	auto grow{ [&](uint64_t off, uint64_t len) {
		if (off > kMaxElf || len > kMaxElf) return false;
		out = (std::max)(out, off + len);
		return out <= kMaxElf;
	} };

	if (!grow(plan.ePhoff, uint64_t(plan.ePhnum) * kPhdrSize)) return false;
	if (hasShdrs && !grow(plan.eShoff, uint64_t(plan.eShnum) * plan.eShentsize)) return false;

	// --- section headers (48 bytes each)
	for (uint32_t i{ 0 }; i < sectionCount; ++i) {
		const uint8_t* p{ meta.data() + 0x20 + size_t(i) * 0x30 };
		SectionHdr h{};
		h.offset = Be64(p + 0);
		h.size = Be64(p + 8);
		h.type = Be32(p + 16);
		h.index = Be32(p + 20);
		// p + 24: hashed, p + 28: sha1 index (unused, we skip hash checks)
		h.encrypted = Be32(p + 32);
		h.keyIndex = Be32(p + 36);
		h.ivIndex = Be32(p + 40);
		h.compressed = Be32(p + 44);
		plan.sections.push_back(h);

		if (h.type != kSectTypePhdr) continue;       // only program header data is rebuilt
		if (h.index >= plan.ePhnum)       return false;
		if (!inRange(h.offset, h.size, size)) return false;
		if (h.encrypted == kSectEncrypted &&
			(uint64_t(std::max(h.keyIndex, h.ivIndex)) + 1) * 16 > plan.keys.size())
			return false;

		const PhdrInfo& ph{ plan.phdrs[h.index] };
		const uint64_t sz{ h.compressed == kSectCompressed ? ph.filesz : h.size };
		if (!grow(ph.offset, sz)) return false;
	}

	plan.outSize = out;

	return true;
}

bool
buildElf(const uint8_t* s, const aesProv& aes, const SelfPlan& plan, uint8_t* out) {
	memset(out, 0, size_t(plan.outSize));
	memcpy(out, s + plan.elfOffset, kEhdrSize);
	memcpy(out + plan.ePhoff, s + plan.phdrOffset, size_t(plan.ePhnum) * kPhdrSize);
	if (plan.eShnum != 0 && plan.shdrOffset != 0)
		memcpy(out + plan.eShoff, s + plan.shdrOffset, size_t(plan.eShnum) * plan.eShentsize);

	std::vector<uint8_t> packed;
	for (const SectionHdr& h : plan.sections) {
		if (h.type != kSectTypePhdr) continue;

		const PhdrInfo& ph{ plan.phdrs[h.index] };
		uint8_t* dst{ out + ph.offset };
		const bool enc{ h.encrypted == kSectEncrypted };
		const uint8_t* key{ enc ? &plan.keys[size_t(h.keyIndex) * 16] : nullptr };
		const uint8_t* iv{ enc ? &plan.keys[size_t(h.ivIndex) * 16] : nullptr };

		if (h.compressed == kSectCompressed) {
			packed.assign(s + h.offset, s + h.offset + h.size);
			if (enc && !aesCtr(aes, key, iv, packed.data(), packed.size())) return false;

			uLongf outLen{ static_cast<uLongf>(ph.filesz) };
			if (uncompress(dst, &outLen, packed.data(), static_cast<uLong>(packed.size())) != Z_OK ||
				outLen != ph.filesz)
				return false;
		}
		else {
			memcpy(dst, s + h.offset, size_t(h.size));
			if (enc && !aesCtr(aes, key, iv, dst, size_t(h.size))) return false;
		}
	}
	return true;
}

bool
LpsLoadSelfKeys(
	const char* keyFile,
	SelfKeys& keys) {
	if (!keyFile) return false;
	if (keys) LpsReleaseSelfKeys(keys);

	HANDLE h{ openForRead(keyFile) };
	if (h == INVALID_HANDLE_VALUE) return false;

	std::string text;
	LARGE_INTEGER sz{};
	
	if (GetFileSizeEx(h, &sz) && sz.QuadPart > 0 && sz.QuadPart <= (1 << 20)) {
		text.resize(size_t(sz.QuadPart));
		DWORD got{ 0 };
		if (!ReadFile(h, text.data(), DWORD(text.size()), &got, nullptr) || got != text.size())
			text.clear();
	}
	
	CloseHandle(h);
	if (text.empty()) return false;

	std::vector<SelfKeyEntry> table;
	std::istringstream in{ text };
	std::string line;

	while (std::getline(in, line)) {
		const size_t hash{ line.find('#') };
		if (hash != std::string::npos) line.resize(hash);

		std::istringstream ls{ line };
		std::string type, rev, erk, riv;
		if (!(ls >> type)) continue;
		if (!(ls >> rev >> erk >> riv)) return false;

		SelfKeyEntry e{};
		e.selfType = uint32_t(strtoul(type.c_str(), nullptr, 0));
		e.keyRevision = uint16_t(strtoul(rev.c_str(), nullptr, 0));
		if (!parseHex(erk, e.erk, 32) || !parseHex(riv, e.riv, 16))
			return false;
		table.push_back(e);
	}

	if (table.empty()) return false;

	keys = new NOTHROW SelfKeys_T();
	if (!keys) return false;

	AcquireSRWLockExclusive(&keys->keyLock);
	keys->keys.swap(table);
	ReleaseSRWLockExclusive(&keys->keyLock);

	return true;
}

void
LpsReleaseSelfKeys(
	SelfKeys& keys) {
	if (!keys) return;

	delete keys;
}

bool
LpsSelfToElf(
	SelfKeys keys,
	const void* selfIn, 
	size_t selfSize,
	void* elfOut, 
	size_t elfOutCapacity, 
	size_t& elfSize) {
	elfSize = 0;
	const uint8_t* self{ static_cast<const uint8_t*>(selfIn) };
	if (!self || selfSize < 4) return false;

	if (memcmp(self, kElfMagic, 4) == 0) {
		elfSize = selfSize;
		if (!elfOut) return true;
		if (elfOutCapacity < selfSize) return false;
		memcpy(elfOut, self, selfSize);
		return true;
	}

	aesProv aes;
	if (!aes.open()) return false;

	SelfPlan plan;
	if (!parseSelf(keys, self, selfSize, aes, plan)) return false;

	elfSize = size_t(plan.outSize);
	if (!elfOut) return true;
	if (elfOutCapacity < elfSize) return false;
	
	return buildElf(self, aes, plan, static_cast<uint8_t*>(elfOut));
}
