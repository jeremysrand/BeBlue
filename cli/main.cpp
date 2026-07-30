#include <getopt.h>
#include <stdio.h>

#include <Path.h>

#include "common/BlueSCSIDevice.h"
#include "common/BlueSCSIScan.h"

#include "cli/CommandRegistry.h"
#include "cli/Command.h"
#include "cli/GlobalOpts.h"


// Implementation

static void usage(const char * argv0, CommandRegistry & registry)
{
	printf("USAGE: %s [-h] [-d device] [-v] command...\n", argv0);
	printf("  Commands:\n");
	registry.PrintUsage();
}


int main(int argc, char *argv[]) 
{
	int c;
	const char * devicePath = NULL;
	
	GlobalOpts globalOpts;
	CommandRegistry registry;
	
	while ((c = getopt(argc, argv, "hvrfd:l:")) != EOF) {
		switch (c) {
			case 'h':
			case '?':
				usage(argv[0], registry);
				exit(-1);
				
			case 'v':
				globalOpts.SetVerbose(true);
				break;
				
			case 'r':
				globalOpts.SetRecurse(true);
				break;
				
			case 'f':
				globalOpts.SetForce(true);
				break;
				
			case 'd':
				devicePath = optarg;
				break;
				
			case 'l':
				globalOpts.AddFileLogger(optarg);
				break;
		}
	}
	
	if (optind >= argc) {
		usage(argv[0], registry);
		exit(-1);
	}
	
	Command * command = registry.GetCommand(argv[optind]);
	if (command == NULL) {
		usage(argv[0], registry);
		exit(-1);
	}
		
	BlueSCSIDevice * device = NULL;
	BlueSCSIScan * scan = NULL;
	
	command->SetGlobalOpts(globalOpts);
	if ((command->RequiresOneDevice()) ||
		(devicePath != NULL)) {
		if (devicePath != NULL) {
			BPath path(devicePath);
			device = new BlueSCSIDevice(&path, command);
			if (!device->IsBlueSCSI()) {
				delete device;
				fprintf(stderr, "ERROR: Device at %s is not a BlueSCSI.\n",
					devicePath);
				exit(-1);
			}
			globalOpts.SetDevice(device);
			command->VerbosePrintf("Using BlueSCSI device %s\n",
				globalOpts.Device().PathString());
		} else {
			scan = new BlueSCSIScan(command);
			if (scan->NumDevices() == 0) {
				delete scan;
				fprintf(stderr, "ERROR: No BlueSCSI devices found.\n");
				exit(-1);
			}
			globalOpts.SetDevice(scan->DeviceAt(0));
			command->VerbosePrintf("Found %d BlueSCSI devices, using %s\n",
					scan->NumDevices(), globalOpts.Device().PathString()); 
		}
	}
	
	int result = -1;	
	if (!command->ParseArgs(argc - optind, &(argv[optind])))
		usage(argv[0], registry);
	else
		result = command->Execute();
	
	if (scan != NULL)
		delete scan;
	if (device != NULL)
		delete device;
	
	exit(result);
}