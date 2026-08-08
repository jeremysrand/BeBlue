#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scsi.h>
#include <CAM.h>

#include "common/Common.h"
#include "common/SCSICommand.h"


// Defines

#define SCSI_TIMEOUT 1000000

#define SCSI_INQUIRY 0x12
#define SCSI_INQ_TYPE_MASK 0x1f
#define SCSI_INQ_TYPE_OFFSET 0
#define SCSI_INQ_SCSI_VERSION_OFFSET 2
#define SCSI_INQ_VENDOR_STR_OFFSET 8
#define SCSI_INQ_DEVICE_STR_OFFSET 16
#define SCSI_INQ_VERSION_STR_OFFSET 32

#define SCSI_MODE_SENSE 0x1a


// Implementation

SCSICommand::SCSICommand(BPath * path, Logger * logger)
	: fd(-1),
	  errorStr(NULL),
	  logger(logger)
{
	const char * dev = path->Path();
	if ((fd = open(dev, 0)) < 0) {
		RaiseError(FormatError("Unable to open dev %s, %s", dev, strerror(errno)));
		return;
	}
}


SCSICommand::~SCSICommand()
{
	if (errorStr != NULL) {
		delete[](errorStr);
	}
	if (fd >= 0)
		close(fd);
}


void SCSICommand::SetLogger(Logger * arg)
{
	logger = arg;
}


bool SCSICommand::HasError() {
	return errorStr != NULL;
}


const char * SCSICommand::GetErrorStr()
{
	return errorStr;
}


const uint8 * SCSICommand::GetSense()
{
	return sense;
}


void SCSICommand::RaiseError(const char * str)
{
	if (errorStr != NULL) {
		delete[](errorStr);
	}
	
	errorStr = new(char[strlen(str)]);
	strcpy(errorStr, str);
	Log("ERROR: %s", errorStr);
}


void SCSICommand::RaiseError(const char * str, int errnum)
{
	RaiseError(FormatError("%s: %s", str, strerror(errnum)));
}


const char * SCSICommand::FormatError(const char * fmt, ...)
{
	// This function is not thread safe!!!  Nor is there any protection for
	// buffer overflow!!  There doesn't seem to be any snprintf() functions
	// under PPC BeOS.  I guess if I want to make this thread safe and buffer
	// overflow safe, I should use sstream or something like that from C++.
	//
	// Could make this slightly better by putting the error buffer into the
	// SCSICommand instance.  At least then, there is only a race if a single
	// command is being used from multiple threads.
	
	static char buffer[1024];
	va_list args;
	
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);
	
	return buffer;
}


void SCSICommand::Log(const char * fmt, ...)
{
	if (!IsLogging())
		return;
	
	char timestamp[LOGGER_TIMESTAMP_LEN];
	logger->FormatTimestamp(timestamp);
	
	va_list args;
	
	va_start(args, fmt);
	logger->Log(timestamp, fmt, args);
	va_end(args);
}


bool SCSICommand::IsLogging()
{
	return (logger != NULL);
}


void SCSICommand::LogCommand(uint8 * command, uint8 commandLen, size_t dataLen)
{
	Log("SCSI command with commandLen = %u, dataLen = %u", commandLen, dataLen);
	switch (commandLen) {
		case 0:
			break;
			
		case 1:
			Log("  Command: %02x",
				(uint32)command[0]);
			break;
			
		case 2:
			Log("  Command: %02x %02x",
				(uint32)command[0], (uint32)command[1]);
			break;
			
		case 3:
			Log("  Command: %02x %02x %02x",
				(uint32)command[0], (uint32)command[1], (uint32)command[2]);
			break;
			
		case 4:
			Log("  Command: %02x %02x %02x %02x",
				(uint32)command[0], (uint32)command[1], (uint32)command[2], (uint32)command[3]);
			break;
			
		case 5:
			Log("  Command: %02x %02x %02x %02x %02x",
				(uint32)command[0], (uint32)command[1], (uint32)command[2], (uint32)command[3],
				(uint32)command[4]);
			break;
			
		case 6:
			Log("  Command: %02x %02x %02x %02x %02x %02x",
				(uint32)command[0], (uint32)command[1], (uint32)command[2], (uint32)command[3],
				(uint32)command[4], (uint32)command[5]);
			break;
			
		case 7:
			Log("  Command: %02x %02x %02x %02x %02x %02x %02x",
				(uint32)command[0], (uint32)command[1], (uint32)command[2], (uint32)command[3],
				(uint32)command[4], (uint32)command[5], (uint32)command[6]);
			break;
			
		case 8:
			Log("  Command: %02x %02x %02x %02x %02x %02x %02x %02x",
				(uint32)command[0], (uint32)command[1], (uint32)command[2], (uint32)command[3],
				(uint32)command[4], (uint32)command[5], (uint32)command[6], (uint32)command[7]);
			break;
			
		case 9:
			Log("  Command: %02x %02x %02x %02x %02x %02x %02x %02x %02x",
				(uint32)command[0], (uint32)command[1], (uint32)command[2], (uint32)command[3],
				(uint32)command[4], (uint32)command[5], (uint32)command[6], (uint32)command[7],
				(uint32)command[8]);
			break;
			
		case 10:
			Log("  Command: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
				(uint32)command[0], (uint32)command[1], (uint32)command[2], (uint32)command[3],
				(uint32)command[4], (uint32)command[5], (uint32)command[6], (uint32)command[7],
				(uint32)command[8], (uint32)command[9]);
			break;
		
		default:
			Log("  Command: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x...",
				(uint32)command[0], (uint32)command[1], (uint32)command[2], (uint32)command[3],
				(uint32)command[4], (uint32)command[5], (uint32)command[6], (uint32)command[7],
				(uint32)command[8], (uint32)command[9]);
			break;
	}
}


