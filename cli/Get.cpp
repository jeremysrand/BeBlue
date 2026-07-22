#include <Entry.h>

#include "cli/GlobalOpts.h"
#include "cli/Get.h"

#include "common/BlueSCSIGet.h"


// Implementation

Get::Get() : Command(),
	src(NULL),
	dest(NULL)
{
}


const char * Get::Command()
{
	return "get";
}


const char * Get::Usage()
{
	return "get source [dest]";
}


bool Get::RequiresOneDevice()
{
	return true;
}


bool Get::ParseArgs(int argc, const char * argv[])
{
	if (argc <= 1) {
		fprintf(stderr, "ERROR: Must provide a source to get\n");
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


void Get::HandleGetError(const char * err, status_t status)
{
	fprintf(stderr, "ERROR: %s\n", err);
	if (status != B_NO_ERROR)
		fprintf(stderr, "    %s\n", strerror(status));
}


int Get::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	
	BlueSCSIGet * get = new BlueSCSIGet(device, this);
	if (!get->SetSrc(src)) {
		delete get;
		fprintf(stderr, "ERROR: Unable to parse src %s into dir and filename\n",
			src);
		return -1;
	}
	
	BEntry entry;
	if (dest != NULL) {
		status_t status = entry.SetTo(dest);
		if (status != B_NO_ERROR) {
			delete get;
			HandleGetError("Unable to get entry for destination", status);
			return -1;
		}
		get->SetDest(&entry);
	}
	
	if (!get->SetRecurse(globalOpts->ShouldRecurse())) {
		delete get;
		fprintf(stderr, "ERROR: Unable to set recurse mode\n");
		return -1;
	}
	get->SetForce(globalOpts->ShouldForce());
	
	int result = get->Get() ? 0 : -1;
	
	if (result != 0)
		fprintf(stderr, "ERROR: Get operation failed\n");
	
	delete get;
	return result;
}
