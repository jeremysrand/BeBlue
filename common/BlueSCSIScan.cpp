#include "common/BlueSCSICommand.h"
#include "common/BlueSCSIScan.h"
#include "common/Logger.h"


// Defines

#define SCSI_BUS "/dev/bus/scsi"
#define SCSI_DISKS "/dev/disk/scsi"
#define SCSI_RAW_DEV_NAME "raw"


// Implementation

BlueSCSIScan::BlueSCSIScan(BlueSCSIDeviceErrorHandler * errHandlerArg)
	: errHandler(errHandlerArg),
	  logger(errHandlerArg != NULL ? errHandlerArg->GetLogger() : NULL)
{
	Scan();
}


BlueSCSIScan::~BlueSCSIScan()
{
	for (int32 i = 0; i < NumDevices(); i++) {
		BlueSCSIDevice * device = DeviceAt(i);
		delete device;
	}
}


void BlueSCSIScan::Log(const char * fmt, ...)
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


bool BlueSCSIScan::IsLogging()
{
	return (logger != NULL);
}


int32 BlueSCSIScan::NumDevices()
{
	return deviceList.CountItems();
}


BlueSCSIDevice * BlueSCSIScan::DeviceAt(int32 index)
{
	return (BlueSCSIDevice *)deviceList.ItemAt(index);
}


void BlueSCSIScan::Scan()
{
	BDirectory dir(SCSI_BUS);
	if (dir.InitCheck() == B_NO_ERROR) {
		Log("Scanning the %s directory for devices", SCSI_BUS);
		Walk(&dir);
	} else {
		Log("Unable to scan the %s directory for devices", SCSI_BUS);
		dir.SetTo(SCSI_DISKS);
		if (dir.InitCheck() == B_NO_ERROR) {
			Log("Scanning the %s directory for devices", SCSI_DISKS);
			Walk(&dir);
		} else {
			Log("Unable to scan the %s directory for devices", SCSI_DISKS);
			Log("No SCSI devices can be found");
		}
	}
}


void BlueSCSIScan::Walk(BDirectory * dir)
{
	if (dir->InitCheck() != B_OK)
		return;
		
	BEntry entry;
	while (dir->GetNextEntry(&entry) >= 0) {
		if (entry.IsDirectory()) {
			BDirectory subdir(&entry);
			Walk(&subdir);
		} else {
			BPath path;
			if ((entry.GetPath(&path) == B_OK) &&
			    (strcmp(path.Leaf(), SCSI_RAW_DEV_NAME) == 0)) {
			    Log("Checking if %s is a BlueSCSI", path.Path());
			    CheckIfBlueSCSI(&path);
			}
		}
	}
}


void BlueSCSIScan::CheckIfBlueSCSI(BPath * path)
{
	BlueSCSIDevice * device = new BlueSCSIDevice(path, errHandler);
	if (device->IsBlueSCSI()) {
		Log("Device at %s is a BlueSCSI, adding it to the list of devices",
			path->Path());
		deviceList.AddItem(device);
	} else {
		Log("Device at %s is not a BlueSCSI, ignoring", path->Path());
		delete device;
	}
}
