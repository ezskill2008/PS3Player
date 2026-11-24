#pragma once
#include <libPs3Core/Include/LibCore.h>

#include <stdio.h>
#include <stdint.h>

#include <fstream>
#include <filesystem>

typedef struct ScePupHeader { // size is 0x30-bytes
	uint8_t			magic[7];
	uint8_t			formatFlag;
	uint64_t		packageVersion;
	uint64_t		imageVersion;
	uint64_t		segmentNum;
	uint64_t		fileOffset;
	uint64_t		fileSize;
} ScePupHeader;

typedef struct ScePupSegmentEntry { // size is 0x20-bytes
	uint64_t		id;
	uint64_t		offset;
	uint64_t		size;
	uint32_t		signAlgorithm;
	uint32_t		padding;
} ScePupSegmentEntry;

typedef struct ScePupDigestEntry { // size is 0x20-bytes
	uint64_t		segmentIndex;
	uint8_t			digest[0x14];
	uint32_t		padding;
} ScePupDigestEntry;

typedef struct {
	uint8_t			digest[0x14]; // HMAC-SHA1
} PUPHeaderDigest;
