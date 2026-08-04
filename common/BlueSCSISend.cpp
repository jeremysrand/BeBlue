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
	memset(cwd, 0, sizeof(cwd));
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


void BlueSCSISend::SetForce(bool arg)
{
	force = arg;
}


bool BlueSCSISend::SetDest(const char * dest)
{
	return device.ParsePath(dest, cwd, dir, filename);
}


void BlueSCSISend::HandleSendError(const char * err, status_t status)
{
	if (errHandler != NULL)
		errHandler->HandleSendError(err, status);
	
	if (status == B_NO_ERROR)
		device.Log("ERROR: %s", err);
	else
		device.Log("ERROR: %s, %s (%d)", err, strerror(status), status);
}


status_t BlueSCSISend::RaiseError(const char * err, status_t status)
{
	if (status != B_NO_ERROR)
		HandleSendError(err, status);
		
	return status;
}

bool BlueSCSISend::BuildDest()
{
	if (RaiseError("Unable to get filename of source entry",
		src->GetName(beFilename)) != B_NO_ERROR)
		return false;
	
	bool destCanBeDir = device.SupportsSetWorkingDir();
	if (filename[0] == '\0') {
		destCanBeDir = false;	
		if (strlen(beFilename) > BLUE_SCSI_MAX_FILE_NAME_LEN) {
			HandleSendError("Source filename is too long for the BlueSCSI");
			return false;
		}
		
		device.Log("Destination is not set so send \"%s\" to current working directory", beFilename);
		if (!device.ParsePath(beFilename, cwd, dir, filename))
			return false;
	}
	
	
	bool keepGoing = true;
	while (keepGoing) {
		keepGoing = false;
		
		if (dir[0] != '\0') {
			device.Log("Change cwd to destination dir \"%s\"", dir);
			if (!comm.SetWorkingDir(dir, sizeof(dir))) {
				HandleSendError("Unable to change current working directory");
				return false;
			}
		}
		
		uint8 numFiles = 0;
		if (!comm.CountFiles(&numFiles)) {
			HandleSendError("Unable to count files in the destination directory");
			return false;
		}
		device.Log("There are %u files in the destination dir", (uint32)numFiles);
		
		
		// If there are no files in the destination directory, then the file
		// cannot already exist.  We can just proceed with the send.
		if (numFiles == 0) {
			device.Log("Proceed with send because there are no files in the destination dir");
			break;
		}
		
		device.Log("List files in destination dir to check if it exists already");
		
		// Need to list all of the files in the destination directory to see
		// if the target exists already.
		BlueSCSIFileEntry * fileEntries = new(BlueSCSIFileEntry[numFiles]);
		if (!comm.ListFiles(fileEntries, numFiles)) {
			delete[] fileEntries;
			HandleSendError("Unable to list files in the destination directory");
			return false;
		}
		
		BlueSCSIFileEntry * destFileEntry = NULL;
		for (int i = 0; i < numFiles; i++) {
			if (strcmp(fileEntries[i].name, filename) == 0) {
				device.Log("Found \"%s\" already exists in destination directory", filename);
				destFileEntry = &(fileEntries[i]);
				break;
			}
		}
		
		// If the destination does not exist, then we can proceed with the send
		// and create the new file/directory in the destination directory.
		if (destFileEntry == NULL) {
			device.Log("File \"%s\" does not exist in destination dir, continue send", filename);
			delete[] fileEntries;
			break;
		}
		
		bool isFile = (destFileEntry->type == BLUE_SCSI_FILE_TYPE);
		delete[] fileEntries;
		
		if (isFile) {
			if (!src->IsFile()) {
				HandleSendError("The source is not a file but the destination is a file which already exists");
				return false;
			}
			
			device.Log("Source is a file and destination is an existing file");
			
			if (!force) {
				HandleSendError("The destination file already exists but force is not enabled");
				return false;
			}
			
			// The source is a file and the destination is a file which exists.
			// But force is enabled so proceed with the send.
			device.Log("Will overwrite \"%s\" file at destination because force is on", filename); 
			break;
		}
		
		
		// If the destination at this point cannot be treated like a directory
		// into which the source is copied, then we better be copying a directory
		if (!destCanBeDir) {
			if (!src->IsDirectory()) {
				HandleSendError("The source is not a directory but the destination is a directory which already exissts");
				return false;
			}
			
			device.Log("Source is a directory and destination is an existing directory");
			
			if (!force) {
				HandleSendError("The destination directory already exists but force is not enabled");
				return false;
			}
			
			// The source is a directory and the destination is a directory which
			// exists.  But force is enabled so proceed with the send.
			device.Log("Will overwrite \"%s\" directory at destination because force is on", filename); 
			break;
		}
		
		device.Log("The destination \"%s\" is an existing directory so will send source into that dir", filename);
		
		// This case happens when the user provides a specific destination and
		// that destination is an existing directory.  In that case, we will
		// send the source file/directory into the destination directory and
		// inherit the source filename for the destination.
		size_t dirLen = strlen(dir);
		if (dirLen + strlen(filename) + 1 >= BLUE_SCSI_MAX_WORKING_DIR_LEN) {
			HandleSendError("The directory portion of the path is too long");
			return false;
		}
		
		char * ptr = &(dir[dirLen - 1]);
		if (*ptr != '/')
			ptr++;
		*ptr = '/';
		ptr++;
		strcpy(ptr, filename);
			
		if (strlen(beFilename) > BLUE_SCSI_MAX_FILE_NAME_LEN) {
			HandleSendError("Source filename is too long for the BlueSCSI");
			return false;
		}
		strcpy(filename, beFilename);
		
		keepGoing = true;
		destCanBeDir = false;
	}
	
	return true;
}


