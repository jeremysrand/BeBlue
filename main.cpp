#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <Directory.h>
#include <Entry.h>
#include <Path.h>

#include "BlueSCSICommand.h"


// Defines

#define SCSI_BUS "/dev/bus/scsi"
#define SCSI_RAW_DEV_NAME "raw"


// Implementation

static void inquiry(const char * dev)
{
	int fd;
	
	printf("Running inquiry against dev = %s\n", dev);
	if ((fd = open(dev, 0)) < 0) {
		fprintf(stderr, "Unable top open dev %s, %s\n", dev, strerror(errno));
		return;
	}
	
	SCSIInquiryResult inqResult;
	BlueSCSICommand comm(fd);
	
	if (!comm.IsBlueSCSIInquiry(&inqResult)) {
		close(fd);
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
	
	printf("  Type:         %u (%s)\n", (uint32)inqResult.type, inqResult.typeStr);
	printf("  SCSI Version: %u\n", (uint32)inqResult.scsiVersion);
	printf("  Vendor:       \"%s\"\n", inqResult.vendorStr);
	printf("  Device:       \"%s\"\n", inqResult.deviceStr);
	printf("  Version:      \"%s\"\n", inqResult.versionStr);
	
	uint8 data[128];
	if (!comm.IsBlueSCSIModeSense(data, sizeof(data))) {
		close(fd);
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
	
	BlueSCSICapResult capResult;
	if (!comm.GetCapabilities(&capResult)) {
		close(fd);
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
	
	printf("  Capabilities:\n");
	printf("    Version:         %u\n", capResult.version);
	printf("    Flags:           %u\n", capResult.flags);
	printf("    Large Transfers: %s\n", comm.SupportsLargeTransfers(&capResult) ? "Supported" : "Unsupported");
	printf("    Large Send:      %s\n", comm.SupportsLargeSend(&capResult) ? "Supported" : "Unsupported");
	printf("    Set Working Dir: %s\n", comm.SupportsSetWorkingDir(&capResult) ? "Supported" : "Unsupported");

#if 0
	if (!comm.SetDebug(false)) {
		close(fd);
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
#endif
	
	BlueSCSIDebugResult debug;
	if (!comm.GetDebug(&debug)) {
		close(fd);
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
	printf("  Debug: %s\n", (debug.flag ? "Enabled" : "Disabled"));
	
	if (comm.SupportsSetWorkingDir(&capResult)) {
		char workingDir[64];
#if 0
		strcpy(workingDir, "/");
		if (!comm.SetWorkingDir(workingDir, sizeof(workingDir))) {
			close(fd);
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return;
		}
		
#endif
		if (!comm.GetWorkingDir(workingDir, sizeof(workingDir))) {
			close(fd);
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return;
		}
	
		printf("  Working Dir: %s\n", workingDir);
	}
	
	close(fd);
}

static void walkDevs(const char * path)
{
	BDirectory dir(path);
	if (dir.InitCheck() == B_OK) {
		BEntry entry;
		while (dir.GetNextEntry(&entry) >= 0) {
			BPath name;
			entry.GetPath(&name);
			if (entry.IsDirectory())
				walkDevs(name.Path());
			else if (strcmp(name.Leaf(), SCSI_RAW_DEV_NAME) == 0)
				inquiry(name.Path());
		}
	}
}

int main(int argc, char *argv[]) 
{
	if (argc >= 2)
		inquiry(argv[1]);
	else
		walkDevs(SCSI_BUS);
	
	exit(0);
}