#include <Entry.h>

#ifndef BLUE_SCSI_GET_H
#define BLUE_SCSI_GET_H


// Forward declarations

class BlueSCSIDevice;
class BlueSCSICommand;
struct BlueSCSIFileEntry;


// Interface

class BlueSCSIGetErrorHandler {
	public:
		virtual void HandleGetError(const char * err, status_t status) = 0;
};


class BlueSCSIGet {
	public:
		BlueSCSIGet(BlueSCSIDevice & deviceArg, BlueSCSIGetErrorHandler * errHandlerArg = NULL);
		~BlueSCSIGet();
		
		bool SetSrc(const char * src);
		void SetRecurse(bool arg);
		void SetForce(bool arg);
		void SetDest(BEntry * arg);
		
		const char * Filename();
		
		bool Get();
		
	private:
		bool GetFromRightDir();
		bool GetFile(const BlueSCSIFileEntry * fileEntry, BEntry * entry);
		bool GetDir(const BlueSCSIFileEntry * fileEntry, BEntry * entry);
		
		status_t RaiseError(const char * err, status_t status);
		void HandleGetError(const char * err, status_t status = B_NO_ERROR);
		
	private:
		BlueSCSIDevice & device;
		BlueSCSICommand & comm;
		
		char dir[BLUE_SCSI_MAX_WORKING_DIR_LEN];
		char filename[BLUE_SCSI_MAX_FILE_NAME_LEN + 1];
		
		bool recurse;
		bool force;
		
		BEntry * dest;
		
		char * buffer;
		uint32 bufferSize;
		
		BlueSCSIGetErrorHandler *errHandler;
};

#endif