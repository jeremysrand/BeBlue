/* scsi_common.c
** 
** Shared scsi utils.  
**
** 02/07/99 - swetland
*/

/*
	Copyright 1999, Be Incorporated.   All Rights Reserved.
	This file may be used under the terms of the Be Sample Code License.
*/

#include <stdio.h>
#include <string.h>

#include <SupportDefs.h>
#include <OS.h>
#include <CAM.h>
#include <Drivers.h>
#include <KernelExport.h>
#include <scsi.h>

#include "scsi_common.h"


// Defines

#define LOCKED_DATA   0x01
#define LOCKED_SENSE  0x02

#define SCSI_TIMEOUT 1000000

#define SCSI_INQUIRY 0x12
#define SCSI_INQ_TYPE_MASK 0x1f
#define SCSI_INQ_TYPE_OFFSET 0
#define SCSI_INQ_SCSI_VERSION_OFFSET 2
#define SCSI_INQ_VENDOR_STR_OFFSET 8
#define SCSI_INQ_DEVICE_STR_OFFSET 16
#define SCSI_INQ_VERSION_STR_OFFSET 32
#define SCSI_INQ_VENDOR_INFO_STR_OFFSET 36
#define SCSI_INQ_LEN 56

#define DAYNA_VENDOR_STR "Dayna"
#define DAYNA_VENDOR_STR_LEN 5
#define DAYNA_DEVICE_STR "SCSI/Link"
#define DAYNA_DEVICE_STR_LEN 9

#define SCSI_DATAPATH_ENABLE 0x0e

#define SCSI_SET_MODE 0x0c
#define SCSI_MODE_BROADCAST 0x04

#define SCSI_GET_STATS 0x09
#define SCSI_STATS_LEN 18

#define SCSI_WRITE_PKT 0x0a

#define SCSI_READ_PKT 0x08
#define SCSI_READ_FLAGS 0xc0

#define MAC_ADDR_LEN 6

#define PKT_HEADER_LEN 6
#define PKT_CRC_LEN 4
#define PKT_DROPPED 0xffffffffu
#define PKT_MORE_FLAG 0x10u
#define PKT_ETHERNET_HDR_LEN 14


// Implementation

status_t 
do_raw_command(cam_for_driver_module_info *cam,
			   raw_device_command *rdc, tDeviceId * id)
{
	CCB_HEADER *ccbh = NULL;
	CCB_SCSIIO *ccb;
	status_t result = B_ERROR;
	int mem_locked = 0;
	
	/* sanity check the command length */
	if((rdc->command_length != 6) &&
	   (rdc->command_length != 10) &&
	   (rdc->command_length != 12)) {
	   	dprintf("raw: bad command_length (%d)\n",rdc->command_length);
	   	goto drc_exit;
	}

	/* make sure data is there and lockable */
	if(rdc->data_length) {
		if(lock_memory(rdc->data, rdc->data_length, B_DMA_IO | B_READ_DEVICE)) {
			dprintf("raw: cannot lock data 0x%08x/%d\n",
					rdc->data,rdc->data_length);
			goto drc_exit;
		} else {
			mem_locked |= LOCKED_DATA;
		}
	}

	/* lock sense data if we've got it */
	if(rdc->sense_data_length){
		if(lock_memory(rdc->sense_data, rdc->sense_data_length, 
						B_DMA_IO | B_READ_DEVICE)) {
			dprintf("raw: cannot lock sense data 0x%08x/%d\n",
					rdc->sense_data,rdc->sense_data_length);
			goto drc_exit;
		} else {
			mem_locked |= LOCKED_SENSE;
		}
	}

	if(!(ccbh = (*cam->xpt_ccb_alloc)())) {
		goto drc_exit;
	}
	
	ccbh->cam_path_id = id->bus;
	ccbh->cam_target_id = id->target;
	ccbh->cam_target_lun = id->lun;
	ccbh->cam_flags = ((rdc->flags & B_RAW_DEVICE_DATA_IN) ? CAM_DIR_IN : CAM_DIR_OUT );

	ccb = (CCB_SCSIIO *) ccbh;
	ccb->cam_dxfer_len = rdc->data_length;
	ccb->cam_data_ptr = (uchar *) rdc->data;
	ccb->cam_sense_len = rdc->sense_data_length;
	ccb->cam_sense_ptr = (unsigned char *) rdc->sense_data;
	ccb->cam_cdb_len = rdc->command_length;
	memcpy(ccb->cam_cdb_io.cam_cdb_bytes, rdc->command, rdc->command_length);
		
	/* make it so */
	(*cam->xpt_action) (ccbh);

	rdc->scsi_status = ccb->cam_scsi_status;
	rdc->cam_status = ccb->cam_ch.cam_status;

	result = B_OK;
	
drc_exit:
	if((result == B_OK) && (rdc->flags & B_RAW_DEVICE_REPORT_RESIDUAL)){
		rdc->sense_data_length = rdc->sense_data_length - ccb->cam_sense_resid;
		rdc->data_length = rdc->data_length - ccb->cam_resid;
	}	
	if(ccbh) {
		(*cam->xpt_ccb_free)(ccbh);
	}
	if(mem_locked & LOCKED_DATA) {
		unlock_memory(rdc->data, rdc->data_length, B_DMA_IO|B_READ_DEVICE);
	}
	if(mem_locked & LOCKED_SENSE) {
		unlock_memory(rdc->sense_data, rdc->sense_data_length, 
						  B_DMA_IO|B_READ_DEVICE);
	}
	return result;
}


