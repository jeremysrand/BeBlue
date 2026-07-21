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


void BlueSCSISend::SetRecurse(bool arg)
{
	recurse = arg;
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
	HandleSendError("TODO - Write this code...");
	return false;
}
