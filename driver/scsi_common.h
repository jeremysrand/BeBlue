#include <KernelExport.h>
#include <Drivers.h>

#ifndef SCSI_COMMAND_H
#define SCSI_COMMAND_H


// Defines

#define SCSI_DEVICE_TYPE_CPU 3

// 2x max ZuluSCSI multi-packet response
//   2x1530 = 3060, rounded up to 3072
// I got this from irixdayna on github, a driver for Dayna on IRIX.
#define RX_BUFFER_SIZE 3072


// Types

typedef struct tDeviceId {
	int bus;
	int target;
	int lun;
} tDeviceId;

typedef struct tRxBuffer {
	uint8 * ptr;
	uint8 data[RX_BUFFER_SIZE];
} tRxBuffer;


// API

extern status_t do_raw_command(cam_for_driver_module_info *cam,
			   raw_device_command *rdc, tDeviceId * id);
			   
extern status_t execute_command(cam_for_driver_module_info *cam, tDeviceId * id,
	uint8 * command, uint8 commandLen, void * data, size_t dataLen);

extern int is_dayna_port(cam_for_driver_module_info *cam, tDeviceId * id);

extern status_t enable_datapath(cam_for_driver_module_info *cam, tDeviceId * id,
	bool enable);
	
extern status_t set_mode(cam_for_driver_module_info *cam, tDeviceId * id);

extern status_t get_mac_addr(cam_for_driver_module_info *cam, tDeviceId * id,
	uint8 * buf);
	
extern status_t do_multi(cam_for_driver_module_info *cam, tDeviceId * id,
	char * addr);
	
extern status_t tx_packet(cam_for_driver_module_info *cam, tDeviceId * id,
	const void * packet, size_t packetLen);

// Returns:
//   - Size of packet read in bytes if > 0
//   - 0 if there is no packet to read
//   - -1 if an error occurs
extern int rx_packet(cam_for_driver_module_info *cam, tDeviceId * id,
	tRxBuffer * buffer, void * packet, size_t maxPacketSize);

#endif