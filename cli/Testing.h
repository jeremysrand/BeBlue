#include "cli/Command.h"

#ifndef CLI_TESTING_H
#define CLI_TESTING_H


// Interface

class Testing : public Command {
	public:
		Testing();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
	private:
};


#endif