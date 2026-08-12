#include <errno.h>

#include "cli/FileLogger.h"

#include "VersionStr.h"


// Implementation

FileLogger::FileLogger(const char * path) :
	Logger(),
	file(fopen(path, "w")),
	status(errno)
{
	if (file != NULL) {
		char timestamp[LOGGER_TIMESTAMP_LEN];
		FormatTimestamp(timestamp);
		fprintf(file, "%s  %s\n", timestamp, VERSION_DETAIL);
	}
}


FileLogger::~FileLogger()
{
	if (file != NULL)
		fclose(file);
}


void FileLogger::Log(const char * timestamp, const char * fmt, va_list ap)
{
	if (file == NULL)
		return;
		
	fprintf(file, "%s  ", timestamp);
	vfprintf(file, fmt, ap);
	fprintf(file, "\n");
	fflush(file);
}