#include "cli/GlobalOpts.h"
#include "cli/Send.h"

#include "common/BlueSCSISend.h"


// Implementation

Send::Send() : Command(),
	src(NULL),
	dest(NULL)
{
}


const char * Send::Command()
{
	return "send";
}


const char * Send::Usage()
{
	return "send source [dest]";
}


bool Send::RequiresOneDevice()
{
	return true;
}


bool Send::ParseArgs(int argc, const char * argv[])
{
	if (argc <= 1) {
		fprintf(stderr, "ERROR: Must provide a source to send\n");
		return false;
	}
	
	src = argv[1];
	if (argc == 2) {
		dest = NULL;
		return true;
	}
	
	if (argc > 3) {
		fprintf(stderr, "ERROR: Too many arguments passed to %s command\n", Command());
		return false;
	}
	
	dest = argv[2];
	return true;
}


void Send::HandleSendError(const char * err, status_t status)
{
	fprintf(stderr, "ERROR: %s\n", err);
	if (status != B_NO_ERROR)
		fprintf(stderr, "    %s\n", strerror(status));
}


int Send::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	
	BEntry entry(src);
	status_t status = entry.InitCheck();
	if (status != B_NO_ERROR) {
		HandleSendError("Unable to get entry for source", status);
		return -1;
	}
	
	BlueSCSISend * send = new BlueSCSISend(device, this);
	
	send->SetSrc(&entry);
	if (!send->SetRecurse(globalOpts->ShouldRecurse())) {
		delete send;
		fprintf(stderr, "ERROR: Unable to set recurse mode\n");
		return -1;
	}
	send->SetForce(globalOpts->ShouldForce());
	
	if (dest != NULL) {
		if (!send->SetDest(dest)) {
			delete send;
			fprintf(stderr, "ERROR: Unable to parse dest %s into dir and filename\n",
				dest);
			return -1;
		}
	}
	
	int result = send->Send() ? 0 : -1;
	
	if (result != 0)
		fprintf(stderr, "ERROR: Send operation failed\n");
	
	delete send;
	return result;
}
