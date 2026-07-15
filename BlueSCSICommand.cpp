#include <string.h>

#include "BlueSCSICommand.h"


// Defines

#define BLUE_SCSI_TOOLBOX_METADATA 0xd9
#define BLUE_SCSI_TOOLBOX_VERSION 0x00

#define BLUE_SCSI_GET_CAPABILITIES 0x01
#define BLUE_SCSI_CAP_LARGE_TRANSFERS 0x1
#define BLUE_SCSI_CAP_LARGE_SEND 0x2
#define BLUE_SCSI_CAP_SET_WORKING_DIR 0x4

#define BLUE_SCSI_GET_WORKING_DIR 0x03
#define BLUE_SCSI_SET_WORKING_DIR 0x02

#define BLUE_SCSI_VENDOR "BLUESCSI"
#define BLUE_SCSI_VERSION "1.0"

#define BLUE_SCSI_MODE_SENSE_MIN_DATA_LEN 64
#define BLUE_SCSI_MODE_SENSE_PAGE 0x31
#define BLUE_SCSI_MODE_SENSE_PAGE_LEN 0x2a
#define BLUE_SCSI_MODE_SENSE_STR "BlueSCSI is the BEST STOLEN FROM BLUESCSI"

#define BLUE_SCSI_TOGGLE_DEBUG 0xd6
#define BLUE_SCSI_TOGGLE_DEBUG_SET 0x00
#define BLUE_SCSI_TOGGLE_DEBUG_GET 0x01

// Implementation

BlueSCSICommand::BlueSCSICommand(int fd)
	: SCSICommand(fd)
{
}


BlueSCSICommand::~BlueSCSICommand()
{
}


bool BlueSCSICommand::ToolboxMetadata(uint8 subcommand, uint8 * data, uint8 dataLen)
{
	uint8 command[] = { BLUE_SCSI_TOOLBOX_METADATA, subcommand, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, dataLen, 0x00 };
	return ExecuteCommand(command, sizeof(command), data, dataLen);
}


bool BlueSCSICommand::IsBlueSCSIInquiry(SCSIInquiryResult * result)
{
	if (!Inquiry(result))
		return false;
		
	return ((strcmp(result->vendorStr, BLUE_SCSI_VENDOR) == 0) &&
            (strcmp(result->versionStr, BLUE_SCSI_VERSION) == 0));
}


bool BlueSCSICommand::IsBlueSCSIModeSense(uint8 * data, uint8 dataLen)
{
	if (dataLen < BLUE_SCSI_MODE_SENSE_MIN_DATA_LEN) {
		RaiseError(FormatError("Need %u bytes for mode sense data but got %u",
			(uint32)BLUE_SCSI_MODE_SENSE_MIN_DATA_LEN, (uint32)dataLen));
			return false;
	}
	if (!ModeSense(BLUE_SCSI_MODE_SENSE_PAGE, data, dataLen))
		return false;
	
	
	uint8 blockDescLen = data[SCSI_MODE_SENSE_BLOCK_DESC_LEN_OFFSET];
	if (SCSI_MODE_SENSE_HEADER_SIZE + blockDescLen + 2 >= dataLen)
		return false;
		
	if (data[SCSI_MODE_SENSE_HEADER_SIZE + blockDescLen] != BLUE_SCSI_MODE_SENSE_PAGE)
		return false;
		
	if (data[SCSI_MODE_SENSE_HEADER_SIZE + blockDescLen + 1] != BLUE_SCSI_MODE_SENSE_PAGE_LEN)
		return false;
	
	return (strcmp((const char *)&(data[SCSI_MODE_SENSE_HEADER_SIZE + blockDescLen + 2]), BLUE_SCSI_MODE_SENSE_STR) == 0);
}


bool BlueSCSICommand::GetCapabilities(BlueSCSICapResult * result)
{
	if (!ToolboxMetadata(BLUE_SCSI_GET_CAPABILITIES, (uint8 *)result, sizeof(result)))
		return false;
	
	if (result->version != BLUE_SCSI_TOOLBOX_VERSION) {
		RaiseError(FormatError("Expected BlueSCSI toolbox version %u but got %u",
			(uint32)BLUE_SCSI_TOOLBOX_VERSION, (uint32)result->version));
		return false;
	}
	
	return true;
}


bool BlueSCSICommand::SupportsLargeTransfers(const BlueSCSICapResult * result)
{
	return (result->flags & BLUE_SCSI_CAP_LARGE_TRANSFERS) != 0;
}


bool BlueSCSICommand::SupportsLargeSend(const BlueSCSICapResult * result)
{
	return (result->flags & BLUE_SCSI_CAP_LARGE_SEND) != 0;
}


bool BlueSCSICommand::SupportsSetWorkingDir(const BlueSCSICapResult * result)
{
	return (result->flags & BLUE_SCSI_CAP_SET_WORKING_DIR) != 0;
}


bool BlueSCSICommand::GetDebug(BlueSCSIDebugResult *result)
{
	uint8 command[] = { BLUE_SCSI_TOGGLE_DEBUG, BLUE_SCSI_TOGGLE_DEBUG_GET, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), result, sizeof(*result));
}


bool BlueSCSICommand::SetDebug(bool enabled)
{
	uint8 command[] = { BLUE_SCSI_TOGGLE_DEBUG, BLUE_SCSI_TOGGLE_DEBUG_SET, (enabled ? 0x01 : 0x00), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), NULL, 0);
}


bool BlueSCSICommand::GetWorkingDir(char * path, uint8 maxPathLen)
{
	return ToolboxMetadata(BLUE_SCSI_GET_WORKING_DIR, (uint8 *)path, maxPathLen);
}


bool BlueSCSICommand::SetWorkingDir(char * path, uint8 maxPathLen)
{
	return ToolboxMetadata(BLUE_SCSI_SET_WORKING_DIR, (uint8 *)path, maxPathLen);
}