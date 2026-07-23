#include <string.h>

#include "cli/GlobalOpts.h"
#include "cli/SetCD.h"


// Implementation

SetCD::SetCD() : Command(),
	filename(NULL)
{
}


const char * SetCD::Command()
{
	return "set-cd";
}


const char * SetCD::Usage()
{
	return "set-cd filename";
}


bool SetCD::RequiresOneDevice()
{
	return true;
}


bool SetCD::ParseArgs(int argc, const char * argv[])
{
	if (argc != 2) {
		fprintf(stderr, "ERROR: The %s command takes one argument\n", Command());
		return false;
	}
	
	filename = argv[1];
	return true;
}


int SetCD::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	BlueSCSICommand & comm = device.Command();
	
	uint8 numCDs = 0;
	if (!comm.CountCDs(&numCDs)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	
	if (numCDs == 0) {
		fprintf(stderr, "No CDs found on %s\n", device.PathString());
		return -1;
	}
	
	BlueSCSIFileEntry * fileEntries = new(BlueSCSIFileEntry[numCDs]);
	if (!comm.ListCDs(fileEntries, numCDs)) {
		delete[] fileEntries;
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	
	for (int i = 0; i < numCDs; i++) {
		if (strcmp(filename, fileEntries[i].name) == 0) {
			if (!comm.SetNextCD(fileEntries[i].index)) {
				delete[] fileEntries;
				if (comm.HasError())
					fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
				return -1;				
			}
			
			delete[] fileEntries;
			return 0;
		}
	}
	
	delete[] fileEntries;
	fprintf(stderr, "ERROR: Unable to find CD %s\n", filename);
	
	return -1;
}
