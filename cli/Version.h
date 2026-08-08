#include "cli/Command.h"

#ifndef CLI_VERSION_H
#define CLI_VERSION_H


// Interface

class Version : public Command
{
	public:
		Version();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
};

#endif