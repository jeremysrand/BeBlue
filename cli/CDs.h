#include "cli/Command.h"

#ifndef CLI_CDS_H
#define CLI_CDS_H


// Interface

class CDs : public Command {
	public:
		CDs();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
	
	private:
		const char * FileTypeStr(uint8 type);
};


#endif