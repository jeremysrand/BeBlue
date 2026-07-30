#include <stdarg.h>

#ifndef LOGGER_H
#define LOGGER_H

#define LOGGER_TIMESTAMP_LEN 12


// Interface

class Logger {
	public:
		Logger();
		
		virtual void Log(const char * timestamp, const char * fmt, va_list ap) = 0;
		void FormatTimestamp(char * timestamp);		

	private:
		bigtime_t baseline;
};

#endif