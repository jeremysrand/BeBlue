#include "common/BlueSCSIDevice.h"

#ifndef CLI_COMMAND_H
#define CLI_COMMAND_H

// Forward declarations;

class GlobalOpts;
class Logger;


// Interface

class Command : public BlueSCSIDeviceErrorHandler
{
	public:
		Command();
	
		virtual const char * Command() = 0;
		virtual const char * Usage() = 0;
		virtual bool RequiresOneDevice() = 0;
		virtual bool ParseArgs(int argc, const char * argv[]) = 0;
		virtual int Execute() = 0;
		
		void SetGlobalOpts(GlobalOpts & globalOpts);
		void VerbosePrintf(const char * format, ...);
		void VerboseErrorf(const char * format, ...);
		
		virtual void HandleError(BPath * path, const char * err);
		virtual Logger * GetLogger();
		
	protected:
		GlobalOpts * globalOpts;
};

#endif