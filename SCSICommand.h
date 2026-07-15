#ifndef SCSI_COMMAND_H
#define SCSI_COMMAND_H


#define SCSI_SENSE_SIZE 16


#define SCSI_INQ_VENDOR_LEN 8
#define SCSI_INQ_DEVICE_LEN 16
#define SCSI_INQ_VERSION_LEN 4


struct SCSIInquiryResult {
	uint8 type;
	const char * typeStr;
	char vendor[SCSI_INQ_VENDOR_LEN + 1];
	char device[SCSI_INQ_DEVICE_LEN + 1];
	char version[SCSI_INQ_VERSION_LEN + 1];
};


class SCSICommand {
	public:
		SCSICommand(int fd);
		~SCSICommand();
	
		const char * GetErrorStr();
		const uint8 * GetSense();
		
		bool Inquiry(SCSIInquiryResult * result);
		
	private:
		bool ExecuteCommand(uint8 * command, uint8 command_len, void * data, size_t data_len);
		
		void RaiseError(const char * str1, const char * str2 = NULL);
		void RaiseError(const char * str, int errnum);
		
		const char * FormatError(const char * fmt, ...);

	private:
		int fd;
		uint8 sense[SCSI_SENSE_SIZE];
		char * errorStr;		
		
};

#endif