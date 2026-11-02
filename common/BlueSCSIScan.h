#include <Directory.h>
#include <List.h>
#include <Path.h>

#include "common/BlueSCSIDevice.h"

#ifndef BLUE_SCSI_SCAN_H
#define BLUE_SCSI_SCAN_H


// Macros

#define SCSI_SCAN_DEVICE_TYPE(t) (1u << (t))
#define SCSI_SCAN_DEVICE_ALL \
	SCSI_SCAN_DEVICE_TYPE(SCSI_DEVICE_TYPE_DISK)    | \
	SCSI_SCAN_DEVICE_TYPE(SCSI_DEVICE_TYPE_TAPE)    | \
	SCSI_SCAN_DEVICE_TYPE(SCSI_DEVICE_TYPE_PRINTER) | \
	SCSI_SCAN_DEVICE_TYPE(SCSI_DEVICE_TYPE_CPU)     | \
	SCSI_SCAN_DEVICE_TYPE(SCSI_DEVICE_TYPE_WORM)    | \
	SCSI_SCAN_DEVICE_TYPE(SCSI_DEVICE_TYPE_CDROM)   | \
	SCSI_SCAN_DEVICE_TYPE(SCSI_DEVICE_TYPE_SCANNER) | \
	SCSI_SCAN_DEVICE_TYPE(SCSI_DEVICE_TYPE_OPTICAL) | \
	SCSI_SCAN_DEVICE_TYPE(SCSI_DEVICE_TYPE_CHANGER) | \
	SCSI_SCAN_DEVICE_TYPE(SCSI_DEVICE_TYPE_COMM)
#define SCSI_SCAN_IS_DEVICE_TYPE_SET(mask, t) \
	(((mask) & (SCSI_SCAN_DEVICE_TYPE(t))) != 0 ? true : false)


// Forward declarations

class Logger;


// Interface

typedef uint32 tSCSIDeviceMask;


class BlueSCSIScan {
	public:
		BlueSCSIScan(BlueSCSIDeviceErrorHandler * errHandler = NULL,
			tSCSIDeviceMask deviceMask = SCSI_SCAN_DEVICE_ALL);
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
		tSCSIDeviceMask deviceMask;
};


#endif
