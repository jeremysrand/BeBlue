#include <stdlib.h>

#include "cli/GlobalOpts.h"
#include "cli/WifiJoin.h"


// Implementation

WifiJoin::WifiJoin() : Command(),
	channel(0),
	ssid(NULL),
	key(NULL)
{
}


const char * WifiJoin::Command()
{
	return "wifi-join";
}


const char * WifiJoin::Usage()
{
	return "wifi-join channel ssid key";
}


bool WifiJoin::RequiresOneDevice()
{
	return true;
}


bool WifiJoin::ParseArgs(int argc, const char * argv[])
{
	if (argc != 4) {
		fprintf(stderr, "ERROR: The %s command takes three arguments\n", Command());
		return false;
	}
	
	int chan = atoi(argv[1]);
	if ((chan < 1) ||
	    (chan > 255)) {
		fprintf(stderr, "ERROR: Invalid channel number: %s\n", argv[1]);
		return false;
	}
	
	channel = (uint8)chan;
	ssid = argv[2];
	if (strlen(ssid) >= BLUE_SCSI_SSID_LEN) {
		fprintf(stderr, "ERROR: SSID must be less than %u characters long\n",
			BLUE_SCSI_SSID_LEN);
		return false;
	}
	
	key = argv[3];
	if (strlen(key) >= BLUE_SCSI_KEY_LEN) {
		fprintf(stderr, "ERROR: The key must be less than %u characters long\n",
			BLUE_SCSI_KEY_LEN);
		return false;
	}
	return true;
}


int WifiJoin::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	BlueSCSICommand & comm = device.Command();
	
	printf("Joining %s on %s:\n", ssid, device.PathString());
	
	BlueSCSINetworkJoinRequest request;
	strcpy(request.ssid, ssid);
	strcpy(request.key, key);
	request.channel = channel;
	
	if (!comm.WifiJoin(&request)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	printf("   Joined!\n");
	
	return 0;
}

