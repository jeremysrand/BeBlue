#include "cli/Command.h"

#ifndef CLI_WORKING_DIR_H
#define CLI_WORKING_DIR_H


// Interface

class WorkingDir : public Command {
	public:
		WorkingDir();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
	private:
		const char * workingDir;
};


#endif