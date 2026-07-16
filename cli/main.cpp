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
	printf("Running against dev = %s\n", dev);
	
	BlueSCSICommand comm(dev);
	if (comm.HasError()) {
		fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
	
	SCSIInquiryResult inqResult;
	if (!comm.IsBlueSCSIInquiry(&inqResult)) {
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
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
	
	BlueSCSICapResult capResult;
	if (!comm.GetCapabilities(&capResult)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
	
	printf("  Capabilities:\n");
	printf("    Version:         %u\n", capResult.version);
	printf("    Flags:           %u\n", capResult.flags);
	printf("    Large Transfers: %s\n", comm.SupportsLargeTransfers(capResult) ? "Supported" : "Unsupported");
	printf("    Large Send:      %s\n", comm.SupportsLargeSend(capResult) ? "Supported" : "Unsupported");
	printf("    Set Working Dir: %s\n", comm.SupportsSetWorkingDir(capResult) ? "Supported" : "Unsupported");

#if 0
	if (!comm.SetDebug(false)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
#endif
	
	BlueSCSIDebugResult debug;
	if (!comm.GetDebug(&debug)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
	printf("  Debug: %s\n", (debug.flag ? "Enabled" : "Disabled"));
	
	if (comm.SupportsSetWorkingDir(capResult)) {
		char workingDir[64];
#if 1
		strcpy(workingDir, "/");
		if (!comm.SetWorkingDir(workingDir, sizeof(workingDir))) {
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return;
		}
		
#endif
		if (!comm.GetWorkingDir(workingDir, sizeof(workingDir))) {
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return;
		}
	
		printf("  Working Dir: %s\n", workingDir);
	}
	
	uint8 numFiles = 0;
	if (!comm.CountFiles(&numFiles)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
	printf("  Num Files: %u\n", (uint32)numFiles);
	
	if (numFiles > 0) {
		BlueSCSIFileEntry * fileEntries = new(BlueSCSIFileEntry[numFiles]);
		if (!comm.ListFiles(fileEntries, numFiles)) {
			delete[] fileEntries;
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return;
		}
		
		for (int i = 0; i < numFiles; i++) {
			printf("\n");
			printf("  FileEntry[%u].name = %s\n", (uint32)fileEntries[i].index, fileEntries[i].name);
			printf("  FileEntry[%u].type = %u\n", (uint32)fileEntries[i].index, (uint32)fileEntries[i].type);
			printf("  FileEntry[%u].size = %Lu\n", (uint32)fileEntries[i].index, comm.GetFileSize(fileEntries[i]));
		}
		
		if (strcmp(fileEntries[0].name, "log.txt") == 0) {
			uint32 numBlocks = comm.GetFileNumBlocks(fileEntries[0]);
#if 0
			// Large transfer test
			char * buffer = new char[numBlocks * BLUE_SCSI_GET_FILE_BLOCK_SIZE];
			if (!comm.GetFile(fileEntries[0].index, 0, buffer, numBlocks * BLUE_SCSI_GET_FILE_BLOCK_SIZE)) {
				delete[] fileEntries;
				delete[] buffer;
				if (comm.HasError())
					fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
				return;
			}
			
			buffer[comm.GetFileSize(fileEntries[0])] = '\0';
			printf("=======  LOG START =======\n");
			puts(buffer);
			printf("=======   LOG END  =======\n");
			delete[] buffer;
#endif
#if 0
			// No large transfer test
			char * buffer  = new char[BLUE_SCSI_GET_FILE_BLOCK_SIZE];
			uint32 finalBlockSize = comm.GetFileSize(fileEntries[0]) % BLUE_SCSI_GET_FILE_BLOCK_SIZE;
			printf("=======  LOG START =======\n");
			for (int i = 0; i < numBlocks; i++) {
				if (!comm.GetFile(fileEntries[0].index, i, buffer, BLUE_SCSI_GET_FILE_BLOCK_SIZE)) {
					delete[] fileEntries;
					delete[] buffer;
					if (comm.HasError())
						fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
					return;
				}
				if (i != numBlocks - 1) {
					fwrite(buffer, BLUE_SCSI_GET_FILE_BLOCK_SIZE, 1, stdout);
				} else {
					fwrite(buffer, finalBlockSize, 1, stdout);
				}
			}
			printf("\n=======   LOG END  =======\n");
			
#endif
		}
		
		delete[] fileEntries;
	}
	
	BlueSCSIListDevsResult listDevs;
	if (!comm.ListDevices(&listDevs)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return;
	}
	printf("\n");
	for (int i = 0; i < BLUE_SCSI_MAX_DEVICES; i++)
		printf("  Device[%d] = %02x\n", i, (uint32)listDevs.devices[i]);
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