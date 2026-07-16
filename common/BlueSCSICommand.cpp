#include <string.h>

#include "BlueSCSICommand.h"


// Defines

#define BLUE_SCSI_TOOLBOX_METADATA 0xd9
#define BLUE_SCSI_TOOLBOX_VERSION 0x00

#define BLUE_SCSI_LIST_DEVICES 0x00

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

#define BLUE_SCSI_COUNT_FILES 0xd2
#define BLUE_SCSI_LIST_FILES 0xd0
#define BLUE_SCSI_GET_FILE 0xd1
#define BLUE_SCSI_SEND_FILE_PREP 0xd3
#define BLUE_SCSI_SEND_FILE 0xd4
#define BLUE_SCSI_SEND_FILE_END 0xd5

#define BLUE_SCSI_FILE_MAX_BLOCKS_PER_TRANSFER 255
#define BLUE_SCSI_FILE_NAME_MAX_LEN 32
#define BLUE_SCSI_SEND_FILE_MAX_OFFSET_BLOCK 0xffffff
#define BLUE_SCSI_SEND_FILE_MAX_BYTES 0xffff

#define BLUE_SCSI_COUNT_CDS 0xda
#define BLUE_SCSI_LIST_CDS 0xd7
#define BLUE_SCSI_SET_NEXT_CD 0xd8

#define BLUE_SCSI_WIFI_CMD 0x1c
#define BLUE_SCSI_WIFI_CMD_SCAN 0x01
#define BLUE_SCSI_WIFI_CMD_COMPLETE 0x02
#define BLUE_SCSI_WIFI_CMD_SCAN_RESULTS 0x03
#define BLUE_SCSI_WIFI_CMD_INFO 0x04
#define BLUE_SCSI_WIFI_CMD_JOIN 0x05


// Implementation

BlueSCSICommand::BlueSCSICommand(const char * dev)
	: SCSICommand(dev)
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


bool BlueSCSICommand::SupportsLargeTransfers(const BlueSCSICapResult & result)
{
	return (result.flags & BLUE_SCSI_CAP_LARGE_TRANSFERS) != 0;
}


bool BlueSCSICommand::SupportsLargeSend(const BlueSCSICapResult & result)
{
	return (result.flags & BLUE_SCSI_CAP_LARGE_SEND) != 0;
}


bool BlueSCSICommand::SupportsSetWorkingDir(const BlueSCSICapResult & result)
{
	return (result.flags & BLUE_SCSI_CAP_SET_WORKING_DIR) != 0;
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


bool BlueSCSICommand::ListDevices(BlueSCSIListDevsResult * result)
{
	return ToolboxMetadata(BLUE_SCSI_LIST_DEVICES, (uint8 *)result, sizeof(*result));
}


bool BlueSCSICommand::CountFiles(uint8 * result)
{
	uint8 command[] = { BLUE_SCSI_COUNT_FILES, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), result, sizeof(*result));
}


bool BlueSCSICommand::ListFiles(BlueSCSIFileEntry * fileEntries, uint8 maxEntries)
{
	uint8 command[] = { BLUE_SCSI_LIST_FILES, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), fileEntries, sizeof(*fileEntries) * maxEntries);
}

uint64 BlueSCSICommand::GetFileSize(const BlueSCSIFileEntry & fileEntry) {
	uint64 result = 0;
	
	result += fileEntry.size[0];
	result <<= 8;
	
	result += fileEntry.size[1];
	result <<= 8;
	
	result += fileEntry.size[2];
	result <<= 8;
	
	result += fileEntry.size[3];
	result <<= 8;
	
	result += fileEntry.size[4];
	return result;
}


uint32 BlueSCSICommand::GetFileNumBlocks(const BlueSCSIFileEntry & fileEntry) {
	return (GetFileSize(fileEntry) / BLUE_SCSI_GET_FILE_BLOCK_SIZE) + 1;
}


