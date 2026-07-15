#include "SCSICommand.h"

#ifndef BLUE_SCSI_COMMAND_H
#define BLUE_SCSI_COMMAND_H


// Defines

#define BLUE_SCSI_MAX_FILE_NAME_LEN 32
#define BLUE_SCSI_FILE_SIZE_BYTES 5

#define BLUE_SCSI_FILE_TYPE 1
#define BLUE_SCSI_DIR_TYPE  0


// Interface

struct BlueSCSICapResult {
	uint8 version;
	uint8 flags;
	uint8 reserved[6];
};

struct BlueSCSIDebugResult {
	uint8 flag;
};

struct BlueSCSIFileEntry {
	uint8 index;
	uint8 type;
	char name[BLUE_SCSI_MAX_FILE_NAME_LEN + 1];
	uint8 size[BLUE_SCSI_FILE_SIZE_BYTES];
};

class BlueSCSICommand : public SCSICommand {
	public:
		BlueSCSICommand(int fd);
		~BlueSCSICommand();
		
		bool IsBlueSCSIInquiry(SCSIInquiryResult * result);
		
		bool IsBlueSCSIModeSense(uint8 * data, uint8 dataLen);
		
		bool GetCapabilities(BlueSCSICapResult * result);
		bool SupportsLargeTransfers(const BlueSCSICapResult * result);
		bool SupportsLargeSend(const BlueSCSICapResult * result);
		bool SupportsSetWorkingDir(const BlueSCSICapResult * result);
		
		bool GetDebug(BlueSCSIDebugResult *result);
		bool SetDebug(bool enabled);
		
		bool GetWorkingDir(char * path, uint8 maxPathLen);
		bool SetWorkingDir(char * path, uint8 maxPathLen);
		
		bool CountFiles(uint8 * result);
		bool ListFiles(BlueSCSIFileEntry * fileEntries, uint8 maxEntries);
		uint64 GetFileSize(BlueSCSIFileEntry * fileEntry);
		
	private:
		bool ToolboxMetadata(uint8 subcommand, uint8 * data, uint8 dataLen);
};

#endif
