#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scsi.h>
#include <CAM.h>

#include "Common.h"
#include "SCSICommand.h"


#define SCSI_TIMEOUT 1000000

#define SCSI_INQUIRY 0x12

#define SCSI_INQ_TYPE_MASK 0x1f
#define SCSI_INQ_VENDOR_OFFSET 8
#define SCSI_INQ_DEVICE_OFFSET 16
#define SCSI_INQ_VERSION_OFFSET 32



SCSICommand::SCSICommand(int fd)
	: fd(fd),
	  errorStr(NULL)
{
}


SCSICommand::~SCSICommand()
{
	if (errorStr != NULL) {
		delete[](errorStr);
	}
}


const char * SCSICommand::GetErrorStr()
{
	return errorStr;
}


const uint8 * SCSICommand::GetSense()
{
	return sense;
}


void SCSICommand::RaiseError(const char * str1, const char * str2)
{
	size_t len = strlen(str1);
	if (str2 != NULL) {
		len += 2 + strlen(str2);
	}
	
	if (errorStr != NULL) {
		delete[](errorStr);
	}
	
	errorStr = new(char[len]);
	if (str2 != NULL) {
		sprintf(errorStr, "%s: %s", str1, str2);
	} else {
		sprintf(errorStr, "%s", str1);
	}
}


void SCSICommand::RaiseError(const char * str1, int errnum)
{
	RaiseError(str1, strerror(errnum));
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


bool SCSICommand::ExecuteCommand(uint8 * command, uint8 command_len, void * data, size_t data_len)
{
	raw_device_command rdc;
	int e;
	
	rdc.data = data;
	rdc.data_length = data_len;
	rdc.sense_data = sense;
	rdc.sense_data_length = 0;
	rdc.timeout = SCSI_TIMEOUT;
	rdc.flags = B_RAW_DEVICE_DATA_IN;
	rdc.command_length = command_len;
	
	if (command_len > sizeof(rdc.command)) {
		RaiseError(FormatError("Commands was %u bytes long must be no more than %u bytes", (uint32)command_len, (uint32)sizeof(rdc.command)));
		return false;
	}
	memcpy(rdc.command, command, command_len);
	
	memset(rdc.sense_data, 0, sizeof(rdc.sense_data));
	
	e = ioctl(fd, B_RAW_DEVICE_COMMAND, &rdc, sizeof(rdc));
	if (e != 0) {
		RaiseError("Error from raw command of device");
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
	
	if (!ExecuteCommand(command, sizeof(command), &data, sizeof(data))) {
		return false;
	}
	
	result->type = data.inquiry_data[0] & SCSI_INQ_TYPE_MASK;
	result->typeStr = typeStrings[(result->type > NUM_ELEMS(typeStrings) ? NUM_ELEMS(typeStrings) : result->type)];
	
	copyString(result->vendor, &(data.inquiry_data[SCSI_INQ_VENDOR_OFFSET]), SCSI_INQ_VENDOR_LEN);
	copyString(result->device, &(data.inquiry_data[SCSI_INQ_DEVICE_OFFSET]), SCSI_INQ_DEVICE_LEN);
	copyString(result->version, &(data.inquiry_data[SCSI_INQ_VERSION_OFFSET]), SCSI_INQ_VERSION_LEN);
	
	return true;
}
