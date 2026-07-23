#include "cli/GlobalOpts.h"
#include "cli/WifiInfo.h"


// Implementation

WifiInfo::WifiInfo() : Command()
{
}


const char * WifiInfo::Command()
{
	return "wifi-info";
}


const char * WifiInfo::Usage()
{
	return Command();
}


bool WifiInfo::RequiresOneDevice()
{
	return true;
}


bool WifiInfo::ParseArgs(int argc, const char * argv[])
{
	if (argc != 1) {
		fprintf(stderr, "ERROR: The %s command takes no arguments\n", Command());
		return false;
	}
	return true;
}


int WifiInfo::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	BlueSCSICommand & comm = device.Command();
	
	BlueSCSINetworkEntry entry;
	if (!comm.WifiInfo(&entry)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	
	printf("Wifi info output from %s:\n", device.PathString());
	printf("  Channel: %u\n", entry.channel);
	printf("  SSID:    \"%s\"\n", entry.ssid);
	printf("  BSSID:   %02x:%02x:%02x:%02x:%02x:%02x\n",
		entry.bssid[0],
		entry.bssid[1],
		entry.bssid[2],
		entry.bssid[3],
		entry.bssid[4],
		entry.bssid[5]);
	printf("  RSSI:    %u\n", entry.rssi);
	printf("  Auth:    %s\n", ((entry.flags & BLUE_SCSI_NETWORK_FLAG_AUTH) != 0 ? "Yes" : "No"));
	
	return 0;
}

