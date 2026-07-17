#include "cli/GlobalOpts.h"
#include "cli/Capabilities.h"

#include "common/BlueSCSIDevice.h"


// Implementation

Capabilities::Capabilities() : Command()
{
}


const char * Capabilities::Command()
{
	return "capabilities";
}


const char * Capabilities::Usage()
{
	return Command();
}


bool Capabilities::RequiresOneDevice()
{
	return true;
}


bool Capabilities::ParseArgs(int argc, const char * argv[])
{
	if (argc != 1) {
		fprintf(stderr, "The %s command takes no arguments\n", Command());
		return false;
	}
	return true;
}


int Capabilities::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	const BlueSCSICapResult & capResult = device.Capabilities();
	printf("Capabilities of %s:\n", device.PathString());
	printf("    Version:         %u\n", capResult.version);
	printf("    Flags:           %u\n", capResult.flags);
	printf("    Large Transfers: %s\n", device.SupportsLargeTransfers() ? "Supported" : "Unsupported");
	printf("    Large Send:      %s\n", device.SupportsLargeSend() ? "Supported" : "Unsupported");
	printf("    Set Working Dir: %s\n", device.SupportsSetWorkingDir() ? "Supported" : "Unsupported");
	
	return 0;
}
