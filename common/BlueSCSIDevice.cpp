#include <stdlib.h>

#include <Path.h>

#include "common/BlueSCSIDevice.h"


// Defines

#define BLUE_SCSI_MODE_SENSE_SIZE 128


// Implementation

BlueSCSIDevice::BlueSCSIDevice(BPath * pathArg, BlueSCSIDeviceErrorHandler * errHandlerArg)
	: isBlueSCSI(false),
	  bus(0),
	  target(0),
	  lun(0),
	  path(pathArg->Path(), NULL, true),
	  comm(&path),
	  inquiry(),
	  capabilities(),
	  errHandler(errHandlerArg)
{
	Init();
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


void BlueSCSIDevice::Init()
{
	if (comm.HasError()) {
		HandleError(comm.GetErrorStr());
		return;
	}
	
	if (!comm.IsBlueSCSIInquiry(&inquiry)) {
		if (comm.HasError())
			HandleError(comm.GetErrorStr());
		return;
	}
	
	uint8 data[BLUE_SCSI_MODE_SENSE_SIZE];
	if (!comm.IsBlueSCSIModeSense(data, sizeof(data))) {
		if (comm.HasError())
			HandleError(comm.GetErrorStr());
		return;
	}
	
	if (!comm.GetCapabilities(&capabilities)) {
		if (comm.HasError())
			HandleError(comm.GetErrorStr());
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
}


void BlueSCSIDevice::SetErrorHandler(BlueSCSIDeviceErrorHandler * errHandlerArg)
{
	errHandler = errHandlerArg;
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