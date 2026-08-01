#include <string.h>

#include "cli/GlobalOpts.h"
#include "cli/Files.h"


// Implementation

Files::Files() : Command(),
	dir(NULL)
{
}


const char * Files::Command()
{
	return "files";
}


const char * Files::Usage()
{
	return "files [path]";
}


bool Files::RequiresOneDevice()
{
	return true;
}


bool Files::ParseArgs(int argc, const char * argv[])
{
	if (argc == 1) {
		dir = NULL;
		return true;
	}
	
	if (argc != 2) {
		fprintf(stderr, "ERROR: The %s command takes zero or one argument\n", Command());
		return false;
	}
	
	if (!globalOpts->Device().SupportsSetWorkingDir()) {
		fprintf(stderr, "ERROR: The device does not support setting the working dir\n");
		return false;
	}
	
	dir = argv[1];
	return true;
}


int Files::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	BlueSCSICommand & comm = device.Command();
	
	char oldDir[BLUE_SCSI_MAX_WORKING_DIR_LEN];
	if (device.SupportsSetWorkingDir()) {
		if (!comm.GetWorkingDir(oldDir, sizeof(oldDir))) {
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return -1;
		}
		if (dir != NULL) {
			char newDir[BLUE_SCSI_MAX_WORKING_DIR_LEN];
			strcpy(newDir, dir);
			if (!comm.SetWorkingDir(newDir, sizeof(newDir))) {
				if (comm.HasError())
					fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
				return -1;
			}
		}
	}
	
	int result = ListFiles(dir != NULL ? dir : oldDir);
	
	if (dir != NULL) {
		if (!comm.SetWorkingDir(oldDir, sizeof(oldDir))) {
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return -1;
		}
	}
	
	return result;
}


int Files::ListFiles(const char * path)
{
	BlueSCSIDevice & device = globalOpts->Device();
	BlueSCSICommand & comm = device.Command();
	
	if (path[0] != '\0')
		printf("List files under %s on %s:\n", path, device.PathString());
	else
		printf("List files on %s:\n", device.PathString());
	
	uint8 numFiles = 0;
	if (!comm.CountFiles(&numFiles)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	
	if (numFiles == 0) {
		printf("  No files found.\n");
		return 0;
	}
	
	BlueSCSIFileEntry * fileEntries = new(BlueSCSIFileEntry[numFiles]);
	if (!comm.ListFiles(fileEntries, numFiles)) {
		delete[] fileEntries;
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	printf("\n");
	printf("+-----------------------------------------------------------------+\n");
	printf("| %-6s| %-5s| %-14s| %-33s|\n", "Index", "Type", "Size", "Name");
	printf("|-----------------------------------------------------------------|\n");
	
	for (int i = 0; i < numFiles; i++) {
		printf("| %-6u| %-5s| %-14Lu| %-33s|\n", (uint32)fileEntries[i].index,
			comm.FileTypeStr(fileEntries[i].type), comm.GetFileSize(fileEntries[i]),
			fileEntries[i].name);
	}
	printf("+-----------------------------------------------------------------+\n");
	
	delete[] fileEntries;
	return 0;
}
