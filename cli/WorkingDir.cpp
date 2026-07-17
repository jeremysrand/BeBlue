#include <string.h>

#include "cli/GlobalOpts.h"
#include "cli/WorkingDir.h"

#include "common/BlueSCSIDevice.h"


// Implementation

WorkingDir::WorkingDir() : Command(),
	workingDir(NULL)
{
}


const char * WorkingDir::Command()
{
	return "workingdir";
}


const char * WorkingDir::Usage()
{
	return "workingdir [path]";
}


bool WorkingDir::RequiresOneDevice()
{
	return true;
}


bool WorkingDir::ParseArgs(int argc, const char * argv[])
{
	if (argc == 1) {
		workingDir = NULL;
		return true;
	}
	
	if (argc != 2) {
		fprintf(stderr, "The %s command takes zero or one argument\n", Command());
		return false;
	}
	
	if (strlen(argv[1]) >= BLUE_SCSI_MAX_WORKING_DIR_LEN) {
		fprintf(stderr, "The working directory length is longer than the max length of %d\n",
			BLUE_SCSI_MAX_WORKING_DIR_LEN);
		return false;
	}
	
	workingDir = argv[1];
	return true;
}


int WorkingDir::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	if (!device.SupportsSetWorkingDir()) {
		fprintf(stderr, "ERROR: Device %s does not support a working directory.\n",
			device.PathString());
		return -1;
	}
	
	BlueSCSICommand & comm = device.Command();
	char buffer[BLUE_SCSI_MAX_WORKING_DIR_LEN];
	if (workingDir == NULL) {
		if (!comm.GetWorkingDir(buffer, sizeof(buffer))) {
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return -1;
		}
	} else {
		strcpy(buffer, workingDir);
		if (!comm.SetWorkingDir(buffer, sizeof(buffer))) {
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return -1;
		}
	}
	
	printf("%s:\n", device.PathString());
	printf("  Working Dir: %s\n", buffer);
	
	return 0;
}
