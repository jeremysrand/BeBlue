#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <Directory.h>
#include <Entry.h>
#include <Path.h>
#include <scsi.h>
#include <CAM.h>


#define SCSI_BUS "/dev/bus/scsi"
#define SCSI_RAW_DEV_NAME "raw"
#define SCSI_SENSE_SIZE 16
#define SCSI_TIMEOUT 1000000

#define SCSI_INQUIRY 0x12

static void inquiry(const char * dev)
{
	int fd;
	int e;
	raw_device_command rdc;
	scsi_inquiry data;
	uchar sense[SCSI_SENSE_SIZE];
	
	printf("Running inquiry against dev = %s\n", dev);
	if ((fd = open(dev, 0)) < 0) {
		fprintf(stderr, "Unable top open dev %s, %s\n", dev, strerror(errno));
		return;
	}
	
	rdc.data = &data;
	rdc.data_length = sizeof(data);
	rdc.sense_data = sense;
	rdc.sense_data_length = 0;
	rdc.timeout = SCSI_TIMEOUT;
	rdc.flags = B_RAW_DEVICE_DATA_IN;
	rdc.command_length = 6;
	rdc.command[0] = SCSI_INQUIRY;
	rdc.command[1] = 0x00;
	rdc.command[2] = 0x00;
	rdc.command[3] = 0x00;
	rdc.command[4] = sizeof(data);
	rdc.command[5] = 0x00;
	
	e = ioctl(fd, B_RAW_DEVICE_COMMAND, &rdc, sizeof(rdc));
	if (e != 0) {
		fprintf(stderr, "Error from ioctl of dev %s, %s\n", dev, strerror(errno));
		close(fd);
	}
	
	if (rdc.cam_status != CAM_REQ_CMP) {
		fprintf(stderr, "Unexpected cam status on dev %s, %x\n", dev, (int) rdc.cam_status);
	}
	
	for (int i = 0; i < sizeof(data.inquiry_data); i++) {
		uchar ch = data.inquiry_data[i];
		printf("%2x: %2x %c\n", i, (int)ch, (isprint(ch) ? ch : ' '));
	}
	
	close(fd);
}

static void walkDevs(const char * path)
{
	BDirectory dir(path);
	if (dir.InitCheck() == B_OK) {
		BEntry entry;
		while (dir.GetNextEntry(&entry) >= 0) {
			BPath name;
			entry.GetPath(&name);
			if (entry.IsDirectory())
				walkDevs(name.Path());
			else if (strcmp(name.Leaf(), SCSI_RAW_DEV_NAME) == 0)
				inquiry(name.Path());
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc >= 2)
		inquiry(argv[1]);
	else
		walkDevs(SCSI_BUS);
	
	exit(0);
}