bool BlueSCSISend::Send()
{
	if (device.IsLogging()) {
		BPath path;
		
		device.Log("Starting to send file(s) to the BlueSCSI");
		if (src->GetPath(&path) == B_NO_ERROR)
			device.Log("  src           = \"%s\"", path.Path());
		else
			device.Log("  src           = <ERROR>");
		device.Log("  cwd           = \"%s\"", cwd);
		device.Log("  dest dir      = \"%s\"", dir);
		device.Log("  dest filename = \"%s\"", filename);
		device.Log("  recurse       = %s", recurse ? "ON" : "OFF");
		device.Log("  force         = %s", force ? "ON" : "OFF");
		device.Log("  supportsBulk  = %s", supportsBulk ? "YES" : "NO");
	}
	
	if (!src->Exists()) {
		HandleSendError("Source entry does not exist");
		return false;
	}
	
	if (!BuildDest())
		return false;
	
	bool result = SendToRightDir();
	
	if (device.SupportsSetWorkingDir()) {
		device.Log("Restoring working dir to \"%s\"", cwd);
		if (!comm.SetWorkingDir(cwd, sizeof(cwd))) {
			HandleSendError("Unable to restore current working directory");
			return false;
		}
	}
	
	return result;
}


bool BlueSCSISend::SetFilenameFromEntry(BEntry * entry, bool useExistingFilename)
{
	if (RaiseError("Unable to get filename of source entry",
		entry->GetName(beFilename)) != B_NO_ERROR)
		return false;
	
	if ((useExistingFilename) &&
		(filename[0] != '\0'))
		return true;
		
	if (strlen(beFilename) > BLUE_SCSI_MAX_FILE_NAME_LEN) {
		HandleSendError("Source filename is too long for the BlueSCSI");
		return false;
	}
	
	strcpy(filename, beFilename);
	device.Log("Set target filename to \"%s\"", filename);
	return true;
}


