/*
	Copyright 1999, Be Incorporated.   All Rights Reserved.
	This file may be used under the terms of the Be Sample Code License.
*/
/*
 * Common methods for ethernet drivers
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ether_driver.h"
#include <fcntl.h>
#include "NetBufList.h"
#include <add-ons/net_server/NetDevice.h>


class EtherDevice : public BNetDevice {
	int fd;
	int nonblock;
	_NetBufList packetlist;
	void fill_packetlist(void);
	BNetPacket *read_packet(void);
	int got_address;
	char address[6];
	unsigned framesize;
public: 
	EtherDevice(void) {
		fd = -1;
		nonblock = 0;
		got_address = 0;
		framesize = 0;
	}
	BIpDevice *OpenIP(void) {
		return (NULL);
	}
	BNetPacket *AllocPacket(void) {
		return (new BStandardPacket());
	}
	BNetPacket *ReceivePacket(void);
	void SendPacket(BNetPacket *packet) {
		write(fd, packet->Data(), packet->Size());
		delete packet;
	}
    void Address(char *address) {
		if (!got_address) {
			ioctl(fd, ETHER_GETADDR, this->address);
			got_address = 1;
		}
		memcpy(address, this->address, sizeof(this->address));
	}
	status_t AddMulticastAddress(const char *address) {
		return (ioctl(fd, ETHER_ADDMULTI, address));
	}
	status_t RemoveMulticastAddress(const char *address) {
		// I don't see a way to remove a multicast address on the DaynaPort
		// in general.  Perhaps it needs to disable, reenable and then add
		// the remaining addresses again.  Just skip it for now.
		//return (ioctl(fd, ETHER_REMMULTI, address));
		return B_OK;
	}
	status_t SetPromiscuous(bool yes) {
		// Not supported on the Dayna port and given the bandwidth over the
		// SCSI bus and the need to poll for reads, you really don't want to
		// do this.
#if 0
		int promisc = yes;
		return (ioctl(fd, ETHER_SETPROMISC, &promisc));
#endif
		return B_ERROR;
	}
    unsigned MaxPacketSize(void) {
#if 0
		if (framesize == 0) {
			if (ioctl(fd, ETHER_GETFRAMESIZE, &framesize) < 0) {
				framesize = 1514;
			}
		}
		return (framesize);
#endif
		return 1514; // Hardcode for the DaynaPort.
	}
	net_device_type Type(void) {
		return (B_ETHER_NET_DEVICE);
	}
	void Statistics(FILE *f) {
	}
    int SetMaxPacketSize(unsigned framesize) {
		if (framesize != 1514) {
			return (B_ERROR);
		}
		return (B_NO_ERROR);
	}
    void Close(void) {
		if (fd >= 0) {
			close(fd);
			fd = -1;
		}
	}
	int Init(const char *devicelink, char *params);
};