status_t execute_command(cam_for_driver_module_info *cam, tDeviceId * id,
	uint8 * command, uint8 commandLen, void * data, size_t dataLen)
{
	raw_device_command rdc;
	
	rdc.data = data;
	rdc.data_length = dataLen;
	rdc.sense_data = NULL;
	rdc.sense_data_length = 0;
	rdc.timeout = SCSI_TIMEOUT;
	rdc.flags = B_RAW_DEVICE_DATA_IN;
	rdc.command_length = commandLen;
	
	if (commandLen > sizeof(rdc.command)) {
		dprintf("Dayna: Comand length too long, len=%u, max=%u\n", (uint32)commandLen, (uint32)sizeof(rdc.command));
		return B_ERROR;
	}
	memcpy(rdc.command, command, commandLen);
	
	if (do_raw_command(cam, &rdc, id) != B_OK) {
		dprintf("Dayna: Command execution failed\n");
		return B_ERROR;
	}
	
	if (rdc.cam_status != CAM_REQ_CMP) {
		dprintf("Dayna: Expected CAM status %u but got %u\n", (uint32)CAM_REQ_CMP, (uint32)rdc.cam_status);
		return B_ERROR;
	}
	
	if (rdc.scsi_status != 0) {
		dprintf("Dayna: Expected SCSI status 0 but got %u", (uint32)rdc.scsi_status);
		return B_ERROR;
	}
	
	return B_OK;
}

 
int is_dayna_port(cam_for_driver_module_info *cam, tDeviceId * id)
{
	uint8 data[SCSI_INQ_LEN];
	uint8 command[] = { SCSI_INQUIRY, 0x00, 0x00, 0x00, sizeof(data), 0x00 };
	if (execute_command(cam, id, command, sizeof(command), data, sizeof(data)) != B_OK) {
		dprintf("Dayna: Inquiry command failed\n");
		return 0;
	}
	
	uint8 type = data[SCSI_INQ_TYPE_OFFSET] & SCSI_INQ_TYPE_MASK;
	if (type != SCSI_DEVICE_TYPE_CPU) {
		dprintf("Dayna: Expected device type %u but got %u\n", (uint32)SCSI_DEVICE_TYPE_CPU,
			(uint32)type);
		return 0;
	}
	
	if (strncmp((char *)&(data[SCSI_INQ_VENDOR_STR_OFFSET]), DAYNA_VENDOR_STR, DAYNA_VENDOR_STR_LEN) != 0) {
		dprintf("Dayna: Expected vendor to be \"%s\" but got \"%s\"\n", DAYNA_VENDOR_STR, &(data[SCSI_INQ_VENDOR_STR_OFFSET]));
		return 0;
	}
	
	if (strncmp((char *)&(data[SCSI_INQ_DEVICE_STR_OFFSET]), DAYNA_DEVICE_STR, DAYNA_DEVICE_STR_LEN) != 0) {
		dprintf("Dayna: Expected device to be \"%s\" but got \"%s\"\n", DAYNA_DEVICE_STR, &(data[SCSI_INQ_DEVICE_STR_OFFSET]));
		return 0;
	}
	
	dprintf("Dayna: Device is a Dayna SCSI/Link based on inquiry\n");
	return 1;
}

status_t enable_datapath(cam_for_driver_module_info *cam, tDeviceId * id,
	bool enable)
{
	uint8 command[] = { SCSI_DATAPATH_ENABLE, 0x00, 0x00, 0x00, 0x00, (enable ? 0x80 : 0x00) };
	if (execute_command(cam, id, command, sizeof(command), NULL, 0) != B_OK) {
		dprintf("Dayna: Enable datapath failed\n");
		return B_ERROR;
	}
	
	return B_OK;
}

status_t set_mode(cam_for_driver_module_info *cam, tDeviceId * id)
{
	uint8 command[] = { SCSI_SET_MODE, 0x00, 0x00, 0x00, SCSI_MODE_BROADCAST, 0x80 };
	if (execute_command(cam, id, command, sizeof(command), NULL, 0) != B_OK) {
		dprintf("Dayna: Set mode failed\n");
		return B_ERROR;
	}
	
	return B_OK;
}

