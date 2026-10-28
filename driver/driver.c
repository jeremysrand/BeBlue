/* ++++++++++
	driver.c
	A skeletal device driver
+++++ */

#include "../VersionStr.h"

#include <KernelExport.h>
#include <Drivers.h>
#include <Errors.h>

#include "devices.h"
#include "scsi_common.h"


// Defines

#define MAX_TARGET 16
#define MAX_LUN 8


// Types

enum {
	ETHER_GETADDR = B_DEVICE_OP_CODES_END,	/* get ethernet address */
	ETHER_INIT,								/* set irq and port */
	ETHER_NONBLOCK,							/* set/unset nonblocking mode */
	ETHER_ADDMULTI,							/* add multicast addr */
	ETHER_REMMULTI,							/* rem multicast addr */
	ETHER_SETPROMISC,						/* set promiscuous */
	ETHER_GETFRAMESIZE						/* get frame size */
};

typedef struct tDevice
{
	tDeviceId id;
	int nonblocking;
}  tDevice;


// Gobals

static cam_for_driver_module_info	*cam;
static const char cam_name[] = B_CAM_FOR_DRIVER_MODULE_NAME;


// Implementation

static int daynaHello(int argc, char *argv[])
{
	kprintf("Hello, world!\n");
	return 0;
}

/* ----------
	init_hardware - called once the first time the driver is loaded
----- */
status_t
init_hardware (void)
{
	dprintf("Dayna: In init_hardware() for my driver, " VERSION_DETAIL "\n");
	return B_OK;
}


/* ----------
	init_driver - optional function - called every time the driver
	is loaded.
----- */
status_t
init_driver (void)
{
	dprintf("Dayna: In init_driver() for my driver, " VERSION_DETAIL "\n");
	int rc = add_debugger_command("daynaHello", daynaHello, "Say hello world");
	dprintf("Dayna: add_debugger_command() = %d\n", rc);
	
	if(get_module(cam_name, (module_info **)&cam) != B_OK)
		return B_ERROR;

	return B_NO_ERROR;
}


/* ----------
	uninit_driver - optional function - called every time the driver
	is unloaded
----- */
void
uninit_driver (void)
{
	dprintf("Dayna: In uninit_driver() for my driver\n");
	int rc = remove_debugger_command("daynaHello", daynaHello);
	dprintf("Dayna: remove_debugger_command() = %d\n", rc);
	
	free_devices();
	put_module(cam_name);
}

	
/* ----------
	my_device_open - handle open() calls
----- */

static status_t
my_device_open (const char *name, uint32 flags, void** c)
{
	dprintf("Dayna: In my_device_open(%s, %u, %p) for my driver\n", name, flags,
		c);
		
	tDevice **cookie = (tDevice **) c;
		
	tDevice * device = (tDevice *)malloc(sizeof(tDevice));
	if (device == NULL)
		return B_ERROR;
	
	if (decode_scsi_ptl(name, &(device->id)) != 0) {
		dprintf("Dayna: Failed to parse the device name");
		free(device);
		return B_ERROR;
	}
	
	device->nonblocking = 0;
	
	if (enable_datapath(cam, &(device->id), true) != B_OK) {
		dprintf("Dayna: Failed enable the datapath");
		free(device);
		return B_ERROR;
	}
	
	if (set_mode(cam, &(device->id)) != B_OK) {
		dprintf("Dayna: Failed set mode on interface");
		free(device);
		return B_ERROR;
	}
	
	*cookie = device;
	return 0;
}


/* ----------
	my_device_read - handle read() calls
----- */

static status_t
my_device_read (void* cookie, off_t position, void *buf, size_t* num_bytes)
{
	dprintf("Dayna: In my_device_read(%p, %Ld, %p, %p) for my driver\n", cookie,
		position, buf, num_bytes);
	*num_bytes = 0;				/* tell caller nothing was read */
	return B_OK;
}


/* ----------
	my_device_write - handle write() calls
----- */

static status_t
my_device_write (void* cookie, off_t position, const void* buffer, size_t* num_bytes)
{
	dprintf("Dayna: In my_device_write(%p, %Ld, %p, %p) for my driver\n", cookie,
		position, buffer, num_bytes);
	
	size_t buf_size = *num_bytes;
	*num_bytes = 0;				/* tell caller nothing was written by default */
	
	tDevice * device = (tDevice *) cookie;
	if (device == NULL)
		return B_ERROR;
		
	status_t result = tx_packet(cam, &(device->id), buffer, buf_size);
	if (result == B_OK)
		*num_bytes = buf_size;
	
	return result;
}


/* ----------
	my_device_control - handle ioctl calls
----- */

