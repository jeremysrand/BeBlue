#include "cli/GlobalOpts.h"
#include "cli/Scan.h"

#include "common/BlueSCSIDevice.h"
#include "common/BlueSCSIScan.h"


// Implementation

Scan::Scan() : Command()
{
}


const char * Scan::Command()
{
	return "scan";
}


const char * Scan::Usage()
{
	return Command();
}


bool Scan::RequiresOneDevice()
{
	return false;
}


bool Scan::ParseArgs(int argc, const char * argv[])
{
	if (argc != 1) {
		fprintf(stderr, "The %s command takes no arguments\n", Command());
		return false;
	}
	return true;
}


int Scan::Execute()
{
	printf("%-3s %-3s %-3s %-8s %-8s %-16s %-4s\n", "Bus", "ID", "LUN", "Type",
		"Vendor", "Device", "Rev");
	printf("-----------------------------------------------------\n");
	if (globalOpts->HasDevice()) {
		PrintDevice(globalOpts->Device());
		return 0;
	}
	
	BlueSCSIScan scan;
	for (int32 i = 0; i < scan.NumDevices(); i++)
		PrintDevice(*scan.DeviceAt(i));
	
	return 0;	
}


void Scan::PrintDevice(BlueSCSIDevice & device)
{
	const SCSIInquiryResult & inquiry = device.Inquiry();
	
	VerbosePrintf("%s:\n", device.PathString());
	printf("%3d %3d %3d %-8s %-8s %-16s %-4s\n", device.Bus(), device.Target(),
		device.Lun(), inquiry.typeStr, inquiry.vendorStr, inquiry.deviceStr,
		inquiry.versionStr);
}

