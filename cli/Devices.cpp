#include "cli/GlobalOpts.h"
#include "cli/Devices.h"


// Implementation

Devices::Devices() : Command()
{
}


const char * Devices::Command()
{
	return "devices";
}


const char * Devices::Usage()
{
	return Command();
}


bool Devices::RequiresOneDevice()
{
	return true;
}


bool Devices::ParseArgs(int argc, const char * argv[])
{
	if (argc != 1) {
		fprintf(stderr, "ERROR: The %s command takes no arguments\n", Command());
		return false;
	}
	return true;
}


int Devices::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	BlueSCSICommand & comm = device.Command();
	
	BlueSCSIListDevsResult listDevs;
	if (!comm.ListDevices(&listDevs)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	printf("Devices on %s:\n\n", device.PathString());
	
	printf("+------------------------------------+\n");
	printf("| %-3s| %-6s| %-22s|\n", "ID", "Type", "Description");
	printf("|------------------------------------|\n");
	for (int i = 0; i < BLUE_SCSI_MAX_DEVICES; i++)
		printf("| %-3d| 0x%02x  | %-22s|\n", i, (uint32)listDevs.devices[i],
			comm.DeviceStr((uint32)listDevs.devices[i]));
	printf("+------------------------------------+\n");

	return 0;
}

