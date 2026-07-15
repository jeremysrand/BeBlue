#include "SCSICommand.h"

#ifndef BLUE_SCSI_COMMAND_H
#define BLUE_SCSI_COMMAND_H


// Interface

struct BlueSCSICapResult {
	uint8 version;
	uint8 flags;
	uint8 reserved[6];
};

struct BlueSCSIDebugResult {
	uint8 flag;
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
		
	private:
		bool ToolboxMetadata(uint8 subcommand, uint8 * data, uint8 dataLen);
};

#endif
