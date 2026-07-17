#include <Path.h>

#include "common/SCSICommand.h"

#ifndef BLUE_SCSI_COMMAND_H
#define BLUE_SCSI_COMMAND_H


// Defines

#define BLUE_SCSI_MAX_FILE_NAME_LEN 32
#define BLUE_SCSI_FILE_SIZE_BYTES 5

#define BLUE_SCSI_FILE_TYPE 1
#define BLUE_SCSI_DIR_TYPE  0

#define BLUE_SCSI_MAX_DEVICES 8

#define BLUE_SCSI_DEVICE_FIXED_DISK           0x00
#define BLUE_SCSI_DEVICE_REMOVABLE_DISK       0x01
#define BLUE_SCSI_DEVICE_OPTICAL_DISK         0x02
#define BLUE_SCSI_DEVICE_FLOPPY_DISK          0x03
#define BLUE_SCSI_DEVICE_MAGNETO_OPTICAL_DISK 0x04
#define BLUE_SCSI_DEVICE_TAPE_DEVICE          0x05
#define BLUE_SCSI_DEVICE_NETWORK_DEVICE       0x06
#define BLUE_SCSI_DEVICE_ZIP_DISK             0x07
#define BLUE_SCSI_DEVICE_NO_DEVICE            0xff

#define BLUE_SCSI_GET_FILE_BLOCK_SIZE 4096
#define BLUE_SCSI_SEND_FILE_BLOCK_SIZE 512

#define BLUE_SCSI_SSID_LEN 64
#define BLUE_SCSI_KEY_LEN 64
#define BLUE_SCSI_BSSID_LEN 6

#define BLUE_SCSI_NETWORK_FLAG_AUTH 0x1

#define BLUE_SCSI_NUM_NETWORK_ENTRIES 10

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

struct BlueSCSIListDevsResult {
	uint8 devices[BLUE_SCSI_MAX_DEVICES];
};

struct BlueSCSINetworkEntry {
	char ssid[BLUE_SCSI_SSID_LEN];
	char bssid[BLUE_SCSI_BSSID_LEN];
	int8 rssi;
	uint8 channel;
	uint8 flags;
	uint8 padding;
};

struct BlueSCSINetworkEntries {
	BlueSCSINetworkEntry entries[BLUE_SCSI_NUM_NETWORK_ENTRIES];
};

struct BlueSCSINetworkJoinRequest {
	char ssid[BLUE_SCSI_SSID_LEN];
	char key[BLUE_SCSI_KEY_LEN];
	uint8 channel;
	uint8 padding;
};

class BlueSCSICommand : public SCSICommand {
	public:
		BlueSCSICommand(BPath * path);
		~BlueSCSICommand();
		
		bool IsBlueSCSIInquiry(SCSIInquiryResult * result);
		
		bool IsBlueSCSIModeSense(uint8 * data, uint8 dataLen);
		
		bool GetCapabilities(BlueSCSICapResult * result);
		bool SupportsLargeTransfers(const BlueSCSICapResult & result);
		bool SupportsLargeSend(const BlueSCSICapResult & result);
		bool SupportsSetWorkingDir(const BlueSCSICapResult & result);
		
		bool ListDevices(BlueSCSIListDevsResult * result);
		
		bool GetDebug(BlueSCSIDebugResult *result);
		bool SetDebug(bool enabled);
		
		bool GetWorkingDir(char * path, uint8 maxPathLen);
		bool SetWorkingDir(char * path, uint8 maxPathLen);
		
		bool CountFiles(uint8 * result);
		bool ListFiles(BlueSCSIFileEntry * fileEntries, uint8 maxEntries);
		uint64 GetFileSize(const BlueSCSIFileEntry & fileEntry);
		uint32 GetFileNumBlocks(const BlueSCSIFileEntry & fileEntry);
		
		bool GetFile(uint8 fileIndex, uint32 blockOffset, char * buffer,
			size_t bufferSize);
			
		bool PrepareToSendFile(const char * filename);
		bool SendFileBulk(uint32 blockOffset, char * buffer, size_t bufferSize);
		bool SendFileBytes(uint32 blockOffset, char * buffer, size_t bufferSize);
		bool SendFileEnd();
		
		bool CountCDs(uint8 * result);
		bool ListCDs(BlueSCSIFileEntry * fileEntries, uint8 maxEntries);
		bool SetNextCD(uint8 fileIndex);
		
		bool StartWifiScan(bool * started);
		bool CheckWifiScanComplete(bool * completed);
		bool WifiScanResults(BlueSCSINetworkEntries * result);
		bool WifiInfo(BlueSCSINetworkEntry * result);
		bool WifiJoin(BlueSCSINetworkJoinRequest * request);
		
	private:
		bool ToolboxMetadata(uint8 subcommand, uint8 * data, uint8 dataLen);
};

#endif
