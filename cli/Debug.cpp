#include <string.h>

#include "cli/GlobalOpts.h"
#include "cli/Debug.h"


// Implementation

Debug::Debug() : Command(),
	setDebug(false),
	debugFlag(false)
{
}


const char * Debug::Command()
{
	return "debug";
}


const char * Debug::Usage()
{
	return "debug [on|off]";
}


bool Debug::RequiresOneDevice()
{
	return true;
}


bool Debug::ParseArgs(int argc, const char * argv[])
{
	if (argc == 1) {
		setDebug = false;
		return true;
	}
	
	if (argc != 2) {
		fprintf(stderr, "ERROR: The %s command takes zero or one argument\n", Command());
		return false;
	}
	
	setDebug = true;
	if (strcmp(argv[1], "on") == 0) {
		debugFlag = true;
		return true;
	}
	if (strcmp(argv[1], "off") == 0) {
		debugFlag = false;
		return true;
	}
	
	fprintf(stderr, "ERROR: The %s command argument must me \"on\" or \"off\"\n", Command());
	return false;
}


int Debug::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	BlueSCSICommand & comm = device.Command();
	
	if (!setDebug) {
		BlueSCSIDebugResult debug;
		if (!comm.GetDebug(&debug)) {
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return -1;
		}
		
		debugFlag = debug.flag;
	} else if (!comm.SetDebug(debugFlag)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	
	printf("%s:\n", device.PathString());
	printf("  Debug: %s\n", (debugFlag ? "Enabled" : "Disabled"));
	
	return 0;
}
