#pragma once
#include <stdint.h>
#include <vector>

//Source: https://www.gnu.org/software/tar/manual/html%5fnode/Standard.html

#define TMAGIC   "ustar "
#define TMAGLEN  6
#define TVERSION "00"
#define TVERSLEN 2

enum class TarTypeFlag {
	REGTYPE = '0',          // regular file
	AREGTYPE = '\0',        // regular file
	LNKTYPE = '1',          // link
	SYMTYPE = '2',          // symbolic link
	CHRTYPE = '3',          // character special
	BLKTYPE = '4',          // block special
	DIRTYPE = '5',          // directory
	FIFOTYPE = '6',         // FIFO special
	CONTTYPE = '7',         // contiguous; treated as a regular file
	XHDTYPE = 'x',          // Extended header referring to the next file in the archive
	XGLTYPE = 'g'           // Global extended header
};

enum class TarMode {
	TSUID = 04000,			// set UID on execution
	TSGID = 02000,			// set GID on execution
	TSVTX = 01000,			// reserved
							// file permissions
	TUREAD = 00400,			// read by owner
	TUWRITE = 00200,        // write by owner
	TUEXEC = 00100,			// execute/search by owner
	TGREAD = 00040,			// read by group
	TGWRITE = 00020,        // write by group
	TGEXEC = 00010,			// execute/search by group
	TOREAD = 00004,			// read by other
	TOWRITE = 00002,        // write by other
	TOEXEC = 00001			// execute/search by other
};

typedef struct TarHeader {
	char				name[100];
	uint32_t			mode;
	uint32_t			uid;
	uint32_t			gid;
	uint64_t			size;
	uint64_t			mtime;
	uint32_t			chksum;
	char				typeflag;
	char				linkname[100];
	char				magic[6];
	uint8_t				version;
	char				uname[32];
	char				gname[32];
	uint64_t			devmajor;
	uint64_t			devminor;
	char				prefix[155];
} TarHeader;

typedef struct TarEntry {
	TarHeader			header{};
	uint64_t			offset{};
} TarEntry;

bool ParseTarFile(
	const std::vector<uint8_t>& tar,
	std::vector<TarEntry>& entries);

void LogTarEntries(
	const std::vector<TarEntry>& entries);
