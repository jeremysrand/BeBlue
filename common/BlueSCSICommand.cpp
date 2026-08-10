#include <string.h>

#include "common/BlueSCSICommand.h"


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

#define BLUE_SCSI_VENDOR_INFO "BlueSCSI"
#define BLUE_SCSI_VENDOR_INFO_LEN 8
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

#define BLUE_SCSI_FILE_NAME_MAX_LEN 32
#define BLUE_SCSI_SEND_FILE_MAX_OFFSET_BLOCK 0xffffff

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

BlueSCSICommand::BlueSCSICommand(BPath * path, Logger * logger)
	: SCSICommand(path, logger)
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
	Log("Checking for BlueSCSI via an Inquiry command");
	if (!Inquiry(result))
		return false;
		
	bool isBlueSCSI = 
		(strncmp(result->vendorInfoStr, BLUE_SCSI_VENDOR_INFO, BLUE_SCSI_VENDOR_INFO_LEN) == 0);
    
    Log("Device is %sa BlueSCSI from Inquiry result", (isBlueSCSI ? "" : "not "));
    
    return isBlueSCSI;
}


bool BlueSCSICommand::IsBlueSCSIModeSense(uint8 * data, uint8 dataLen)
{
	Log("Checking for BlueSCSI via a Mode Sense command");
	if (dataLen < BLUE_SCSI_MODE_SENSE_MIN_DATA_LEN) {
		RaiseError(FormatError("Need %u bytes for mode sense data but got %u",
			(uint32)BLUE_SCSI_MODE_SENSE_MIN_DATA_LEN, (uint32)dataLen));
			return false;
	}
	if (!ModeSense(BLUE_SCSI_MODE_SENSE_PAGE, data, dataLen))
		return false;
	
	
	uint8 blockDescLen = data[SCSI_MODE_SENSE_BLOCK_DESC_LEN_OFFSET];
	
	bool isBlueSCSI = true;
	if (SCSI_MODE_SENSE_HEADER_SIZE + blockDescLen + 2 >= dataLen)
		isBlueSCSI = false;
	else if (data[SCSI_MODE_SENSE_HEADER_SIZE + blockDescLen] != BLUE_SCSI_MODE_SENSE_PAGE)
		isBlueSCSI = false;
	else if (data[SCSI_MODE_SENSE_HEADER_SIZE + blockDescLen + 1] != BLUE_SCSI_MODE_SENSE_PAGE_LEN)
		isBlueSCSI = false;
	else
		isBlueSCSI = (strcmp((const char *)&(data[SCSI_MODE_SENSE_HEADER_SIZE + blockDescLen + 2]), BLUE_SCSI_MODE_SENSE_STR) == 0);
	
    Log("Device is %sa BlueSCSI from Mode Sense result", (isBlueSCSI ? "" : "not "));
    
	return isBlueSCSI;
}


