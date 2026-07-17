#include "cli/GlobalOpts.h"
#include "cli/Testing.h"


// Implementation

Testing::Testing() : Command()
{
}


const char * Testing::Command()
{
	return "testing";
}


const char * Testing::Usage()
{
	return Command();
}


bool Testing::RequiresOneDevice()
{
	return true;
}


bool Testing::ParseArgs(int argc, const char * argv[])
{
	if (argc != 1) {
		fprintf(stderr, "The %s command takes no arguments\n", Command());
		return false;
	}
	return true;
}


int Testing::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	
	printf("Running against dev = %s\n", device.PathString());
	if (!device.IsBlueSCSI()) {
		printf("  Not a BlueSCSI!!\n");
		return -1;
	}
	
	BlueSCSICommand & comm = device.Command();
	
	uint8 numFiles = 0;
	if (!comm.CountFiles(&numFiles)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	printf("  Num Files: %u\n", (uint32)numFiles);
	
	if (numFiles > 0) {
		BlueSCSIFileEntry * fileEntries = new(BlueSCSIFileEntry[numFiles]);
		if (!comm.ListFiles(fileEntries, numFiles)) {
			delete[] fileEntries;
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return -1;
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
				return -1;
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
		return -1;
	}
	printf("\n");
	for (int i = 0; i < BLUE_SCSI_MAX_DEVICES; i++)
		printf("  Device[%d] = %02x\n", i, (uint32)listDevs.devices[i]);

	return 0;
}

