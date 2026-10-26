#include <libPs3Decrypt/Src/ReadFile.h>

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
