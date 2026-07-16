#ifndef SCSI_COMMAND_H
#define SCSI_COMMAND_H


// Defines

#define SCSI_SENSE_SIZE 16

#define SCSI_INQ_VENDOR_STR_LEN 8
#define SCSI_INQ_DEVICE_STR_LEN 16
#define SCSI_INQ_VERSION_STR_LEN 4

#define SCSI_MODE_SENSE_BLOCK_DESC_LEN_OFFSET 3
#define SCSI_MODE_SENSE_HEADER_SIZE 4


// Interface

struct SCSIInquiryResult {
	uint8 type;
	uint8 scsiVersion;
	const char * typeStr;
	char vendorStr[SCSI_INQ_VENDOR_STR_LEN + 1];
	char deviceStr[SCSI_INQ_DEVICE_STR_LEN + 1];
	char versionStr[SCSI_INQ_VERSION_STR_LEN + 1];
};


class SCSICommand {
	public:
		SCSICommand(int fd);
		~SCSICommand();
	
		bool HasError();
		const char * GetErrorStr();
		const uint8 * GetSense();
		
		bool Inquiry(SCSIInquiryResult * result);
		bool ModeSense(uint8 page, uint8 * data, uint8 dataLen);
		
	protected:
		bool ExecuteCommand(uint8 * command, uint8 commandLen, void * data, size_t dataLen);
		
		void RaiseError(const char * str);
		void RaiseError(const char * str, int errnum);
		
		const char * FormatError(const char * fmt, ...);

	private:
		int fd;
		uint8 sense[SCSI_SENSE_SIZE];
		char * errorStr;		
		
};

#endif