#include <Directory.h>
#include <List.h>
#include <Path.h>

#include "common/BlueSCSIDevice.h"

#ifndef BLUE_SCSI_SCAN_H
#define BLUE_SCSI_SCAN_H


// Forward declarations

class Logger;


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
		
		void Log(const char * fmt, ...);
		bool IsLogging();
		
	private:
		BList deviceList;
		BlueSCSIDeviceErrorHandler * errHandler;
		Logger * logger;
};


#endif
