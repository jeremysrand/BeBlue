#include "cli/GlobalOpts.h"


// Implementation

GlobalOpts::GlobalOpts()
	: device(NULL),
	  verbose(false),
	  recurse(false),
	  force(false)
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


bool GlobalOpts::ShouldRecurse()
{
	return recurse;
}


void GlobalOpts::SetRecurse(bool arg)
{
	recurse = arg;
}


bool GlobalOpts::ShouldForce()
{
	return force;
}


void GlobalOpts::SetForce(bool arg)
{
	force = arg;
}