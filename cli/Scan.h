#include "cli/Command.h"

#ifndef CLI_SCAN_H
#define CLI_SCAN_H

// Forward declarations

class BlueSCSIDevice;


// Interface

class Scan : public Command
{
	public:
		Scan();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
	private:
		void PrintDevice(BlueSCSIDevice & device);
};

#endif