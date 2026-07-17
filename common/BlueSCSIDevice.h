#include <Path.h>

#include "common/BlueSCSICommand.h"

#ifndef BLUE_SCSI_DEVICE_H
#define BLUE_SCSI_DEVICE_H


// Interface

class BlueSCSIDeviceErrorHandler {
	public:
		virtual void HandleError(BPath * path, const char * err) = 0;
};

class BlueSCSIDevice
{
	public:
		BlueSCSIDevice(BPath * path, BlueSCSIDeviceErrorHandler * errHandler = NULL);
		
		bool IsBlueSCSI();
		
		const BPath & Path();
		const char * PathString();
		
		BlueSCSICommand & Command();
		
		const SCSIInquiryResult & Inquiry();
		
		const BlueSCSICapResult & Capabilities();
		bool SupportsLargeTransfers();
		bool SupportsLargeSend();
		bool SupportsSetWorkingDir();
		
		void SetErrorHandler(BlueSCSIDeviceErrorHandler * errHandlerArg);
		

	private:
		void Init();
		void HandleError(const char * err);

	private:
		bool isBlueSCSI;
		BPath path;
		BlueSCSICommand comm;
		SCSIInquiryResult inquiry;
		BlueSCSICapResult capabilities;
		BlueSCSIDeviceErrorHandler * errHandler;
};

#endif