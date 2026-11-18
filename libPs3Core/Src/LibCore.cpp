#include <libPs3Core/Include/CoreTypes.h>
#include <libPs3Core/Include/LibCore.h>

#include <stdio.h>
#include <fstream>
#include <filesystem>

void
LcLogError(
	const char* file,
	const char* function,
	size_t line) {
	printf("An error occured at: %s:%u %s\n", file, (int)line, function);
}

bool
LcWriteFile(
	const char* file,
	const uint8_t* const data,
	size_t length) {
	if (!(file && data && length))
		return false;

	const std::filesystem::path p{ file };

	if (!std::filesystem::exists(p.parent_path())) {
		std::filesystem::create_directories(p.parent_path());
	}

	std::ofstream ofile{ file, std::ios::out | std::ios::binary };
	ofile.write((char*)data, length);
	ofile.flush();

	return true;
}
