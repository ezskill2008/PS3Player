#pragma once
#include <libPs3Core/Include/LibCore.h>

#include <vector>

struct MemRange {
	uint32_t start{ 0 };
	uint32_t end{ 0 };
	bool     exec{ false };
};

struct GuestMem {
	static constexpr uint64_t	kSize{ 1ull << 32 };
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
		const uint64_t end{ uint64_t(addr) + size };
		if (end > kSize) return false;
		for (const MemRange& r : ranges) if (addr < r.end && end > r.start) return false;

		const uint64_t s{ addr & ~0xFFFull };
		const uint64_t e{ (end + 0xFFF) & ~0xFFFull };
		if (!VirtualAlloc(base + s, size_t(e - s), MEM_COMMIT, PAGE_READWRITE)) return false;
		ranges.push_back({ addr, uint32_t(end), exec });

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
	
	uint32_t Read32(
		uint32_t addr) const {
		const uint8_t* p{ base + addr };
		
		return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
	}

	void Write32(
		uint32_t addr, 
		uint32_t v) {
		uint8_t* p{ base + addr };
		p[0] = uint8_t(v >> 24); p[1] = uint8_t(v >> 16); p[2] = uint8_t(v >> 8); p[3] = uint8_t(v);
	}
};