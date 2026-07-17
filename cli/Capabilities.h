#include "cli/Command.h"

#ifndef CLI_CAPABILITIES_H
#define CLI_CAPABILITIES_H


// Interface

class Capabilities : public Command {
	public:
		Capabilities();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
	private:
};


#endif