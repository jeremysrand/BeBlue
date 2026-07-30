#include <stdio.h>

#include "common/Logger.h"


#ifndef CLI_FILE_LOGGER_H
#define CLI_FILE_LOGGER_H


// Interface

class FileLogger : public Logger
{
	public:
		FileLogger(const char * path);
		~FileLogger();
		
		status_t InitCheck();
		virtual void Log(const char * timestamp, const char * fmt, va_list ap);
		
	private:
		FILE * file;
		status_t status;
};


#endif