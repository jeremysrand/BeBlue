#include <Path.h>

#include "common/BlueSCSICommand.h"

#ifndef BLUE_SCSI_DEVICE_H
#define BLUE_SCSI_DEVICE_H


// Interface

class Logger;

class BlueSCSIDeviceErrorHandler {
	public:
		virtual void HandleError(BPath * path, const char * err) = 0;
		virtual Logger * GetLogger() = 0;
};

class BlueSCSIDevice
{
	public:
		BlueSCSIDevice(BPath * path, BlueSCSIDeviceErrorHandler * errHandler = NULL);
		
		bool IsBlueSCSI();
		
		const BPath & Path();
		const char * PathString();
		
		int32 Bus();
		int32 Target();
		int32 Lun();
		
		BlueSCSICommand & Command();
		
		const SCSIInquiryResult & Inquiry();
		
		const BlueSCSICapResult & Capabilities();
		bool SupportsLargeTransfers();
		bool SupportsLargeSend();
		bool SupportsSetWorkingDir();
		
		void SetErrorHandler(BlueSCSIDeviceErrorHandler * errHandlerArg);
		
		bool ParsePath(const char * path, char * cwd, char * dir, char * filename);

		void HandleError(const char * err);
		
		void Log(const char * fmt, ...);
		bool IsLogging();
		
	private:
		void Init();

	private:
		bool isBlueSCSI;
		int32 bus;
		int32 target;
		int32 lun;
		BPath path;
		Logger * logger;
		BlueSCSICommand comm;
		SCSIInquiryResult inquiry;
		BlueSCSICapResult capabilities;
		BlueSCSIDeviceErrorHandler * errHandler;
};

#endif