bool SCSICommand::ExecuteCommand(uint8 * command, uint8 commandLen, void * data, size_t dataLen)
{
	raw_device_command rdc;
	int e;
	
	if (errorStr != NULL) {
		delete[](errorStr);
		errorStr = NULL;
	}
	LogCommand(command, commandLen, dataLen);
	
	rdc.data = data;
	rdc.data_length = dataLen;
	rdc.sense_data = sense;
	rdc.sense_data_length = 0;
	rdc.timeout = SCSI_TIMEOUT;
	rdc.flags = B_RAW_DEVICE_DATA_IN;
	rdc.command_length = commandLen;
	
	if (commandLen > sizeof(rdc.command)) {
		RaiseError(FormatError("Commands was %u bytes long must be no more than %u bytes", (uint32)commandLen, (uint32)sizeof(rdc.command)));
		return false;
	}
	memcpy(rdc.command, command, commandLen);
	
	memset(rdc.sense_data, 0, sizeof(rdc.sense_data));
	
	e = ioctl(fd, B_RAW_DEVICE_COMMAND, &rdc, sizeof(rdc));
	if (e != 0) {
		RaiseError(FormatError("Error from raw command of device: %s", strerror(errno)));
		return false;
	}
	
	if (rdc.cam_status != CAM_REQ_CMP) {
		RaiseError(FormatError("Expected CAM status %u but got %u", (uint32)CAM_REQ_CMP, (uint32)rdc.cam_status));
		return false;
	}
	
	if (rdc.scsi_status != 0) {
		RaiseError(FormatError("Expected SCSI status 0 but got %u", (uint32)rdc.scsi_status));
		return false;
	}
	
	Log("SCSICommand succeeded");
	return true;
}


static void copyString(char * dst, uint8 * src, uint32 maxLen) {
	memcpy(dst, src, maxLen);
	dst[maxLen] = '\0';
	// This loop removes any trailing whitespace from the string.
	for (char * ptr = dst + maxLen - 1; ptr >= dst; ptr--) {
		if (isspace(*ptr))
			*ptr = '\0';
		else
			return;
	}
}


bool SCSICommand::Inquiry(SCSIInquiryResult * result)
{
	static char * typeStrings[] = {
		"Disk",
		"Tape",
		"Printer",
		"CPU",
		"WORM",
		"CD-ROM",
		"Scanner",
		"Optical",
		"Changer",
		"Comm",
		"Unknown"
	};
	scsi_inquiry data;
	uint8 command[] = { SCSI_INQUIRY, 0x00, 0x00, 0x00, sizeof(data), 0x00 };
	
	Log("Execute Inquiry command");
	if (!ExecuteCommand(command, sizeof(command), &data, sizeof(data))) {
		return false;
	}
	
	result->type = data.inquiry_data[SCSI_INQ_TYPE_OFFSET] & SCSI_INQ_TYPE_MASK;
	result->typeStr = typeStrings[(result->type > NUM_ELEMS(typeStrings) ?
		NUM_ELEMS(typeStrings) : result->type)];
		
	result->scsiVersion = data.inquiry_data[SCSI_INQ_SCSI_VERSION_OFFSET];
	
	copyString(result->vendorStr, &(data.inquiry_data[SCSI_INQ_VENDOR_STR_OFFSET]), SCSI_INQ_VENDOR_STR_LEN);
	copyString(result->deviceStr, &(data.inquiry_data[SCSI_INQ_DEVICE_STR_OFFSET]), SCSI_INQ_DEVICE_STR_LEN);
	copyString(result->versionStr, &(data.inquiry_data[SCSI_INQ_VERSION_STR_OFFSET]), SCSI_INQ_VERSION_STR_LEN);
	
	Log("Inquiry successful");
	Log("  type    = %s (%u)", result->typeStr, (uint32)result->type);
	Log("  vendor  = %s", result->vendorStr);
	Log("  device  = %s", result->deviceStr);
	Log("  version = %s", result->versionStr);
	
	return true;
}


bool SCSICommand::ModeSense(uint8 page, uint8 * data, uint8 dataLen)
{
	uint8 command[] = { SCSI_MODE_SENSE, 0x00, page, 0x00, dataLen, 0x00 };
	Log("Execute ModeSense command");
	return ExecuteCommand(command, sizeof(command), data, dataLen);
}
