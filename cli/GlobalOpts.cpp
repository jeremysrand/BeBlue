#include "cli/GlobalOpts.h"


// Implementation

GlobalOpts::GlobalOpts()
	: device(NULL),
	  verbose(false)
{
}


bool GlobalOpts::HasDevice()
{
	return device != NULL;
}


BlueSCSIDevice & GlobalOpts::Device()
{
	return *device;
}


void GlobalOpts::SetDevice(BlueSCSIDevice * deviceArg)
{
	device = deviceArg;
}


bool GlobalOpts::IsVerbose()
{
	return verbose;
}


void GlobalOpts::SetVerbose(bool arg)
{
	verbose = arg;
}
