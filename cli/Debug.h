#include "cli/Command.h"

#ifndef CLI_DEBUG_H
#define CLI_DEBUG_H


// Interface

class Debug : public Command {
	public:
		Debug();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
	private:
		bool setDebug;
		bool debugFlag;
};


#endif