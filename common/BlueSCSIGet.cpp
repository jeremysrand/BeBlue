#include <string.h>

#include <File.h>

#include "common/BlueSCSIDevice.h"
#include "common/BlueSCSIGet.h"


BlueSCSIGet::BlueSCSIGet(BlueSCSIDevice & deviceArg, BlueSCSIGetErrorHandler * errHandlerArg) :
	device(deviceArg),
	comm(deviceArg.Command()),
	recurse(false),
	force(false),
	dest(NULL),
	buffer(NULL),
	bufferSize(0),
	errHandler(errHandlerArg)
{
	memset(dir, 0, sizeof(dir));
	memset(filename, 0, sizeof(filename));
	
	if (device.SupportsLargeTransfers())
		bufferSize = BLUE_SCSI_FILE_MAX_BLOCKS_PER_TRANSFER * BLUE_SCSI_GET_FILE_BLOCK_SIZE;
	else
		bufferSize = BLUE_SCSI_GET_FILE_BLOCK_SIZE;
	buffer = new char[bufferSize];
}


BlueSCSIGet::~BlueSCSIGet()
{
	delete[] buffer;
}


bool BlueSCSIGet::SetSrc(const char * src)
{
	return device.SplitPath(src, dir, filename);
}


void BlueSCSIGet::SetRecurse(bool arg)
{
	recurse = arg;
}


void BlueSCSIGet::SetForce(bool arg)
{
	force = arg;
}


void BlueSCSIGet::SetDest(BEntry * arg)
{
	dest = arg;
}


const char * BlueSCSIGet::Filename()
{
	return filename;
}


void BlueSCSIGet::HandleGetError(const char * err, status_t status)
{
	if (errHandler != NULL)
		errHandler->HandleGetError(err, status);
}


status_t BlueSCSIGet::RaiseError(const char * err, status_t status)
{
	if (status != B_NO_ERROR)
		HandleGetError(err, status);
		
	return status;
}


bool BlueSCSIGet::Get()
{
	char cwd[BLUE_SCSI_MAX_WORKING_DIR_LEN];
	if (dir[0] != '\0') {
		if (!comm.GetWorkingDir(cwd, sizeof(cwd))) {
			HandleGetError("Unable to get current working directory");
			return false;
		}
		if (!comm.SetWorkingDir(dir, sizeof(dir))) {
			HandleGetError("Unable to change current working directory");
			return false;
		}
	}
	
	bool result = GetFromRightDir();
	
	if (dir[0] != '\0') {
		if (!comm.SetWorkingDir(cwd, sizeof(cwd))) {
			HandleGetError("Unable to restore current working directory");
			return false;
		}
	}
	
	return result;
}


bool BlueSCSIGet::GetFromRightDir()
{
	uint8 numFiles = 0;
	if (!comm.CountFiles(&numFiles)) {
		HandleGetError("Unable to count files in the source directory");
		return false;
	}
	
	if (numFiles == 0) {
		HandleGetError("No files exist in the source directory");
		return false;
	}
		
	BlueSCSIFileEntry * fileEntries = new(BlueSCSIFileEntry[numFiles]);
	if (!comm.ListFiles(fileEntries, numFiles)) {
		delete[] fileEntries;
		HandleGetError("Unable to list files in the source directory");
		return false;
	}
	
	BlueSCSIFileEntry * srcFileEntry = NULL;
	for (int i = 0; i < numFiles; i++) {
		if (strcmp(fileEntries[i].name, filename) == 0) {
			srcFileEntry = &(fileEntries[i]);
		}
	}
	
	if (srcFileEntry == NULL) {
		delete[] fileEntries;
		HandleGetError("File does not exist in the source directory");
		return false;
	}
	
	bool result = false;
	
	switch (srcFileEntry->type) {
		case BLUE_SCSI_FILE_TYPE:
			result = GetFile(srcFileEntry, dest);
			break;
			
		case BLUE_SCSI_DIR_TYPE:
			if (!recurse)
				HandleGetError("Source is a directory but recurse has not been set");
			else
				result = GetDir(srcFileEntry, dest);
			break;
		default:
			HandleGetError("Unexpected source file type");
	}
	
	delete[] fileEntries;
	return result;
}


bool BlueSCSIGet::GetFile(const BlueSCSIFileEntry * fileEntry, BEntry * entryArg)
{
	BEntry * entry = entryArg;
	BEntry localEntry;
	if (entry == NULL) {
		if (RaiseError("Unable to create entry for file in current working dir",
			localEntry.SetTo(fileEntry->name)) != B_NO_ERROR)
			return false;
		
		entry = &localEntry;
	} else if ((entry->Exists()) &&
		(entry->IsDirectory())) {
		BDirectory dir(entry);
		
		if (RaiseError("Unable to get destination directory",
			dir.InitCheck()) != B_NO_ERROR)
			return false;
		
		if (RaiseError("Unable to create entry for file in dest dir",
			localEntry.SetTo(&dir, fileEntry->name)) != B_NO_ERROR)
			return false;
		
		entry = &localEntry;
	}
	
	uint32 openMode = B_WRITE_ONLY | B_CREATE_FILE;
	if (force)
		openMode |= B_ERASE_FILE;
	else
		openMode |= B_FAIL_IF_EXISTS;
	
	BFile destFile(entry, openMode);
	if (RaiseError("Unable to open the destination file for writing",
			destFile.InitCheck()) != B_NO_ERROR)
		return false;
	
	uint64 bytesLeft = comm.GetFileSize(*fileEntry);
	uint32 blockOffset = 0;
	while (bytesLeft > 0) {
		if (!comm.GetFile(fileEntry->index, blockOffset, buffer, bufferSize)) {
			HandleGetError("Unable to read contents of file");
			return false;
		}
		uint64 bytesRead = bufferSize;
		if (bytesRead > bytesLeft)
			bytesRead = bytesLeft;
			
		if (destFile.Write(buffer, bytesRead) != bytesRead) {
			HandleGetError("Unable to write contents to file");
			return false;
		}
		
		bytesLeft -= bytesRead;
		blockOffset += (bytesRead / BLUE_SCSI_GET_FILE_BLOCK_SIZE);
	}
	
	return true;
}


bool BlueSCSIGet::GetDir(const BlueSCSIFileEntry * fileEntry, BEntry * entryArg)
{
	HandleGetError("TODO - Write this code!");
	return false;
}
