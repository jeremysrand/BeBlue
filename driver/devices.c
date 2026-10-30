#include <stdlib.h>
#include <string.h>

#include <KernelExport.h>

#include "devices.h"
#include "scsi_common.h"


// Defines

#define DEVICE_NAME_PREFIX "net/dayna/"
#define DEVICE_NAME_MAX_LEN 32


// Types

typedef struct tDeviceListElem {
	char name[DEVICE_NAME_MAX_LEN];
	tDeviceId id;
	struct tDeviceListElem * next;
} tDeviceListElem;


// Globals

int num_devices = 0;
tDeviceListElem * device_list = NULL;
char ** device_names = NULL;


// Implementation

static void free_device(tDeviceListElem * device_list_elem)
{
	// Might need to do more cleanup here in the future as the device
	// structure fills out
	dprintf("Dayna: Freeing device %s\n", device_list_elem->name);
	free(device_list_elem);
}


void free_devices(void)
{
	tDeviceListElem * device_list_elem = device_list;
	while (device_list_elem != NULL) {
		tDeviceListElem * next = device_list_elem->next;
		free_device(device_list_elem);
		device_list_elem = next;
	}
	
	if (device_names != NULL) {
		free(device_names);
		device_names = NULL;
	}
	
	num_devices = 0;	
}


void add_device(tDeviceId * id)
{
	tDeviceListElem * device_list_elem = (tDeviceListElem *)malloc(sizeof(tDeviceListElem));
	if (device_list_elem == NULL)
		return;
		
	sprintf(device_list_elem->name, DEVICE_NAME_PREFIX "%d", num_devices);
	device_list_elem->id = *id;
	num_devices++;
	
	dprintf("Dayna: Added device %s\n", device_list_elem->name);
	
	device_list_elem->next = device_list;
	device_list = device_list_elem;
}


const char ** get_device_names(void)
{
	dprintf("Dayna: get_device_names()\n");
	if (device_names == NULL) {
		dprintf("Dayna: Allocate device_names, num_devices = %d\n", num_devices);
		device_names = (char **) malloc(sizeof(char *) * (num_devices + 1));
		if (device_names != NULL) {
			tDeviceListElem * device_list_elem = device_list;
			for (int devnum = 0; devnum < num_devices; devnum++) {
				device_names[devnum] = device_list_elem->name;
				dprintf("Dayna: device_names[%d] = %s\n", devnum, device_list_elem->name);
				device_list_elem = device_list_elem->next;
			}
			dprintf("Dayna: device_names[%d] = <NULL>\n", num_devices);
			device_names[num_devices] = NULL;			
		}
	}
	dprintf("Dayna: get_device_names() = %p\n", device_names);
	return device_names;
}


const tDeviceId * get_device_id(const char * name)
{
	tDeviceListElem * device_list_elem = device_list;
	while (device_list_elem != NULL) {
		if (strcmp(name, device_list_elem->name) == 0) {
			return &(device_list_elem->id);
		}
	}
	return NULL;	
}