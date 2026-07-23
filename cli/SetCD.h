#include "cli/Command.h"

#ifndef CLI_SET_CD_H
#define CLI_SET_CD_H


// Interface

class SetCD : public Command {
	public:
		SetCD();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
	
	private:
		const char * filename;
};


#endif