static status_t
my_device_control (void* cookie, uint32 op, void* arg, size_t len)
{
	dprintf("Dayna: In my_device_control(%p, %u, %p, %u) for my driver\n", cookie,
		op, arg, len);
	tDevice * device = (tDevice *) cookie;
	if (device == NULL)
		return B_ERROR;
	
	switch (op) {
	case B_RAW_DEVICE_COMMAND:
		 if (arg == NULL)
		 	return B_ERROR;
		 	
		dprintf("Dayna: Execute raw command\n");
		raw_device_command *rdc = (raw_device_command *) arg;
		return do_raw_command(cam, rdc, &(device->id));
		
	case ETHER_INIT:
		dprintf("Dayna: ETHER_INIT, nothing to do\n");
		return B_NO_ERROR;

	case ETHER_GETADDR:
		if (arg == NULL)
			return (B_ERROR);
		
		dprintf("Dayna: GET_ADDR\n");
		return get_mac_addr(cam, &(device->id), (uint8 *)arg);

	case ETHER_NONBLOCK:
		if (arg == NULL)
			return B_ERROR;
		
		memcpy(&device->nonblocking, arg, sizeof(device->nonblocking));
		dprintf("Dayna: NON_BLOCK %x\n", device->nonblocking);
		return B_NO_ERROR;

	case ETHER_ADDMULTI:
		if (arg == NULL)
			return B_ERROR;
		
		dprintf("Dayna: DO_MULTI\n");
		return do_multi(cam, &(device->id), (char *)arg);
	}
	return B_DEV_INVALID_IOCTL;

}


/* ----------
	my_device_close - handle close() calls
----- */

static status_t
my_device_close (void* cookie)
{
	dprintf("Dayna: In my_device_close(%p) for my driver\n", cookie);
	return B_OK;
}


/* -----
	my_device_free - called after the last device is closed, and after
	all i/o is complete.
----- */
static status_t
my_device_free (void* cookie)
{
	dprintf("Dayna: In my_device_free(%p) for my driver\n", cookie);
	tDevice * device = (tDevice *) cookie;
	enable_datapath(cam, &(device->id), false);
	free(device);
	return B_OK;
}

/* -----
	function pointers for the device hooks entry points
----- */

device_hooks my_device_hooks = {
	my_device_open, 			/* -> open entry point */
	my_device_close, 			/* -> close entry point */
	my_device_free,			/* -> free cookie */
	my_device_control, 		/* -> control entry point */
	my_device_read,			/* -> read entry point */
	my_device_write			/* -> write entry point */
};


static int
get_max_path (CCB_HEADER *ccbh)
{
	uchar max_path;

	ccbh->cam_func_code = XPT_PATH_INQ;
	ccbh->cam_path_id = 0xFF;	/* return only highest path id */
	(*cam->xpt_action) (ccbh);		/* do path inquiry */
	if (ccbh->cam_status != CAM_REQ_CMP)
		return -1;
	max_path = ((CCB_PATHINQ *)ccbh)->cam_hpath_id;
	return (max_path == 0xFF) ? -1 : max_path;
}

/* ----------
	publish_devices - return a null-terminated array of devices
	supported by this driver.
----- */

_EXPORT const char**
publish_devices()
{
	char		tmp[64];
	CCB_HEADER	*ccbh;				/* -> ccb */
	tDeviceId   id;
	int			max_path;			/* highest known path id */
	int         instance = 0;
	
	free_devices();
	cam->minfo.rescan();
	
	if (!(ccbh = (*cam->xpt_ccb_alloc)())){
		return NULL;
	}

	if ((max_path = get_max_path (ccbh)) < 0) { /* get highest path id */
		(*cam->xpt_ccb_free) (ccbh);
		return NULL;
	}


	ccbh->cam_func_code = XPT_GDEV_TYPE;
    ((CCB_GETDEV *)ccbh)->cam_inq_data = NULL;

	/* for each path id, look for disks */
	for (id.bus = 0; id.bus <= max_path ; id.bus++) {
		ccbh->cam_path_id = id.bus;
		for (id.target = 0; id.target < MAX_TARGET; id.target++) {
			ccbh->cam_target_id = id.target;
			for (id.lun = 0; id.lun < MAX_LUN; id.lun++) {
				ccbh->cam_target_lun = id.lun;
				(*cam->xpt_action)(ccbh);	/* do get_device_type */
				if (ccbh->cam_status != CAM_REQ_CMP)
					continue;
				if (((CCB_GETDEV *)ccbh)->cam_pd_type != SCSI_DEVICE_TYPE_CPU)
					continue;
					
				if (is_dayna_port(cam, &id))
					add_device(&id);
			}
		}
	}

	(*cam->xpt_ccb_free) (ccbh);

	return get_device_names();
}

/* ----------
	find_device - return ptr to device hooks structure for a
	given device name
----- */

_EXPORT device_hooks*
find_device(const char* name)
{
	dprintf("Dayna: In find_device(%s) for my driver\n", name);
	return &my_device_hooks;
}


_EXPORT int32 api_version = B_CUR_DRIVER_API_VERSION;