bool BlueSCSISend::SendToRightDir()
{
	if (src->IsFile())
		return SendFile(src, true);
		
	if (src->IsDirectory()) {
		if (!recurse) {
			HandleSendError("Source is a directory but recurse has not been set");
			return false;
		}
		return SendDir(src, true);
	}
		
	if (src->IsSymLink()) {
		HandleSendError("Cannot send symbolic links to the BlueSCSI");
		return false;
	}
	
	HandleSendError("Cannot send unknown entry type to the BlueSCSI");
	return false;
}


bool BlueSCSISend::SendFile(BEntry * entry, bool useExistingFilename)
{
	if (!SetFilenameFromEntry(entry, useExistingFilename))
		return false;
	
	device.Log("Send file \"%s\" to the BlueSCSI", beFilename);
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
		device.Log("Try to read %lu bytes from the file", bufferSize);
		ssize_t readSize = file.Read(buffer, bufferSize);
		if (readSize == 0)
			break;
			
		if (readSize < 0) {
			result = false;
			RaiseError("Error reading from source file", readSize);
			break;
		}
		
		device.Log("Read %ld bytes from the file", readSize);
		if (!supportsBulk) {
			device.Log("Write %ld non-bulk bytes to the file", readSize);
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
				device.Log("Write %lu bulk bytes to the file", blockBufferSize);
				if (!comm.SendFileBulk(blockOffset, buffer, blockBufferSize)) {
					result = false;
					HandleSendError("Error writing bulk data to target file");
					break;
				}
			}
			
			if (bytesToWrite > 0) {
				device.Log("Write %u non-bulk bytes to the file", bytesToWrite);
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
	
	device.Log("File send complete, %s", (result ? "SUCCESS" : "FAILED"));
	return result;
}


bool BlueSCSISend::SendDir(BEntry * dirEntry, bool useExistingFilename)
{
	if (!SetFilenameFromEntry(dirEntry, useExistingFilename))
		return false;
	
	device.Log("Send directory \"%s\" to the BlueSCSI", beFilename);
	device.Log("Must add the dest filename to the dest directory");
	device.Log("  dest dir is currently \"%s\"", dir);
	uint32 oldDirLen = strlen(dir); 	
	if (oldDirLen + strlen(filename) + 1 >= BLUE_SCSI_MAX_WORKING_DIR_LEN) {
		HandleSendError("Target path length is too long for the BlueSCSI");
		return false;
	}
	
	dir[oldDirLen] = '/';
	strcpy(&(dir[oldDirLen + 1]), filename);
	device.Log("  dest dir is now \"%s\"", dir);
	
	if (!comm.SetWorkingDir(dir, sizeof(dir))) {
		HandleSendError("Unable to create target directory");
		return false;
	}
	
	device.Log("Open source directory to iterate over entries");
	BDirectory srcDir(dirEntry);
	if (RaiseError("Unable to open source directory",
		srcDir.InitCheck()) != B_NO_ERROR)
		return false;
	
	BEntry entry;
	status_t status;
	device.Log("Copy files in the source directory to the BlueSCSI");
	while ((status = srcDir.GetNextEntry(&entry)) == B_NO_ERROR)
		if ((entry.IsFile()) &&
			(!SendFile(&entry)))
			return false;
	
	if (status != B_ENTRY_NOT_FOUND) {
		RaiseError("Unable to walk source directory", status);
		return false;
	}
	
	srcDir.Rewind();
	device.Log("Copy sirectories in the source directory to the BlueSCSI");
	while ((status = srcDir.GetNextEntry(&entry)) == B_NO_ERROR)
		if ((entry.IsDirectory()) &&
			(!SendDir(&entry)))
			return false;
	
	if (status != B_ENTRY_NOT_FOUND) {
		RaiseError("Unable to walk source directory", status);
		return false;
	}
	
	dir[oldDirLen] = '\0';
	device.Log("Reset source dir back to \"%s\"", dir);
	
	device.Log("Directory send complete, SUCCESS");
	return true;
}


