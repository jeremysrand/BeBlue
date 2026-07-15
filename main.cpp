#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <Directory.h>
#include <Entry.h>
#include <Path.h>

#include "SCSICommand.h"


#define SCSI_BUS "/dev/bus/scsi"
#define SCSI_RAW_DEV_NAME "raw"

static void inquiry(const char * dev)
{
	int fd;
	
	printf("Running inquiry against dev = %s\n", dev);
	if ((fd = open(dev, 0)) < 0) {
		fprintf(stderr, "Unable top open dev %s, %s\n", dev, strerror(errno));
		return;
	}
	
	SCSIInquiryResult inqResult;
	SCSICommand comm(fd);
	
	if (!comm.Inquiry(&inqResult)) {
		fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
	
	printf("  Type:    %u (%s)\n", (uint32)inqResult.type, inqResult.typeStr);
	printf("  Vendor:  \"%s\"\n", inqResult.vendor);
	printf("  Device:  \"%s\"\n", inqResult.device);
	printf("  Version: \"%s\"\n", inqResult.version);
	
	const uint8 * sense = comm.GetSense();
	printf("  Sense:");
	for (int i = 0; i < SCSI_SENSE_SIZE; i++) {
		if ((i % 8) == 0)
			printf("\n      ");
		printf("0x%02x ", (int)sense[i]);
	}
	printf("\n\n");
	
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