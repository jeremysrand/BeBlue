#ifndef DEVICES_H
#define DEVICES_H

// Forward declarations

typedef struct tDeviceId tDeviceId;


// API

void free_devices(void);

void add_device(tDeviceId * id);

const char ** get_device_names(void);

const tDeviceId * get_device_id(const char * name);


#endif