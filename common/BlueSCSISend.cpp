#include <string.h>

#include "common/BlueSCSIDevice.h"
#include "common/BlueSCSISend.h"


// Implementation

BlueSCSISend::BlueSCSISend(BlueSCSIDevice & deviceArg, BlueSCSISendErrorHandler * errHandlerArg) :
	device(deviceArg),
	comm(deviceArg.Command()),
	recurse(false),
	supportsBulk(device.SupportsLargeSend()),
	src(NULL),
	buffer(NULL),
	bufferSize(0),
	errHandler(errHandlerArg)
{
	memset(dir, 0, sizeof(dir));
	memset(filename, 0, sizeof(filename));
	memset(beFilename, 0, sizeof(beFilename));
	
	if (supportsBulk) {
		bufferSize = BLUE_SCSI_FILE_MAX_BLOCKS_PER_TRANSFER * BLUE_SCSI_SEND_FILE_BLOCK_SIZE;
	} else {
		bufferSize = BLUE_SCSI_SEND_FILE_MAX_BYTES;
	}
	buffer = new char[bufferSize];
}

BlueSCSISend::~BlueSCSISend()
{
	delete[] buffer;
}


void BlueSCSISend::SetSrc(BEntry * arg)
{
	src = arg;
}


bool BlueSCSISend::SetRecurse(bool arg)
{
	if ((arg) && (!device.SupportsSetWorkingDir())) {
		HandleSendError("Cannot enable recurse because the BlueSCSI does not support changing the working dir");
		return false;
	}
	recurse = arg;
	return true;
}


bool BlueSCSISend::SetDest(const char * dest)
{
	return device.SplitPath(dest, dir, filename);
}


void BlueSCSISend::HandleSendError(const char * err, status_t status)
{
	if (errHandler != NULL)
		errHandler->HandleSendError(err, status);
}


status_t BlueSCSISend::RaiseError(const char * err, status_t status)
{
	if (status != B_NO_ERROR)
		HandleSendError(err, status);
		
	return status;
}


bool BlueSCSISend::Send()
{
	char cwd[BLUE_SCSI_MAX_WORKING_DIR_LEN];
	
	if (!src->Exists()) {
		HandleSendError("Source entry does not exist");
		return false;
	}
	
	if (dir[0] != '\0') {
		if (!comm.GetWorkingDir(cwd, sizeof(cwd))) {
			HandleSendError("Unable to get current working directory");
			return false;
		}
		if (!comm.SetWorkingDir(dir, sizeof(dir))) {
			HandleSendError("Unable to change current working directory");
			return false;
		}
	}
	
	bool result = SendToRightDir();
	
	if (dir[0] != '\0') {
		if (!comm.SetWorkingDir(cwd, sizeof(cwd))) {
			HandleSendError("Unable to restore current working directory");
			return false;
		}
	}
	
	return result;
}


bool BlueSCSISend::SetFilenameFromEntry(BEntry * entry, bool useExistingFilename)
{
	if ((useExistingFilename) &&
		(filename[0] != '\0'))
		return true;
		
	if (RaiseError("Unable to get filename of source entry",
		src->GetName(beFilename)) != B_NO_ERROR)
		return false;
		
	if (strlen(beFilename) > BLUE_SCSI_MAX_FILE_NAME_LEN) {
		HandleSendError("Source filename is too long for the BlueSCSI");
		return false;
	}
	
	strcpy(filename, beFilename);
	return true;
}


bool BlueSCSISend::SendToRightDir()
{
	if (src->IsFile())
		return SendFile(src, true);
		
	if (src->IsDirectory())
		return SendDir(src, true);
		
	if (src->IsSymLink()) {
		HandleSendError("Cannot send symbolic links to the BlueSCSI");
		return false;
	}
	
	HandleSendError("Cannot send unknown entry type to the BlueSCSI");
	return false;
}


bool BlueSCSISend::SendFile(BEntry * entry, bool useExistingFilename)
{
	if (!SetFilenameFromEntry(src, useExistingFilename))
		return false;
	
	BFile file(entry, B_READ_ONLY);
	if (RaiseError("Unable to open source file for reading",
		file.InitCheck()) != B_NO_ERROR)
		return false;
	
	if (!comm.PrepareToSendFile(filename)) {
		HandleSendError("Unable to open target file for writing");
		return false;
	}
	
	bool result = true;
	uint32 blockOffset = 0;
	while (true) {
		ssize_t readSize = file.Read(buffer, bufferSize);
		if (readSize == 0)
			break;
			
		if (readSize < 0) {
			result = false;
			RaiseError("Error reading from source file", readSize);
			break;
		}
		
		if (!supportsBulk) {
			if (!comm.SendFileBytes(blockOffset, buffer, readSize)) {
				result = false;
				HandleSendError("Error writing to target file");
				break;
			}
		} else {
			uint32 blocksToWrite = (readSize / BLUE_SCSI_SEND_FILE_BLOCK_SIZE);
			uint32 bytesToWrite = (readSize % BLUE_SCSI_SEND_FILE_BLOCK_SIZE);
			size_t blockBufferSize = blocksToWrite * BLUE_SCSI_SEND_FILE_BLOCK_SIZE;
			
			if (blocksToWrite > 0) {
				if (!comm.SendFileBulk(blockOffset, buffer, blockBufferSize)) {
					result = false;
					HandleSendError("Error writing bulk data to target file");
					break;
				}
			}
			
			if (bytesToWrite > 0) {
				if (!comm.SendFileBytes(blockOffset, &(buffer[blockBufferSize]), bytesToWrite)) {
					result = false;
					HandleSendError("Error writing to target file");
					break;
				}
				
			}
		}
		
		blockOffset += (readSize / BLUE_SCSI_SEND_FILE_BLOCK_SIZE);
	}
	
	if (!comm.SendFileEnd()) {
		HandleSendError("Unable to close the target file on the BlueSCSI");
		return false;
	}
	return result;
}


bool BlueSCSISend::SendDir(BEntry * entry, bool useExistingFilename)
{
	if (!SetFilenameFromEntry(src, useExistingFilename))
		return false;
	
	HandleSendError("TODO - Write this code...");
	return false;
}


