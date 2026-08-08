#include <stdio.h>

#include "cli/GlobalOpts.h"
#include "cli/Version.h"


#define VERSION "BeBlueCli v0.9.3"


// Implementation

Version::Version() : Command()
{
}


const char * Version::Command()
{
	return "version";
}


const char * Version::Usage()
{
	return Command();
}


bool Version::RequiresOneDevice()
{
	return false;
}


bool Version::ParseArgs(int argc, const char * argv[])
{
	if (argc != 1) {
		fprintf(stderr, "ERROR: The %s command takes no arguments\n", Command());
		return false;
	}
	return true;
}


int Version::Execute()
{
	printf("%s\n", VERSION);
	return 0;	
}