bool BlueSCSICommand::GetCapabilities(BlueSCSICapResult * result)
{
	Log("Getting BlueSCSI capabilities from the device");
	if (!ToolboxMetadata(BLUE_SCSI_GET_CAPABILITIES, (uint8 *)result, sizeof(result)))
		return false;
	
	if (result->version != BLUE_SCSI_TOOLBOX_VERSION) {
		RaiseError(FormatError("Expected BlueSCSI toolbox version %u but got %u",
			(uint32)BLUE_SCSI_TOOLBOX_VERSION, (uint32)result->version));
		return false;
	}
	
	if (IsLogging()) {
		Log("Device is a supported BlueSCSI from the capabilities result");
		Log("  Supports large transfers: %s", (SupportsLargeTransfers(*result) ? "YES" : "NO"));
		Log("  Supports large send:      %s", (SupportsLargeSend(*result) ? "YES" : "NO"));
		Log("  Supports set working dir: %s", (SupportsSetWorkingDir(*result) ? "YES" : "NO"));
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
	Log("Getting the debug mode from BlueSCSI");
	uint8 command[] = { BLUE_SCSI_TOGGLE_DEBUG, BLUE_SCSI_TOGGLE_DEBUG_GET, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	bool success = ExecuteCommand(command, sizeof(command), result, sizeof(*result));
	if (success)
		Log("Debug mode is %s", (result->flag ? "ON" : "OFF"));
	return success;
}


bool BlueSCSICommand::SetDebug(bool enabled)
{
	Log("Setting the debug mode for the BlueSCSI to %s", (enabled ? "ON" : "OFF"));
	uint8 command[] = { BLUE_SCSI_TOGGLE_DEBUG, BLUE_SCSI_TOGGLE_DEBUG_SET, (enabled ? 0x01 : 0x00), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), NULL, 0);
}


bool BlueSCSICommand::GetWorkingDir(char * path, uint8 maxPathLen)
{
	Log("Getting the working directory of the BlueSCSI");
	bool success = ToolboxMetadata(BLUE_SCSI_GET_WORKING_DIR, (uint8 *)path, maxPathLen);
	if (success)
		Log("BlueSCSI working directory is: \"%s\"", path);
	return success;
}


bool BlueSCSICommand::SetWorkingDir(char * path, uint8 maxPathLen)
{
	Log("Setting the working directory of the BlueSCSI to \"%s\"", path);
	return ToolboxMetadata(BLUE_SCSI_SET_WORKING_DIR, (uint8 *)path, maxPathLen);
}


const char * BlueSCSICommand::DeviceStr(uint8 devNumber)
{
	switch (devNumber) {
		case BLUE_SCSI_DEVICE_FIXED_DISK:
			return "Fixed Disk";
		case BLUE_SCSI_DEVICE_REMOVABLE_DISK:
			return "Removable Disk";
		case BLUE_SCSI_DEVICE_OPTICAL_DISK:
			return "Optical Disk";
		case BLUE_SCSI_DEVICE_FLOPPY_DISK:
			return "Floppy Disk";
		case BLUE_SCSI_DEVICE_MAGNETO_OPTICAL_DISK:
			return "Magneto Optical Disk";
		case BLUE_SCSI_DEVICE_TAPE_DEVICE:
			return "Tape Device";
		case BLUE_SCSI_DEVICE_NETWORK_DEVICE:
			return "Network Device";
		case BLUE_SCSI_DEVICE_ZIP_DISK:
			return "Zip Disk";
		case BLUE_SCSI_DEVICE_NO_DEVICE:
			return "No Device";
	}
	
	return "<UNKNOWN>";
}

bool BlueSCSICommand::ListDevices(BlueSCSIListDevsResult * result)
{
	Log("Listing devices emulated by the BlueSCSI");
	bool success = ToolboxMetadata(BLUE_SCSI_LIST_DEVICES, (uint8 *)result, sizeof(*result));
	if ((success) && (IsLogging())) {
		Log("List of devices emulated:");
		for (int i = 0; i < BLUE_SCSI_MAX_DEVICES; i++) {
			if (result->devices[i] == BLUE_SCSI_DEVICE_NO_DEVICE)
				continue;
			Log("  ID: %d -> %s (%u)", i, DeviceStr(result->devices[i]),
				(uint32)result->devices[i]);
		}
	}
	return success;
}


bool BlueSCSICommand::CountFiles(uint8 * result)
{
	Log("Counting files in the working directory of the BlueSCSI");
	uint8 command[] = { BLUE_SCSI_COUNT_FILES, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	bool success = ExecuteCommand(command, sizeof(command), result, sizeof(*result));
	if (success)
		Log("There are %u files in the working directory", (uint32)*result);
	return success;
}


bool BlueSCSICommand::ListFiles(BlueSCSIFileEntry * fileEntries, uint8 maxEntries)
{
	Log("Listing files in the working directory of the BlueSCSI");
	uint8 command[] = { BLUE_SCSI_LIST_FILES, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	bool success = ExecuteCommand(command, sizeof(command), fileEntries, sizeof(*fileEntries) * maxEntries);
	if ((success) && (IsLogging())) {
		Log("Files in the working directory:");
		for (int i = 0; i < maxEntries; i++) {
			Log("  ID: %3u  T: %s  S: %14Lu  N: %s",
				fileEntries[i].index, FileTypeStr(fileEntries[i].type), 
				GetFileSize(fileEntries[i]), fileEntries[i].name);
		}
	}
	return success;
}


const char * BlueSCSICommand::FileTypeStr(uint8 type)
{
	switch (type) {
		case BLUE_SCSI_FILE_TYPE:
			return "F";
		case BLUE_SCSI_DIR_TYPE:
			return "D";
	}
	
	return "?";
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
	Log("Getting file index %u at block offset %u into %lu byte buffer", (uint32)fileIndex,
		blockOffset, bufferSize);
	if ((bufferSize % BLUE_SCSI_GET_FILE_BLOCK_SIZE) != 0) {
		RaiseError(FormatError("Buffer size must be a multiple of %u but the size is %lu",
			(uint32)BLUE_SCSI_GET_FILE_BLOCK_SIZE, bufferSize));
		return false;
	}
	
	uint32 numBlocks = bufferSize / BLUE_SCSI_GET_FILE_BLOCK_SIZE;
	if (numBlocks > BLUE_SCSI_FILE_MAX_BLOCKS_PER_TRANSFER) {
		RaiseError(FormatError("Buffer size was %lu which is %u blocks but max per transfer is %u",
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
	Log("Prepare to send file \"%s\"", filename);
	char buffer[BLUE_SCSI_FILE_NAME_MAX_LEN + 1];
	if (strlen(filename) > BLUE_SCSI_FILE_NAME_MAX_LEN) {
		RaiseError(FormatError("Filename length must be %u characters in length but name is %u characters",
			(uint32)BLUE_SCSI_FILE_NAME_MAX_LEN, (uint32)strlen(filename)));
		return false;
	}
	strcpy(buffer, filename);
	uint8 command[] = { BLUE_SCSI_SEND_FILE_PREP, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), buffer, sizeof(buffer));
}


bool BlueSCSICommand::SendFileBulk(uint32 blockOffset, char * buffer, size_t bufferSize)
{
	Log("Sending bulk file data at block offset %u from buffer of %lu bytes",
		blockOffset, bufferSize);
	if (blockOffset > BLUE_SCSI_SEND_FILE_MAX_OFFSET_BLOCK) {
		RaiseError(FormatError("Block offset %u is beyond limit of %u",
			blockOffset, (uint32)BLUE_SCSI_SEND_FILE_MAX_OFFSET_BLOCK));
		return false;
	}
	
	if ((bufferSize % BLUE_SCSI_SEND_FILE_BLOCK_SIZE) != 0) {
		RaiseError(FormatError("Buffer size is %lu is not a multiple of %u blocks",
			bufferSize, (uint32)BLUE_SCSI_SEND_FILE_BLOCK_SIZE));
		return false;
	}
	
	uint32 numBlocks = bufferSize / BLUE_SCSI_SEND_FILE_BLOCK_SIZE;
	if (numBlocks > BLUE_SCSI_FILE_MAX_BLOCKS_PER_TRANSFER) {
		RaiseError(FormatError("Buffer size was %lu which is %u blocks but max per transfer is %u",
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
	Log("Sending non-bulk file data at block offset %u from buffer of %lu bytes",
		blockOffset, bufferSize);
	if (blockOffset > BLUE_SCSI_SEND_FILE_MAX_OFFSET_BLOCK) {
		RaiseError(FormatError("Block offset %u is beyond limit of %u",
			blockOffset, (uint32)BLUE_SCSI_SEND_FILE_MAX_OFFSET_BLOCK));
		return false;
	}
	
	if (bufferSize > BLUE_SCSI_SEND_FILE_MAX_BYTES) {
		RaiseError(FormatError("Buffer size was %lu which is larger than the max of %u",
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
	Log("End of file send");
	uint8 command[] = { BLUE_SCSI_SEND_FILE_END, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), NULL, 0);
}


bool BlueSCSICommand::CountCDs(uint8 * result)
{
	Log("Counting CDs on the BlueSCSI");
	uint8 command[] = { BLUE_SCSI_COUNT_CDS, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	bool success = ExecuteCommand(command, sizeof(command), result, sizeof(*result));
	if (success)
		Log("There are %u CDs on the BlueSCSI", (uint32)*result);
	return success;
}


bool BlueSCSICommand::ListCDs(BlueSCSIFileEntry * fileEntries, uint8 maxEntries)
{
	Log("Listing CDs on the BlueSCSI");
	uint8 command[] = { BLUE_SCSI_LIST_CDS, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	bool success = ExecuteCommand(command, sizeof(command), fileEntries, sizeof(*fileEntries) * maxEntries);
	if ((success) && (IsLogging())) {
		Log("CDs on the BlueSCSI:");
		for (int i = 0; i < maxEntries; i++) {
			Log("  ID: %3u  T: %s  S: %14Lu  N: %s",
				fileEntries[i].index, FileTypeStr(fileEntries[i].type), 
				GetFileSize(fileEntries[i]), fileEntries[i].name);
		}
	}
	return success;
}


bool BlueSCSICommand::SetNextCD(uint8 fileIndex)
{
	Log("Setting next CD on the BlueSCSI to index %u", (uint32)fileIndex);
	uint8 command[] = { BLUE_SCSI_SET_NEXT_CD, fileIndex, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), NULL, 0);
}


bool BlueSCSICommand::StartWifiScan(bool * started)
{
	Log("Starting a WiFi scan on the BlueSCSI");
	uint8 result = 0;
	uint8 command[] = { BLUE_SCSI_WIFI_CMD, BLUE_SCSI_WIFI_CMD_SCAN, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	if (!ExecuteCommand(command, sizeof(command), &result, sizeof(result)))
		return false;
		
	*started = (result != 0);
	Log("Wifi scan was%s started on the BlueSCSI", (result != 0 ? "" : " not"));
	return true;
}


bool BlueSCSICommand::CheckWifiScanComplete(bool * completed)
{
	Log("Checking if WiFi scan is complete on the BlueSCSI");
	uint8 result = 0;
	uint8 command[] = { BLUE_SCSI_WIFI_CMD, BLUE_SCSI_WIFI_CMD_COMPLETE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	if (!ExecuteCommand(command, sizeof(command), &result, sizeof(result)))
		return false;
		
	*completed = (result != 0);
	Log("Wifi scan is%s completed on the BlueSCSI", (result != 0 ? "" : " not"));
	return true;
}


bool BlueSCSICommand::WifiScanResults(BlueSCSINetworkEntries * result)
{
	Log("Getting Wifi scan results from the BlueSCSI");
	uint8 command[] = { BLUE_SCSI_WIFI_CMD, BLUE_SCSI_WIFI_CMD_SCAN_RESULTS, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	bool success = ExecuteCommand(command, sizeof(command), result, sizeof(*result));
	if ((success) && (IsLogging())) {
		Log("Wifi scan results from the BlueSCSI:");
		for (int i = 0; i < BLUE_SCSI_NUM_NETWORK_ENTRIES; i++) {
			BlueSCSINetworkEntry * entry = &(result->entries[i]);
			if (strlen(entry->ssid) == 0)
				continue;
			Log("  Chan=%2u  RSSI=%u  AUTH=%s  BSSID=%02x:%02x:%02x:%02x:%02x:%02x  SSID=%s",
				entry->channel,
				entry->rssi,
				((entry->flags & BLUE_SCSI_NETWORK_FLAG_AUTH) != 0 ? "Yes" : "No "),
				entry->bssid[0],
				entry->bssid[1],
				entry->bssid[2],
				entry->bssid[3],
				entry->bssid[4],
				entry->bssid[5],
				entry->ssid);
		}
	}
	return success;
}


bool BlueSCSICommand::WifiInfo(BlueSCSINetworkEntry * result)
{
	Log("Getting WiFi info from the BlueSCSI");
	uint8 command[] = { BLUE_SCSI_WIFI_CMD, BLUE_SCSI_WIFI_CMD_INFO, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	bool success = ExecuteCommand(command, sizeof(command), result, sizeof(*result));
	if ((success) && (IsLogging())) {
		Log("Wifi info from the BlueSCSI:");
		Log("  Channel: %u\n", result->channel);
		Log("  SSID:    \"%s\"\n", result->ssid);
		Log("  BSSID:   %02x:%02x:%02x:%02x:%02x:%02x\n",
			result->bssid[0],
			result->bssid[1],
			result->bssid[2],
			result->bssid[3],
			result->bssid[4],
			result->bssid[5]);
		Log("  RSSI:    %u\n", result->rssi);
		Log("  Auth:    %s\n", ((result->flags & BLUE_SCSI_NETWORK_FLAG_AUTH) != 0 ? "Yes" : "No"));
	}
	return success;
}


bool BlueSCSICommand::WifiJoin(BlueSCSINetworkJoinRequest * request)
{
	Log("Joining WiFi SSID \"%s\" at channel %u on the BlueSCSI", request->ssid, (uint32)request->channel);
	uint8 command[] = { BLUE_SCSI_WIFI_CMD, BLUE_SCSI_WIFI_CMD_JOIN, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return ExecuteCommand(command, sizeof(command), request, sizeof(*request));
}