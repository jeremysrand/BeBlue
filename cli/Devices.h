#include "cli/Command.h"

#ifndef CLI_DEVICES_H
#define CLI_DEVICES_H


// Interface

class Devices : public Command {
	public:
		Devices();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
	private:
		const char * DeviceStr(uint32 devNumber);
};


#endif