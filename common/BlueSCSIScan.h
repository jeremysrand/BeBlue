#include <Directory.h>
#include <List.h>
#include <Path.h>

#include "common/BlueSCSIDevice.h"

#ifndef BLUE_SCSI_SCAN_H
#define BLUE_SCSI_SCAN_H


// Interface

class BlueSCSIScan {
	public:
		BlueSCSIScan(BlueSCSIDeviceErrorHandler * errHandler = NULL);
		~BlueSCSIScan();
		
		int32 NumDevices();
		BlueSCSIDevice * DeviceAt(int32 index);
		
	private:
		void Scan();
		void Walk(BDirectory * dir);
		void CheckIfBlueSCSI(BPath * path);
		
	private:
		BList deviceList;
		BlueSCSIDeviceErrorHandler * errHandler;
};


#endif
