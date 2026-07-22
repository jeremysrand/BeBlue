#include <Entry.h>

#ifndef BLUE_SCSI_SEND_H
#define BLUE_SCSI_SEND_H


// Forward declarations

class BlueSCSIDevice;
class BlueSCSICommand;


// Interfaces

class BlueSCSISendErrorHandler {
	public:
		virtual void HandleSendError(const char * err, status_t status) = 0;
};


class BlueSCSISend {
	public:
		BlueSCSISend(BlueSCSIDevice & deviceArg, BlueSCSISendErrorHandler * errHandlerArg = NULL);
		~BlueSCSISend();
		
		
		void SetSrc(BEntry * arg);
		bool SetRecurse(bool arg);
		bool SetDest(const char * arg);
		
		bool Send();
		
	private:
		status_t RaiseError(const char * err, status_t status);
		void HandleSendError(const char * err, status_t status = B_NO_ERROR);
		
		bool SendToRightDir();
		bool SendFile(BEntry * entry, bool useExistingFilename = false);
		bool SendDir(BEntry * entry, bool useExistingFilename = false);
		
		bool SetFilenameFromEntry(BEntry * entry, bool useExistingFilename);
		
	private:
		BlueSCSIDevice & device;
		BlueSCSICommand & comm;
		
		char dir[BLUE_SCSI_MAX_WORKING_DIR_LEN];
		char filename[BLUE_SCSI_MAX_FILE_NAME_LEN + 1];
		char beFilename[B_FILE_NAME_LENGTH];
		
		bool recurse;
		bool supportsBulk;
		
		BEntry * src;
		
		char * buffer;
		uint32 bufferSize;
		
		BlueSCSISendErrorHandler *errHandler;
};

#endif