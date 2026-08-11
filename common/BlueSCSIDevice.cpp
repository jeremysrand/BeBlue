#include <stdlib.h>

#include <Path.h>

#include "common/BlueSCSIDevice.h"


// Defines

#define BLUE_SCSI_MODE_SENSE_SIZE 64


// Implementation

BlueSCSIDevice::BlueSCSIDevice(BPath * pathArg,
	BlueSCSIDeviceErrorHandler * errHandlerArg, bool force)
	: isBlueSCSI(false),
	  bus(0),
	  target(0),
	  lun(0),
	  path(pathArg->Path(), NULL, true),
	  logger(errHandlerArg != NULL ? errHandlerArg->GetLogger() : NULL),
	  comm(&path, logger),
	  inquiry(),
	  capabilities(),
	  errHandler(errHandlerArg)
{
	Init(force);
	if (IsLogging()) {
		Log("Device at %s is%s a BlueSCSI", PathString(), (IsBlueSCSI() ? "" : " not"));
		Log("  Bus    = %d", Bus());
		Log("  Target = %d", Target());
		Log("  LUN    = %d", Lun());
	}
}


bool BlueSCSIDevice::IsBlueSCSI()
{
	return isBlueSCSI;
}


int32 BlueSCSIDevice::Bus()
{
	return bus;
}


int32 BlueSCSIDevice::Target()
{
	return target;
}


int32 BlueSCSIDevice::Lun()
{
	return lun;
}


void BlueSCSIDevice::Init(bool force)
{
	if (comm.HasError()) {
		HandleError(comm.GetErrorStr());
		return;
	}
	
	if (!comm.IsBlueSCSIInquiry(&inquiry)) {
		if (comm.HasError())
			HandleError(comm.GetErrorStr());
		if (!force)
			return;
	}
	
	uint8 data[BLUE_SCSI_MODE_SENSE_SIZE];
	if (!comm.IsBlueSCSIModeSense(data, sizeof(data))) {
		if (comm.HasError())
			HandleError(comm.GetErrorStr());
			
		if (IsLogging()) {
			for (int i = 0; i < BLUE_SCSI_MODE_SENSE_SIZE; i += 8) {
				Log("DATA[%02d]: %02x %02x %02x %02x %02x %02x %02x %02x",
					i, (uint32)data[i], (uint32)data[i + 1],
					(uint32)data[i + 2], (uint32)data[i + 3],
					(uint32)data[i + 4], (uint32)data[i + 5],
					(uint32)data[i + 6], (uint32)data[i + 7]);
			}
		}
		if (!force)
			return;
	}
	
	if (!comm.GetCapabilities(&capabilities)) {
		if (comm.HasError())
			HandleError(comm.GetErrorStr());
		if (!force)
			return;
	}
	
	isBlueSCSI = true;
	
	BPath lunPath;
	if (path.GetParent(&lunPath) == B_NO_ERROR) {
		lun = (int32)atoi(lunPath.Leaf());
		
		BPath targetPath;
		if (lunPath.GetParent(&targetPath) == B_NO_ERROR) {
			target = (int32)atoi(targetPath.Leaf());
			
			BPath busPath;
			if (targetPath.GetParent(&busPath) == B_NO_ERROR) {
				bus = (int32)atoi(busPath.Leaf());	
			}
		}
	}
}


void BlueSCSIDevice::HandleError(const char * err)
{
	if (errHandler != NULL)
		errHandler->HandleError(&path, err);
	Log("ERROR: %s", err);
}


void BlueSCSIDevice::SetErrorHandler(BlueSCSIDeviceErrorHandler * errHandlerArg)
{
	errHandler = errHandlerArg;
	if (errHandlerArg != NULL)
		comm.SetLogger(errHandlerArg->GetLogger());
}


void BlueSCSIDevice::Log(const char * fmt, ...)
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


bool BlueSCSIDevice::IsLogging()
{
	return (logger != NULL);
}


const BPath & BlueSCSIDevice::Path()
{
	return path;
}


const char * BlueSCSIDevice::PathString()
{
	return path.Path();
}


BlueSCSICommand & BlueSCSIDevice::Command()
{
	return comm;
}


const SCSIInquiryResult & BlueSCSIDevice::Inquiry()
{
	return inquiry;
}


const BlueSCSICapResult & BlueSCSIDevice::Capabilities()
{
	return capabilities;
}


bool BlueSCSIDevice::SupportsLargeTransfers()
{
	return comm.SupportsLargeTransfers(capabilities);
}


bool BlueSCSIDevice::SupportsLargeSend()
{
	return comm.SupportsLargeSend(capabilities);
}


bool BlueSCSIDevice::SupportsSetWorkingDir()
{
	return comm.SupportsSetWorkingDir(capabilities);
}


bool BlueSCSIDevice::ParsePath(const char * path, char * cwd, char * dir, char * filename)
{
	Log("Parsing path \"%s\"", path);
	size_t pathLen = strlen(path);
	
	if (pathLen == 0) {
		HandleError("A path must have one or more characters in it");
		return false;
	}

	int lastSlashIndex = -1;
	for (int i = pathLen - 1; i >= 0; i--) {
		if (path[i] == '/') {
			lastSlashIndex = i;
			break;
		}
	}
	
	if (lastSlashIndex == pathLen - 1) {
		HandleError("A path must not end in a slash");
		return false;
	}
	
	int startOfFilename = lastSlashIndex + 1;
	if ((pathLen - startOfFilename)	> BLUE_SCSI_MAX_FILE_NAME_LEN) {
		HandleError("File name too long");
		return false;
	}
	
	strcpy(filename, &(path[startOfFilename]));
	
	if (SupportsSetWorkingDir()) {
		if (!comm.GetWorkingDir(cwd, BLUE_SCSI_MAX_WORKING_DIR_LEN))
			return false;
	} else
		cwd[0] = '\0';
	
	if (lastSlashIndex == -1) {
		// No directory in the path so it is a file in the cwd.
		strcpy(dir, cwd);
	} else {
		if (!SupportsSetWorkingDir()) {
			HandleError("Path contains at least one directory but setting cwd is not supported");
			return false;
		}
		if (path[0] == '/') {
			if (lastSlashIndex >= BLUE_SCSI_MAX_WORKING_DIR_LEN) {
				HandleError("The directory portion of the path is too long");
				return false;
			}
			if (lastSlashIndex == 0)
				lastSlashIndex++;
			memcpy(dir, path, lastSlashIndex);
			dir[lastSlashIndex] = '\0';
		} else {
			size_t cwdLen = strlen(cwd);
			if (cwdLen + lastSlashIndex + 1 >= BLUE_SCSI_MAX_WORKING_DIR_LEN) {
				HandleError("The directory portion of the path is too long");
				return false;
			}
			strcpy(dir, cwd);
			char * ptr = &(dir[cwdLen - 1]);
			if (*ptr != '/')
				ptr++;
			*ptr = '/';
			ptr++;
			
			memcpy(ptr, path, lastSlashIndex);
			ptr += lastSlashIndex;
			*ptr = '\0';
		}
	}
	
	if (IsLogging()) {
		Log("Parsed path \"%s\"", path);
		Log("  cwd      = \"%s\"", cwd);
		Log("  dir      = \"%s\"", dir);
		Log("  filename = \"%s\"", filename);
	}	
	
	return true;
}