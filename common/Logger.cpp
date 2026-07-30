#include <kernel/OS.h>

#include "common/Logger.h"


// Implementation

Logger::Logger() :
	baseline(system_time())
{
}


void Logger::FormatTimestamp(char * timestamp)
{
	bigtime_t time = system_time() - baseline;
	
	// Convert to ms
	time /= 1000;
	
	uint32 msecs = time % 1000;
	
	// Convert to sec
	time /= 1000;
	
	if (time >= 1000000) {
		strcpy(timestamp, " <TOO BIG> ");
	} else {
		sprintf(timestamp, "%6Ld.%03d ", time, msecs);
	}	
}