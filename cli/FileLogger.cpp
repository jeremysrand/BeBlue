#include <errno.h>

#include "cli/FileLogger.h"


// Implementation

FileLogger::FileLogger(const char * path) :
	Logger(),
	file(fopen(path, "w")),
	status(errno)
{
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
}