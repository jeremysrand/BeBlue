#include "cli/GlobalOpts.h"
#include "cli/Get.h"


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


int Get::Execute()
{
	printf("src:     %s\n", src);
	printf("dest:    %s\n", dest != NULL ? dest : "<NONE>");
	printf("recurse: %s\n", globalOpts->ShouldRecurse() ? "ON" : "OFF");
	printf("force:   %s\n", globalOpts->ShouldForce() ? "ON" : "OFF");
	
	// TODO - Write this...
	
	return 0;
}
