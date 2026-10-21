#ifndef DEVICES_H
#define DEVICES_H


// API

void free_devices(void);

void add_device(int bus, int target, int lun);

const char ** get_device_names(void);


#endif