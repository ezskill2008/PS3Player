#pragma once
#include <libPs3Core/Include/LibCore.h>

#define ISO_KEY_SIZE 16
#define ISO_SECTOR_SIZE 2048

struct CryptKey {
	uint8_t		bytes[ISO_KEY_SIZE]{};
};

typedef struct IsoDisk_T* IsoDisk;

LPS_API bool LpsReadKey(
	const char* keyFile,
	CryptKey& key);

LPS_API bool LpsLoadDisk(
	const char* path,
	IsoDisk& disk);

LPS_API void LpsReleaseDisk(
	IsoDisk& disk);
