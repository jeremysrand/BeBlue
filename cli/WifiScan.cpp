#include <string.h>
#include <unistd.h>

#include "cli/GlobalOpts.h"
#include "cli/WifiScan.h"


// Implementation

WifiScan::WifiScan() : Command()
{
}


const char * WifiScan::Command()
{
	return "wifi-scan";
}


const char * WifiScan::Usage()
{
	return Command();
}


bool WifiScan::RequiresOneDevice()
{
	return true;
}


bool WifiScan::ParseArgs(int argc, const char * argv[])
{
	if (argc != 1) {
		fprintf(stderr, "ERROR: The %s command takes no arguments\n", Command());
		return false;
	}
	return true;
}


int WifiScan::Execute()
{
	BlueSCSIDevice & device = globalOpts->Device();
	BlueSCSICommand & comm = device.Command();
	
	bool started = false;
	if (!comm.StartWifiScan(&started)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	if (!started) {
		fprintf(stderr, "ERROR: Unable to start wifi scan on %s\n", device.PathString());
		return -1;
	}
	
	printf("Wifi scan started.");
	bool completed = false;
	while (!completed) {
		sleep(1);
		printf(".");
		if (!comm.CheckWifiScanComplete(&completed)) {
			if (comm.HasError())
				fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
			return -1;
		}
	}
	printf("\n");
	
	BlueSCSINetworkEntries result;
	memset(&result, 0, sizeof(result));
	
	if (!comm.WifiScanResults(&result)) {
		if (comm.HasError())
			fprintf(stderr, "ERROR: %s\n", comm.GetErrorStr());
		return -1;
	}
	
	printf("\n");
	printf("+-----------------------------------------------------------------+\n");
	printf("| %-5s| %-5s| %-5s| %-13s| %-65s|\n", "Chan", "RSSI", "Auth", "BSSID", "SSID");
	printf("|-----------------------------------------------------------------|\n");
	
	for (int i = 0; i < BLUE_SCSI_NUM_NETWORK_ENTRIES; i++) {
		BlueSCSINetworkEntry * entry = &(result.entries[i]);
		if (strlen(entry->ssid) == 0)
			continue;
		
		printf("| %-5u| %-5u| %-5s| %02x:%02x:%02x:%02x:%02x:%02x | %-65s|\n",
			entry->channel,
			entry->rssi,
			((entry->flags & BLUE_SCSI_NETWORK_FLAG_AUTH) != 0 ? "Yes" : "No"),
			entry->bssid[0],
			entry->bssid[1],
			entry->bssid[2],
			entry->bssid[3],
			entry->bssid[4],
			entry->bssid[5],
			entry->ssid);
	}
	printf("+-----------------------------------------------------------------+\n");
	
	
	return 0;
}

