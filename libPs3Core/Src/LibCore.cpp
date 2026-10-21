#include <libPs3Core/Include/CoreTypes.h>
#include <libPs3Core/Include/LibCore.h>

#include <stdio.h>

void
LcLogError(
	const char* file,
	const char* function,
	size_t line) {
	printf("An error occured at: %s:%u %s\n", file, (int)line, function);
}
