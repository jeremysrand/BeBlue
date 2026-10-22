#include <KernelExport.h>
#include <Drivers.h>

#ifndef SCSI_COMMAND_H
#define SCSI_COMMAND_H


// Defines

#define SCSI_DEVICE_TYPE_CPU 3


// Types

typedef struct tDeviceId {
	int bus;
	int target;
	int lun;
} tDeviceId;


// API

extern int decode_scsi_ptl(const char *name, tDeviceId * id);

extern status_t do_raw_command(cam_for_driver_module_info *cam,
			   raw_device_command *rdc, tDeviceId * id);
			   
extern status_t execute_command(cam_for_driver_module_info *cam, tDeviceId * id,
	uint8 * command, uint8 commandLen, void * data, size_t dataLen);

extern int is_dayna_port(cam_for_driver_module_info *cam, tDeviceId * id);

extern status_t get_mac_addr(cam_for_driver_module_info *cam, tDeviceId * id,
	uint8 * buf);
	
extern status_t do_multi(cam_for_driver_module_info *cam, tDeviceId * id,
	char * addr);

#endif