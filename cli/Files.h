#include "cli/Command.h"

#ifndef CLI_FILES_H
#define CLI_FILES_H


// Interface

class Files : public Command {
	public:
		Files();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
	
	private:
		int ListFiles(const char * path);
		const char * FileTypeStr(uint8 type);
		
	private:
		const char * dir;
};


#endif