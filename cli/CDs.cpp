#include <string.h>

#include "cli/GlobalOpts.h"
#include "cli/CDs.h"


// Implementation

CDs::CDs() : Command()
{
}


const char * CDs::Command()
{
	return "cds";
}


const char * CDs::Usage()
{
	return Command();
}


bool CDs::RequiresOneDevice()
{
	return true;
}


bool CDs::ParseArgs(int argc, const char * argv[])
{
	if (argc != 1) {
		fprintf(stderr, "ERROR: The %s command takes no arguments\n", Command());
		return false;
	}
	return true;
}


int CDs::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	BlueSCSICommand & comm = device.Command();
	
	printf("List CDs on %s:\n", device.PathString());
	
	uint8 numCDs = 0;
	if (!comm.CountCDs(&numCDs)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	
	if (numCDs == 0) {
		printf("  No CDs found.\n");
		return 0;
	}
	
	BlueSCSIFileEntry * fileEntries = new(BlueSCSIFileEntry[numCDs]);
	if (!comm.ListCDs(fileEntries, numCDs)) {
		delete[] fileEntries;
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	printf("\n");
	printf("+-----------------------------------------------------------------+\n");
	printf("| %-6s| %-5s| %-14s| %-33s|\n", "Index", "Type", "Size", "Name");
	printf("|-----------------------------------------------------------------|\n");
	
	for (int i = 0; i < numCDs; i++) {
		printf("| %-6u| %-5s| %-14Lu| %-33s|\n", (uint32)fileEntries[i].index,
			FileTypeStr(fileEntries[i].type), comm.GetFileSize(fileEntries[i]),
			fileEntries[i].name);
	}
	printf("+-----------------------------------------------------------------+\n");
	
	delete[] fileEntries;
	return 0;
}


const char * CDs::FileTypeStr(uint8 type)
{
	switch (type) {
		case BLUE_SCSI_FILE_TYPE:
			return "F";
		case BLUE_SCSI_DIR_TYPE:
			return "D";
	}
	
	return "?";
}
