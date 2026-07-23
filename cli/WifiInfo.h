#include "cli/Command.h"

#ifndef CLI_WIFI_INFO_H
#define CLI_WIFI_INFO_H


// Interface

class WifiInfo : public Command {
	public:
		WifiInfo();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
	private:
};


#endif