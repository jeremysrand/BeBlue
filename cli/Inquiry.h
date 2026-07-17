#include "cli/Command.h"

#ifndef CLI_INQUIRY_H
#define CLI_INQUIRY_H


// Interface

class Inquiry : public Command {
	public:
		Inquiry();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
	private:
};


#endif