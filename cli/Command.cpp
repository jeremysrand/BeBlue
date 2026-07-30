#include <stdio.h>

#include "cli/Command.h"
#include "cli/FileLogger.h"
#include "cli/GlobalOpts.h"


Command::Command()
	: globalOpts(NULL)
{
}


void Command::SetGlobalOpts(GlobalOpts & globalOptsArg)
{
	globalOpts = &globalOptsArg;
}


void Command::VerbosePrintf(const char * format, ...)
{
	if ((globalOpts == NULL) ||
		(!globalOpts->IsVerbose()))
		return;
		
	va_list args;
	
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}


void Command::VerboseErrorf(const char * format, ...)
{
	if ((globalOpts == NULL) ||
		(!globalOpts->IsVerbose()))
		return;
		
	va_list args;
	
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}


void Command::HandleError(BPath * path, const char * err)
{
	VerboseErrorf("ERROR: device = %s: %s\n", path->Path(), err);
}


Logger * Command::GetLogger()
{
	if (globalOpts == NULL)
		return NULL;
		
	return globalOpts->Logger();
}