status_t get_mac_addr(cam_for_driver_module_info *cam, tDeviceId * id,
	uint8 * buf)
{
	uint8 data[SCSI_STATS_LEN];
	uint8 command[] = { SCSI_GET_STATS, 0x00, 0x00, 0x00, sizeof(data), 0x00 };
	if (execute_command(cam, id, command, sizeof(command), data, sizeof(data)) != B_OK) {
		dprintf("Dayna: Get stats command failed\n");
		return B_ERROR;
	}
	
	memcpy(buf, data, MAC_ADDR_LEN);
	dprintf("Dayna: Returning MAC addr %02x:%02x:%02x:%02x:%02x:%02x\n",
		(uint32)data[0], (uint32)data[1], (uint32)data[2], (uint32)data[3],
		(uint32)data[4], (uint32)data[5]); 
	return B_OK;
}


status_t do_multi(cam_for_driver_module_info *cam, tDeviceId * id,
	char * addr)
{
	// JSR_TODO - Write this code...
	return B_ERROR;
}


status_t tx_packet(cam_for_driver_module_info *cam, tDeviceId * id,
	const void * packet, size_t packetLen)
{
	uint8 command[] = {
		SCSI_WRITE_PKT,
		0x00,
		0x00,
		(packetLen >> 8) & 0xff,
		packetLen && 0xff,
		0x00
	};
	if (execute_command(cam, id, command, sizeof(command), (void *)packet, packetLen) != B_OK) {
		dprintf("Dayna: Write packet command failed\n");
		return B_ERROR;
	}
	
	return B_OK;
}


static int get_packet_from_buffer(cam_for_driver_module_info *cam, tDeviceId * id,
	tRxBuffer * buffer, void * packet, size_t maxPacketSize)
{
	uint8 * end = buffer->data + RX_BUFFER_SIZE;
	uint8 * ptr = buffer->ptr;
	
	buffer->ptr = NULL;
	
	if (ptr < buffer->data)
		return 0;
		
	if (ptr + PKT_HEADER_LEN > end)
		return 0;
	
	uint32 pkt_len = ((uint32)ptr[0] << 8) | ptr[1];
	if (pkt_len == 0)
		return 0;
		
	uint32 flags = ((uint32)ptr[2] << 24) |
		((uint32)ptr[3] << 16) |
		((uint32)ptr[4] << 8) |
		((uint32)ptr[5]);
	if (flags == PKT_DROPPED) {
		dprintf("Dayna: Packet dropped, resetting\n");
		enable_datapath(cam, id, false);
		enable_datapath(cam, id, true);
		set_mode(cam, id);
		return 0;
	}
	
	if (pkt_len <= PKT_CRC_LEN) {
		dprintf("Dayna: Packet len %u too short\n", pkt_len);
		return 0;
	}
	
	uint32 frame_len = pkt_len - PKT_CRC_LEN;
	if (frame_len < PKT_ETHERNET_HDR_LEN) {
		dprintf("Dayna: Frame len %u too short\n", frame_len);
		return 0;
	}
	
	if (ptr + PKT_HEADER_LEN + frame_len > end) {
		dprintf("Dayna: Frame overruns the buffer, buffer=%p, ptr=%p, frame_len=%u\n",
			buffer->data, ptr, frame_len);
		return 0;
	}
	
	if (frame_len > maxPacketSize)
		frame_len = maxPacketSize;
		
	memcpy(packet, ptr + PKT_HEADER_LEN, frame_len);
	
	if ((flags & PKT_MORE_FLAG) != 0)
		buffer->ptr = ptr + pkt_len + PKT_HEADER_LEN;

	return frame_len;
}


static status_t fill_buffer(cam_for_driver_module_info *cam, tDeviceId * id,
	tRxBuffer * buffer)
{
	uint8 command[] = {
		SCSI_READ_PKT,
		0x00,
		0x00,
		(RX_BUFFER_SIZE >> 8) & 0xff,
		RX_BUFFER_SIZE && 0xff,
		SCSI_READ_FLAGS
	};
	
	// Must clear the first 10 bytes of the buffer to make sure that we correctly
	// identify no packet and don't just return what was there before if the read
	// doesn't change the buffer.  This clears the first 12 bytes which is more than
	// enough.
	uint32 * ptr = (uint32 *)buffer->data;
	ptr[0] = 0;
	ptr[1] = 0;
	ptr[2] = 0;
	
	buffer->ptr = NULL;
	
	if (execute_command(cam, id, command, sizeof(command), (void *)buffer->data, RX_BUFFER_SIZE) != B_OK) {
		dprintf("Dayna: Read packet command failed\n");
		return B_ERROR;
	}
	
	buffer->ptr = buffer->data;
	
	return B_OK;
}


int rx_packet(cam_for_driver_module_info *cam, tDeviceId * id,
	tRxBuffer * buffer, void * packet, size_t maxPacketSize)
{
	int result = get_packet_from_buffer(cam, id, buffer, packet, maxPacketSize);
	if (result != 0)
		return result;
	
	if (fill_buffer(cam, id, buffer) != B_OK)
		return B_ERROR;
		
	return get_packet_from_buffer(cam, id, buffer, packet, maxPacketSize);
}