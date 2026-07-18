#include "cli/Command.h"

#ifndef CLI_SEND_H
#define CLI_SEND_H


// Interface

class Send : public Command {
	public:
		Send();
		
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