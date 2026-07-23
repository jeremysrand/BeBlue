#include "cli/Command.h"

#ifndef CLI_WIFI_SCAN_H
#define CLI_WIFI_SCAN_H


// Interface

class WifiScan : public Command {
	public:
		WifiScan();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
	private:
};


#endif