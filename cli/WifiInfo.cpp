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
	
	BlueSCSIWifiInfo wifiInfo;
	if (!comm.WifiInfo(&wifiInfo)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	
	printf("Wifi info output from %s:\n", device.PathString());
	printf("  Channel: %u\n", wifiInfo.entry.channel);
	printf("  SSID:    \"%s\"\n", wifiInfo.entry.ssid);
	printf("  BSSID:   %02x:%02x:%02x:%02x:%02x:%02x\n",
		wifiInfo.entry.bssid[0],
		wifiInfo.entry.bssid[1],
		wifiInfo.entry.bssid[2],
		wifiInfo.entry.bssid[3],
		wifiInfo.entry.bssid[4],
		wifiInfo.entry.bssid[5]);
	printf("  RSSI:    %d\n", (int)wifiInfo.entry.rssi);
	printf("  Auth:    %s\n", ((wifiInfo.entry.flags & BLUE_SCSI_NETWORK_FLAG_AUTH) != 0 ? "Yes" : "No"));
	
	return 0;
}

