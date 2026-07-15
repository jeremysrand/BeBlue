#include "SCSICommand.h"

#ifndef BLUE_SCSI_COMMAND_H
#define BLUE_SCSI_COMMAND_H


// Defines

#define BLUE_SCSI_CAP_LARGE_TRANSFERS 0x1
#define BLUE_SCSI_CAP_LARGE_SEND 0x2
#define BLUE_SCSI_CAP_SET_WORKING_DIR 0x4


// Interface

struct BlueSCSICapResult {
	uint8 version;
	uint8 flags;
	uint8 reserved[6];
};


class BlueSCSICommand : public SCSICommand {
	public:
		BlueSCSICommand(int fd);
		~BlueSCSICommand();
		
		bool IsBlueSCSIInquiry(SCSIInquiryResult * result);
		
		bool IsBlueSCSIModeSense(uint8 * data, uint8 dataLen);
		
		bool GetCapabilities(BlueSCSICapResult * result);
		
	private:
		bool ToolboxMetadata(uint8 subcommand, uint8 * data, uint8 dataLen);
};

#endif
