#include "common/BlueSCSIDevice.h"


// Defines

#define BLUE_SCSI_MODE_SENSE_SIZE 128


// Implementation

BlueSCSIDevice::BlueSCSIDevice(BPath * pathArg, BlueSCSIDeviceErrorHandler * errHandlerArg)
	: isBlueSCSI(false),
	  path(*pathArg),
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