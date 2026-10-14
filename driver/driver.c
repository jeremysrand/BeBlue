/* ++++++++++
	driver.c
	A skeletal device driver
+++++ */

#include "../VersionStr.h"

#include <KernelExport.h>
#include <Drivers.h>
#include <Errors.h>

#include "scsi_common.h"

static cam_for_driver_module_info	*cam;
static const char cam_name[] = B_CAM_FOR_DRIVER_MODULE_NAME;


typedef struct
{
	int path;
	int target;
	int lun;
}  raw_scsi;

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
	kprintf("In init_hardware() for my driver, " VERSION_DETAIL "\n");
	return B_OK;
}


/* ----------
	init_driver - optional function - called every time the driver
	is loaded.
----- */
status_t
init_driver (void)
{
	kprintf("In init_driver() for my driver\n");
	int rc = add_debugger_command("daynaHello", daynaHello, "Say hello world");
	kprintf("add_debugger_command() = %d\n", rc);
	
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
	kprintf("In uninit_driver() for my driver\n");
	int rc = remove_debugger_command("daynaHello", daynaHello);
	kprintf("remove_debugger_command() = %d\n", rc);
	
	put_module(cam_name);
}

	
/* ----------
	my_device_open - handle open() calls
----- */

static status_t
my_device_open (const char *name, uint32 flags, void** c)
{
	kprintf("In my_device_open(%s, %u, %p) for my driver\n", name, flags,
		c);
		
	raw_scsi **cookie = (raw_scsi **) c;
	raw_scsi *d = NULL;     /* -> sci info */
	int	retval = -1;        /* assume failure */
	
	/* set up private data for this particular device */
	if (!(d = (raw_scsi *) malloc (sizeof(raw_scsi)))) {
		goto exit;
	}

	/* bus/scsi/ is 9 characters... */
	if(decode_scsi_ptl((char *) (name + 9), &(d->path), &(d->target), &(d->lun))) {
		goto exit;
	}
	retval = 0;
	*cookie = d;

exit:
	if (d && retval)
		free (d);

	return retval;
}


/* ----------
	my_device_read - handle read() calls
----- */

static status_t
my_device_read (void* cookie, off_t position, void *buf, size_t* num_bytes)
{
	kprintf("In my_device_read(%p, %Ld, %p, %p) for my driver\n", cookie,
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
	kprintf("In my_device_write(%p, %Ld, %p, %p) for my driver\n", cookie,
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
	kprintf("In my_device_control(%p, %u, %p, %u) for my driver\n", cookie,
		op, arg, len);
	raw_scsi *d = (raw_scsi *) cookie;
	raw_device_command *rdc = (raw_device_command *) arg;
	
	if (op != B_RAW_DEVICE_COMMAND){
		return B_DEV_INVALID_IOCTL;
	}

	return do_raw_command(cam, rdc, d->path, d->target, d->lun);
}


/* ----------
	my_device_close - handle close() calls
----- */

static status_t
my_device_close (void* cookie)
{
	kprintf("In my_device_close(%p) for my driver\n", cookie);
	return B_OK;
}


/* -----
	my_device_free - called after the last device is closed, and after
	all i/o is complete.
----- */
static status_t
my_device_free (void* cookie)
{
	kprintf("In my_device_free(%p) for my driver\n", cookie);
	return B_OK;
}


/* -----
	null-terminated array of device names supported by this driver
----- */

static const char *my_device_name[] = {
	"bus/scsi/0/4/0/raw",
	NULL
};

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

/* ----------
	publish_devices - return a null-terminated array of devices
	supported by this driver.
----- */

_EXPORT const char**
publish_devices()
{
	kprintf("In publish_devices() for my driver\n");
	return my_device_name;
}

/* ----------
	find_device - return ptr to device hooks structure for a
	given device name
----- */

_EXPORT device_hooks*
find_device(const char* name)
{
	kprintf("In find_device(%s) for my driver\n", name);
	return &my_device_hooks;
}


_EXPORT int32 api_version = B_CUR_DRIVER_API_VERSION;