#include "cli/GlobalOpts.h"
#include "cli/Inquiry.h"


// Implementation

Inquiry::Inquiry() : Command()
{
}


const char * Inquiry::Command()
{
	return "inquiry";
}


const char * Inquiry::Usage()
{
	return Command();
}


bool Inquiry::RequiresOneDevice()
{
	return true;
}


bool Inquiry::ParseArgs(int argc, const char * argv[])
{
	if (argc != 1) {
		fprintf(stderr, "The %s command takes no arguments\n", Command());
		return false;
	}
	return true;
}


int Inquiry::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	const SCSIInquiryResult & inquiry = device.Inquiry();
	
	printf("Inquiry output from %s:\n", device.PathString());
	printf("  Type:         %u (%s)\n", (uint32)inquiry.type, inquiry.typeStr);
	printf("  SCSI Version: %u\n", (uint32)inquiry.scsiVersion);
	printf("  Vendor:       \"%s\"\n", inquiry.vendorStr);
	printf("  Device:       \"%s\"\n", inquiry.deviceStr);
	printf("  Version:      \"%s\"\n", inquiry.versionStr);
	
	return 0;
}

