#pragma once
#include <libPs3Core/Include/LibCore.h>

#define ISO_KEY_SIZE 16
#define ISO_SECTOR_SIZE 2048

struct CryptKey {
	uint8_t		bytes[ISO_KEY_SIZE]{};
};

struct IsoFileInfo {
	uint64_t	size;
	uint32_t	lba;
	bool		isDirectory;
};

typedef struct IsoDisk_T* IsoDisk;
typedef struct IsoFile_T* IsoFile;

typedef bool (*LpsDirCallback)(const char* name, const IsoFileInfo* info, void* user);

LPS_API bool LpsReadKey(
	const char* keyFile,
	CryptKey& key);

LPS_API bool LpsLoadDisk(
	const char* path,
	IsoDisk& disk);

LPS_API void LpsReleaseDisk(
	IsoDisk& disk);

LPS_API bool LpsReadSectors(
	IsoDisk disk,
	uint32_t lba,
	uint32_t count,
	void* out);

LPS_API bool LpsSetDiskKey(
	IsoDisk disk,
	const CryptKey& key);

LPS_API bool LpsMountFileSys(
	IsoDisk disk);

LPS_API bool LpsStat(
	IsoDisk disk,
	const char* path,
	IsoFileInfo& info);

LPS_API bool LpsOpenFile(
	IsoDisk disk, 
	const char* path, 
	IsoFile& file);

LPS_API void LpsCloseFile(
	IsoFile& file);

LPS_API uint64_t LpsFileSize(
	IsoFile file);

LPS_API size_t LpsReadFile(
	IsoFile file, 
	uint64_t offset, 
	void* dst, 
	size_t size);

LPS_API bool LpsListDir(
	IsoDisk disk, 
	const char* path, 
	LpsDirCallback cb, 
	void* user);
