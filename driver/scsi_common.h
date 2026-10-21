#include <KernelExport.h>
#include <Drivers.h>

#ifndef SCSI_COMMAND_H
#define SCSI_COMMAND_H


// API

extern int decode_scsi_ptl(const char *name, int *bus, int *targ, int *lun);

extern status_t do_raw_command(cam_for_driver_module_info *cam,
			   raw_device_command *rdc, uint path, uint targ, uint lun);

#endif