#pragma once
#include <libPs3Core/Include/LibCore.h>

#include <vector>

struct MemRange {
	uint32_t start{ 0 };
	uint64_t end{ 0 };
	bool     exec{ false };
};

struct GuestMem {
	static constexpr uint64_t	kSize{ 1ull << 32 };
	static constexpr uint64_t	kPageSize{ 0x1000 };
	static constexpr uint64_t	kPageMask{ kPageSize - 1 };
	uint8_t*					base{ nullptr };
	std::vector<MemRange>		ranges;

	bool Init() {
		base = (uint8_t*)VirtualAlloc(
			nullptr,
			kSize,
			MEM_RESERVE,
			PAGE_NOACCESS);

		return base;
	}

	void Release() {
		if (base) {
			VirtualFree(base, 0, MEM_RELEASE);
			base = nullptr;
		}

		ranges = {};
	}

	bool Map(
		uint32_t addr,
		uint64_t size,
		bool exec) {
		if (!base) { printf("Map: Init() not called\n"); return false; }
		if (size == 0) return true;

		const uint64_t s{ uint64_t(addr) & ~kPageMask };
		const uint64_t e{ (uint64_t(addr) + size + kPageMask) & ~kPageMask };

		if (e > kSize) { printf("Map: range 0x%llx-0x%llx exceeds guest space\n", s, e); return false; }

		for (const MemRange& r : ranges) {
			if (s < r.end && e > r.start) {
				printf("Map: range 0x%llx-0x%llx overlaps existing 0x%llx-0x%llx\n",
					s, e, uint64_t(r.start), uint64_t(r.end));
				return false;
			}
		}

		if (!VirtualAlloc(base + s, size_t(e - s), MEM_COMMIT, PAGE_READWRITE)) {
			printf("Map: VirtualAlloc failed (error %lu)\n", GetLastError());
			return false;
		}

		ranges.push_back({ uint32_t(s), e, exec });
		return true;
	}

	bool IsMapped(
		uint32_t addr,
		uint32_t size) const {
		const uint64_t end{ uint64_t(addr) + size };
		for (const MemRange& r : ranges)
			if (addr >= r.start && end <= r.end) return true;

		return false;
	}

	bool IsExecutable(
		uint32_t addr) const {
		for (const MemRange& r : ranges)
			if (addr >= r.start && addr < r.end) return r.exec;

		return false;
	}

	uint8_t Read8(
		uint32_t addr) const {
		const uint8_t* p{ base + addr };
#if _DEBUG
		if (!IsMapped(addr, sizeof(uint8_t))) {
			printf("Invalid read/write! 0x%llx\n", static_cast<uint64_t>(addr));
			return 0;
		}
#endif
		return p[0];
	}

	uint16_t Read16(
		uint32_t addr) const {
		const uint8_t* p{ base + addr };
#if _DEBUG
		if (!IsMapped(addr, sizeof(uint16_t))) {
			printf("Invalid read/write! 0x%llx\n", static_cast<uint64_t>(addr));
			return 0;
		}
#endif
		return uint16_t((uint16_t(p[0]) << 8) | p[1]);
	}

	uint32_t Read32(
		uint32_t addr) const {
		const uint8_t* p{ base + addr };
#if _DEBUG
		if (!IsMapped(addr, sizeof(uint32_t))) {
			printf("Invalid read/write! 0x%llx\n", static_cast<uint64_t>(addr));
			return 0;
		}
#endif
		return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
	}

	uint64_t Read64(
		uint32_t addr) const {
		const uint8_t* p{ base + addr };
#if _DEBUG
		if (!IsMapped(addr, sizeof(uint64_t))) {
			printf("Invalid read/write! 0x%llx\n", static_cast<uint64_t>(addr));
			return 0;
		}
#endif
		return (uint64_t(p[0]) << 56) | (uint64_t(p[1]) << 48) | (uint64_t(p[2]) << 40) | (uint64_t(p[3]) << 32)
			| (uint64_t(p[4]) << 24) | (uint64_t(p[5]) << 16) | (uint64_t(p[6]) << 8) | p[7];
	}

	void Write8(
		uint32_t addr,
		uint8_t v) {
		uint8_t* p{ base + addr };
#if _DEBUG
		if (!IsMapped(addr, sizeof(uint8_t))) {
			printf("Invalid read/write! 0x%llx\n", static_cast<uint64_t>(addr));
			return;
		}
#endif
		p[0] = v;
	}

	void Write16(
		uint32_t addr,
		uint16_t v) {
		uint8_t* p{ base + addr };
#if _DEBUG
		if (!IsMapped(addr, sizeof(uint16_t))) {
			printf("Invalid read/write! 0x%llx\n", static_cast<uint64_t>(addr));
			return;
		}
#endif
		p[0] = uint8_t(v >> 8); p[1] = uint8_t(v);
	}

	void Write32(
		uint32_t addr,
		uint32_t v) {
		uint8_t* p{ base + addr };
#if _DEBUG
		if (!IsMapped(addr, sizeof(uint32_t))) {
			printf("Invalid read/write! 0x%llx\n", static_cast<uint64_t>(addr));
			return;
		}
#endif
		p[0] = uint8_t(v >> 24); p[1] = uint8_t(v >> 16); p[2] = uint8_t(v >> 8); p[3] = uint8_t(v);
	}

	void Write64(
		uint32_t addr,
		uint64_t v) {
		uint8_t* p{ base + addr };
#if _DEBUG
		if (!IsMapped(addr, sizeof(uint64_t))) {
			printf("Invalid read/write! 0x%llx\n", static_cast<uint64_t>(addr));
			return;
		}
#endif
		p[0] = uint8_t(v >> 56); p[1] = uint8_t(v >> 48); p[2] = uint8_t(v >> 40); p[3] = uint8_t(v >> 32);
		p[4] = uint8_t(v >> 24); p[5] = uint8_t(v >> 16); p[6] = uint8_t(v >> 8);  p[7] = uint8_t(v);
	}
};
