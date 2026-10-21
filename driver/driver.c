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

#define DEVICE_TYPE_CPU 3

static cam_for_driver_module_info	*cam;
static const char cam_name[] = B_CAM_FOR_DRIVER_MODULE_NAME;


typedef struct tDevice
{
	int bus;
	int target;
	int lun;
}  tDevice;


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
	dprintf("Dayna: In init_driver() for my driver\n");
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
		return -1;
	
	if (decode_scsi_ptl(name, &(device->bus), &(device->target), &device->lun) != 0) {
		dprintf("Dayna: Failed to parse the device name");
		free(device);
		return -1;
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
	return B_ERROR;
}


/* ----------
	my_device_write - handle write() calls
----- */

static status_t
my_device_write (void* cookie, off_t position, const void* buffer, size_t* num_bytes)
{
	dprintf("Dayna: In my_device_write(%p, %Ld, %p, %p) for my driver\n", cookie,
		position, buffer, num_bytes);
	*num_bytes = 0;				/* tell caller nothing was written */
	return B_ERROR;
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
	
	if (op == B_RAW_DEVICE_COMMAND){
		raw_device_command *rdc = (raw_device_command *) arg;
		return do_raw_command(cam, rdc, device->bus, device->target, device->lun);
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
	int			path, targ, lun;	/* path, target, lun ids */
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
	for (path = 0; path <= max_path ; path++) {
		ccbh->cam_path_id = path;
		for (targ = 0; targ < 16; targ++) {
			ccbh->cam_target_id = targ;
			for (lun = 0; lun < 8; lun++) {
				ccbh->cam_target_lun = lun;
				(*cam->xpt_action)(ccbh);	/* do get_device_type */
				if (ccbh->cam_status != CAM_REQ_CMP)
					continue;
				if (((CCB_GETDEV *)ccbh)->cam_pd_type != DEVICE_TYPE_CPU)
					continue;
					
				// JSR_TODO - Do an inquiry here and confirm that the vendor
				// is "Dayna" and the device is "SCSI/Link"
				add_device(path, targ, lun);
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