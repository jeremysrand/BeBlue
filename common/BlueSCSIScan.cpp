#include "common/BlueSCSICommand.h"
#include "common/BlueSCSIScan.h"


// Defines

#define SCSI_BUS "/dev/bus/scsi"
#define SCSI_RAW_DEV_NAME "raw"


// Implementation

BlueSCSIScan::BlueSCSIScan(BlueSCSIDeviceErrorHandler * errHandler)
	: errHandler(errHandler)
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
	Walk(&dir);
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
			    	CheckIfBlueSCSI(&path);
			}
		}
	}
}


void BlueSCSIScan::CheckIfBlueSCSI(BPath * path)
{
	BlueSCSIDevice * device = new BlueSCSIDevice(path, errHandler);
	if (device->IsBlueSCSI())
		deviceList.AddItem(device);
	else
		delete device;
}
