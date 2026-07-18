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


const char * Devices::DeviceStr(uint32 devNumber)
{
	switch (devNumber) {
		case BLUE_SCSI_DEVICE_FIXED_DISK:
			return "Fixed Disk";
		case BLUE_SCSI_DEVICE_REMOVABLE_DISK:
			return "Removable Disk";
		case BLUE_SCSI_DEVICE_OPTICAL_DISK:
			return "Optical Disk";
		case BLUE_SCSI_DEVICE_FLOPPY_DISK:
			return "Floppy Disk";
		case BLUE_SCSI_DEVICE_MAGNETO_OPTICAL_DISK:
			return "Magneto Optical Disk";
		case BLUE_SCSI_DEVICE_TAPE_DEVICE:
			return "Tape Device";
		case BLUE_SCSI_DEVICE_NETWORK_DEVICE:
			return "Network Device";
		case BLUE_SCSI_DEVICE_ZIP_DISK:
			return "Zip Disk";
		case BLUE_SCSI_DEVICE_NO_DEVICE:
			return "No Device";
	}
	
	return "<UNKNOWN>";
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
			DeviceStr((uint32)listDevs.devices[i]));
	printf("+------------------------------------+\n");

	return 0;
}

