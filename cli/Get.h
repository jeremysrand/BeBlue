#include "cli/Command.h"

#ifndef CLI_GET_H
#define CLI_GET_H


// Interface

class Get : public Command {
	public:
		Get();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
	private:
		const char * src;
		const char * dest;
};


#endif