bool BlueSCSICommand::GetFile(uint8 fileIndex, uint32 blockOffset, char * buffer,
	size_t bufferSize)
{
	if ((bufferSize % BLUE_SCSI_GET_FILE_BLOCK_SIZE) != 0) {
		RaiseError(FormatError("Buffer size must be a multiple of %u but the size is %Lu",
			(uint32)BLUE_SCSI_GET_FILE_BLOCK_SIZE, bufferSize));
		return false;
	}
	
	uint32 numBlocks = bufferSize / BLUE_SCSI_GET_FILE_BLOCK_SIZE;
	if (numBlocks > BLUE_SCSI_FILE_MAX_BLOCKS_PER_TRANSFER) {
		RaiseError(FormatError("Buffer size was %Lu which is %u blocks but max per transfer is %u",
			bufferSize, numBlocks, (uint32)BLUE_SCSI_FILE_MAX_BLOCKS_PER_TRANSFER));
		return false;
	}
	
	// When large transfers are not supported, all transfers are in single 4k blocks.
	// The numBlocks argument can be 0 when you want a single block for backward
	// compatibility.  When asking for a single block, this code always sends a 0 just in
	// case sending a 1 when large transfer are not supported is a problem.  Seems like 0
	// is the safer bet.
	//
	// Note that it is up to the caller of this routine to check if large transfers are
	// supported before doing a multiblock transfer.
	if (numBlocks == 1)
		numBlocks = 0;
		
	uint8 command[] = { BLUE_SCSI_GET_FILE,
		fileIndex,
		(blockOffset >> 24) & 0xff,
		(blockOffset >> 16) & 0xff,
		(blockOffset >> 8) & 0xff,
		blockOffset & 0xff,
		numBlocks,
		0x00,
		0x00,
		0x00 };
	return ExecuteCommand(command, sizeof(command), buffer, bufferSize);
}


bool BlueSCSICommand::PrepareToSendFile(const char * filename)
{
	char buffer[BLUE_SCSI_FILE_NAME_MAX_LEN + 1];
	if (strlen(filename) > BLUE_SCSI_FILE_NAME_MAX_LEN) {
		RaiseError(FormatError("Filename length must be %u characters in length but name is %u characters",
			(uint32)BLUE_SCSI_FILE_NAME_MAX_LEN, (uint32)strlen(filename)));
		return false;
	}
	uint8 command[] = { BLUE_SCSI_SEND_FILE_PREP, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), buffer, sizeof(buffer));
}


bool BlueSCSICommand::SendFileBulk(uint32 blockOffset, char * buffer, size_t bufferSize)
{
	if (blockOffset > BLUE_SCSI_SEND_FILE_MAX_OFFSET_BLOCK) {
		RaiseError(FormatError("Block offset %u is beyond limit of %u",
			blockOffset, (uint32)BLUE_SCSI_SEND_FILE_MAX_OFFSET_BLOCK));
		return false;
	}
	
	if ((bufferSize % BLUE_SCSI_SEND_FILE_BLOCK_SIZE) != 0) {
		RaiseError(FormatError("Buffer size is %Lu is not a multiple of %u blocks",
			bufferSize, (uint32)BLUE_SCSI_SEND_FILE_BLOCK_SIZE));
		return false;
	}
	
	uint32 numBlocks = bufferSize / BLUE_SCSI_SEND_FILE_BLOCK_SIZE;
	if (numBlocks > BLUE_SCSI_FILE_MAX_BLOCKS_PER_TRANSFER) {
		RaiseError(FormatError("Buffer size was %Lu which is %u blocks but max per transfer is %u",
			bufferSize, numBlocks, (uint32)BLUE_SCSI_FILE_MAX_BLOCKS_PER_TRANSFER));
		return false;
	}
	
	uint8 command[] = { BLUE_SCSI_SEND_FILE,
		0x00,
		0x00,
		(blockOffset >> 16) & 0xff,
		(blockOffset >> 8) & 0xff,
		blockOffset & 0xff,
		numBlocks,
		0x00,
		0x00,
		0x00 };
	return ExecuteCommand(command, sizeof(command), buffer, bufferSize);
}


