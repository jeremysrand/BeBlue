#include "cli/Command.h"

#ifndef CLI_WIFI_JOIN_H
#define CLI_WIFI_JOIN_H


// Interface

class WifiJoin : public Command {
	public:
		WifiJoin();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
	private:
		uint8 channel;
		const char * ssid;
		const char * key;
};


#endif