bool BlueSCSICommand::SendFileBytes(uint32 blockOffset, char * buffer, size_t bufferSize)
{
	if (blockOffset > BLUE_SCSI_SEND_FILE_MAX_OFFSET_BLOCK) {
		RaiseError(FormatError("Block offset %u is beyond limit of %u",
			blockOffset, (uint32)BLUE_SCSI_SEND_FILE_MAX_OFFSET_BLOCK));
		return false;
	}
	
	if (bufferSize > BLUE_SCSI_SEND_FILE_MAX_BYTES) {
		RaiseError(FormatError("Buffer size was %Lu which is larger than the max of %u",
			bufferSize, (uint32)BLUE_SCSI_SEND_FILE_MAX_BYTES));
		return false;
	}
	
	uint8 command[] = { BLUE_SCSI_SEND_FILE,
		(bufferSize >> 8) & 0xff,
		bufferSize & 0xff,
		(blockOffset >> 16) & 0xff,
		(blockOffset >> 8) & 0xff,
		blockOffset & 0xff,
		0x00,
		0x00,
		0x00,
		0x00 };
	return ExecuteCommand(command, sizeof(command), buffer, bufferSize);
}


bool BlueSCSICommand::SendFileEnd()
{
	uint8 command[] = { BLUE_SCSI_SEND_FILE_END, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), NULL, 0);
}


bool BlueSCSICommand::CountCDs(uint8 * result)
{
	uint8 command[] = { BLUE_SCSI_COUNT_CDS, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), result, sizeof(*result));
}


bool BlueSCSICommand::ListCDs(BlueSCSIFileEntry * fileEntries, uint8 maxEntries)
{
	uint8 command[] = { BLUE_SCSI_LIST_CDS, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), fileEntries, sizeof(*fileEntries) * maxEntries);
}


bool BlueSCSICommand::SetNextCD(uint8 fileIndex)
{
	uint8 command[] = { BLUE_SCSI_SET_NEXT_CD, fileIndex, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), NULL, 0);
}


bool BlueSCSICommand::StartWifiScan(bool * started)
{
	uint8 result = 0;
	uint8 command[] = { BLUE_SCSI_WIFI_CMD, BLUE_SCSI_WIFI_CMD_SCAN, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	if (!ExecuteCommand(command, sizeof(command), &result, sizeof(result)))
		return false;
		
	*started = (result != 0);
	return true;
}


bool BlueSCSICommand::CheckWifiScanComplete(bool * completed)
{
	uint8 result = 0;
	uint8 command[] = { BLUE_SCSI_WIFI_CMD, BLUE_SCSI_WIFI_CMD_COMPLETE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	if (!ExecuteCommand(command, sizeof(command), &result, sizeof(result)))
		return false;
		
	*completed = (result != 0);
	return true;
}


bool BlueSCSICommand::WifiScanResults(BlueSCSINetworkEntries * result)
{
	uint8 command[] = { BLUE_SCSI_WIFI_CMD, BLUE_SCSI_WIFI_CMD_SCAN_RESULTS, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), result, sizeof(*result));
}


bool BlueSCSICommand::WifiInfo(BlueSCSINetworkEntry * result)
{
	uint8 command[] = { BLUE_SCSI_WIFI_CMD, BLUE_SCSI_WIFI_CMD_INFO, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), result, sizeof(*result));
}


bool BlueSCSICommand::WifiJoin(BlueSCSINetworkJoinRequest * request)
{
	uint8 command[] = { BLUE_SCSI_WIFI_CMD, BLUE_SCSI_WIFI_CMD_JOIN, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), request, sizeof(*request));
}