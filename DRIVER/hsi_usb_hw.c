#include "private.h"
//----------------------------------------------------------------------------
//  get_regbits
//
//  Retrieves a bitfield from a register location.
//
//  Parameters:
//	*regs		: Pointer to the buffer holding a copy of the HW registers
//	loc			: Register address
//	pos			: Bit field start in the register byte
//	width		: Bit field width [bits]
//
//	Return		: The value of the bit field
//----------------------------------------------------------------------------

unsigned char get_regbits(unsigned char *regs, int loc, int pos, int width)
{
	return (regs[loc] >> pos) & (0xFF >> (8 - width));
}

//----------------------------------------------------------------------------
//  get_regbits
//
//  Writes to a bitfield in a register location.
//
//  Parameters:
//	*regs		: Pointer to the buffer holding a copy of the HW registers
//	loc			: Register address
//	pos			: Bit field start in the register byte
//	width		: Bit field width [bits]
//	value		: Content of the bit field
//----------------------------------------------------------------------------

void set_regbits(unsigned char *regs, int loc, int pos, int width, int value)
{
	regs[loc] = (regs[loc] & (~((0xFF >> (8 - width)) << pos))) | (value << pos);
}

//----------------------------------------------------------------------------
//	hsi_control
//
//	Sends data to the USB dongle's control end-point
//----------------------------------------------------------------------------

NTSTATUS
hsi_control(
	IN WDFDEVICE device, unsigned long addr, int bytes, char *data
	)
{	
	char 							*buf;
	WDF_MEMORY_DESCRIPTOR 			memoryDescriptor;
	WDF_USB_CONTROL_SETUP_PACKET 	controlPacket;
	PDEVICE_CONTEXT					hw_dev;
	NTSTATUS						status = STATUS_SUCCESS;

	hw_dev = GetDeviceContext(device);
	buf = ExAllocatePool(NonPagedPool,bytes);
	
	if (buf == NULL)
		return STATUS_NO_MEMORY;

	memcpy(buf, data, bytes);
	WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memoryDescriptor,
                                      buf,
                                      bytes);
	
	WDF_USB_CONTROL_SETUP_PACKET_INIT_VENDOR(
                            &controlPacket,
                            BmRequestHostToDevice,
                            BmRequestToDevice,
                            0xa0,
                            addr,
                            0);
	controlPacket.Packet.bm.Request.Type =  BmRequestVendor;
	
	if (hw_dev->hw_status == HW_INITED || hw_dev->hw_status == HW_COLD)
	{
		  InterlockedIncrement(&hw_dev->bulk_count);
		  status = WdfUsbTargetDeviceSendControlTransferSynchronously(
                            hw_dev->WdfUsbTargetDevice,
                            NULL,
                            NULL,
                            &controlPacket,
                            &memoryDescriptor,
                            NULL);
		  InterlockedDecrement(&hw_dev->bulk_count);
	}
	
	if (status != STATUS_SUCCESS)
	{
		UsbSamp_DbgPrint(3, ("Error %i in sending control msg \n", status));
	}

	ExFreePool (buf);
	return status;
}

//----------------------------------------------------------------------------
//  hsi_write_bulk
//
//  Sends a bulk message to the USB dongle's end-point ep
//  Note: buffer *buf should have been allocated with GFP_NOIO flag
//----------------------------------------------------------------------------

int
hsi_write_bulk(
	IN WDFDEVICE device, unsigned int Pipe_index ,unsigned char *buf, int bytes
	)
{
	NTSTATUS			status = STATUS_SUCCESS;
	PDEVICE_CONTEXT		hw_dev;
	int err; 
	int bytes_actual;
	WDFUSBPIPE pipe;
	WDF_MEMORY_DESCRIPTOR  memoryDescriptor;
	WDF_REQUEST_SEND_OPTIONS send_option;
	WDF_USB_PIPE_INFORMATION pipe_info;	
	
	
	WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memoryDescriptor,
									  buf,
									  bytes);

	//some send option initialize
	WDF_USB_PIPE_INFORMATION_INIT(&pipe_info);
	
	WDF_REQUEST_SEND_OPTIONS_INIT(
								  &send_option,
								  0
								  );
	
	WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(
										 &send_option,
										 WDF_REL_TIMEOUT_IN_SEC(6)
										 );

	/*if (KeGetCurrentIrql() == PASSIVE_LEVEL)
	{
		UsbSamp_DbgPrint(3, ("##PASSIVE_LEVEL##\n"));
	}else
	{
		UsbSamp_DbgPrint(3, ("NOT_IN##PASSIVE_LEVEL##\n"));
	}*/
	
	bytes_actual = 0;
	hw_dev = GetDeviceContext(device);	
	switch (Pipe_index)
	{
		case USB_FIRMWARE_EP_WRITE:
			pipe   =  hw_dev->Pipe_Firmware_Write;
			break;
		case USB_FIRMWARE_EP_READ:
			pipe   =  hw_dev->Pipe_Firmware_Read;
			break;
		case USB_READ_CONF_EP:
			pipe   =  hw_dev->Pipe_Conf_Read;
			break;
		case USB_WRITE_CONF_EP:
			pipe   =  hw_dev->Pipe_Conf_Write;
			break;
		case USB_READ_DATA_EP:
			pipe   =  hw_dev->Pipe_Data_Read;
			break;
		case USB_WRITE_DATA_EP:
			pipe   =  hw_dev->Pipe_Data_Write;
			break;
		default:
			UsbSamp_DbgPrint(3, ("hsi_Write_bulk : Pipe index ERROR"));
			bytes_actual = -1;
			goto end;
	}
	
	if (hw_dev->hw_status == HW_INITED || hw_dev->hw_status == HW_COLD)
	{
		InterlockedIncrement(&hw_dev->bulk_count);

		status = WdfUsbTargetPipeWriteSynchronously(
													pipe,
													NULL,
													&send_option,
													&memoryDescriptor,
													&bytes_actual
													);
		
		InterlockedDecrement(&hw_dev->bulk_count);
	}

	if (!NT_SUCCESS(status))
	{
		switch (status)
		{
			case STATUS_INFO_LENGTH_MISMATCH:
				UsbSamp_DbgPrint(3, ("STATUS_INFO_LENGTH_MISMATCH\n"));
				break;
			case STATUS_INVALID_PARAMETER:
				UsbSamp_DbgPrint(3, ("STATUS_INVALID_PARAMETER\n"));
				break;
			case STATUS_INSUFFICIENT_RESOURCES:
				UsbSamp_DbgPrint(3, ("STATUS_INSUFFICIENT_RESOURCES\n"));
				break;
			case STATUS_INVALID_DEVICE_REQUEST:
				UsbSamp_DbgPrint(3, ("STATUS_INVALID_DEVICE_REQUEST\n"));
				break;
			case STATUS_IO_TIMEOUT:
				UsbSamp_DbgPrint(3, ("STATUS_IO_TIMEOUT\n"));
				break;
			case STATUS_REQUEST_NOT_ACCEPTED:
				UsbSamp_DbgPrint(3, ("STATUS_REQUEST_NOT_ACCEPTED\n"));
				break;
			case STATUS_INVALID_BUFFER_SIZE:
				UsbSamp_DbgPrint(3, ("STATUS_INVALID_BUFFER_SIZE\n"));
				break;
			default:
				UsbSamp_DbgPrint(3, ("STATUS_UNKNOWN_ERROR:%x\n",status));
		}
	}
	else
	{
		if (bytes_actual < bytes)
		{
			UsbSamp_DbgPrint(3, ("Send too few bytes: %i (!= %i)\n", bytes_actual, bytes));
		}
		else 
		{
			goto end;
		}
	}
	UsbSamp_DbgPrint(3, ("bytes: %i (!= %i)\n", bytes_actual, bytes));
end:
	return bytes_actual;
}

//----------------------------------------------------------------------------
//  hsi_read_bulk
//
//  reads data from USB dongle's end-point ep
//  Note: buffer *buf should have been allocated with GFP_NOIO flag
//----------------------------------------------------------------------------

int
hsi_read_bulk(
	IN WDFDEVICE device,unsigned int Pipe_index ,unsigned char *buf, int bytes
	)
{
	NTSTATUS			status = STATUS_SUCCESS;
	PDEVICE_CONTEXT		hw_dev;
	int 				err; 
	int					bytes_actual;
	WDFUSBPIPE			pipe;
	WDF_MEMORY_DESCRIPTOR  memoryDescriptor;
	WDF_USB_PIPE_INFORMATION pipe_info;	


	
	WDF_USB_PIPE_INFORMATION_INIT(&pipe_info);

	WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memoryDescriptor,
									  buf,
									  0x200);
	
	bytes_actual = 0;
	err = 0;

	hw_dev =  GetDeviceContext(device);
	switch (Pipe_index)
	{
		case USB_FIRMWARE_EP_WRITE:
			pipe   =  hw_dev->Pipe_Firmware_Write;
			break;
		case USB_FIRMWARE_EP_READ:
			pipe   =  hw_dev->Pipe_Firmware_Read;
			break;
		case USB_READ_CONF_EP:
			pipe   =  hw_dev->Pipe_Conf_Read;
			break;
		case USB_WRITE_CONF_EP:
			pipe   =  hw_dev->Pipe_Conf_Write;
			break;
		case USB_READ_DATA_EP:
			pipe   =  hw_dev->Pipe_Data_Read;
			break;
		case USB_WRITE_DATA_EP:
			pipe   =  hw_dev->Pipe_Data_Write;
			break;
		default:
			UsbSamp_DbgPrint(3, ("hsi_read_bulk : Pipe index ERROR"));
			bytes_actual = -1;
			goto end;
	}
	
	UsbSamp_DbgPrint(3, ("hsi_read_bulk start\n"));
	
	if (hw_dev->hw_status == HW_INITED || hw_dev->hw_status == HW_COLD)
	{
		InterlockedIncrement(&hw_dev->bulk_count);
		
		status = WdfUsbTargetPipeReadSynchronously(
												pipe,
												NULL,
												NULL,
												&memoryDescriptor,
												&bytes_actual
												);
		InterlockedDecrement(&hw_dev->bulk_count);
	}

	if (!NT_SUCCESS(status))
		{
			switch (status)
			{
				case STATUS_INFO_LENGTH_MISMATCH:
					UsbSamp_DbgPrint(3, ("STATUS_INFO_LENGTH_MISMATCH\n"));
					break;
				case STATUS_INVALID_PARAMETER:
					UsbSamp_DbgPrint(3, ("STATUS_INVALID_PARAMETER\n"));
					break;
				case STATUS_INSUFFICIENT_RESOURCES:
					UsbSamp_DbgPrint(3, ("STATUS_INSUFFICIENT_RESOURCES\n"));
					break;
				case STATUS_INVALID_DEVICE_REQUEST:
					UsbSamp_DbgPrint(3, ("STATUS_INVALID_DEVICE_REQUEST\n"));
					break;
				case STATUS_IO_TIMEOUT:
					UsbSamp_DbgPrint(3, ("STATUS_IO_TIMEOUT\n"));
					break;
				case STATUS_REQUEST_NOT_ACCEPTED:
					UsbSamp_DbgPrint(3, ("STATUS_REQUEST_NOT_ACCEPTED\n"));
					break;
				case STATUS_INVALID_BUFFER_SIZE:
					UsbSamp_DbgPrint(3, ("STATUS_INVALID_BUFFER_SIZE\n"));
					break;
				default:
					UsbSamp_DbgPrint(3, ("STATUS_UNKNOWN_ERROR:%x\n",status));

			}
		}
		else
		{
			if (bytes_actual < bytes)
			{
				UsbSamp_DbgPrint(3, ("Send too few bytes: %i (!= %i)\n", bytes_actual, bytes));
				bytes_actual = -1;
				goto end;
			}
			else
			{
				UsbSamp_DbgPrint(3, ("hsi_read_bulk end:bytes_actual=%d\n",bytes_actual));
				goto end;
			}
		}
		UsbSamp_DbgPrint(3, ("hsi_read_bulk end\n"));
end:
		return bytes_actual;
	
}

//----------------------------------------------------------------------------
//  config_usb_if
//
//  Uploads the usb i/f configuration to the dongle's CPU memory
//----------------------------------------------------------------------------

NTSTATUS
config_usb_if(
	IN WDFDEVICE device, char *firmware,int size
	)
{
	enum {START = 0, STOP = 1};
	int	 				addr;
	int					err;
	int 				bytes;
	char 				mode;
	PDEVICE_CONTEXT		hw_dev;	
	NTSTATUS 			status;
	
	UsbSamp_DbgPrint(3, ("Start config_usb_if\n"));
	hw_dev = GetDeviceContext(device);
	err = 0;
	addr = 0;
	
	mode = STOP;
	hsi_control(device, 0xe600, 1, &mode);	
	UsbSamp_DbgPrint(3, ("config_usb_if:addr=%d size=%d\n",addr,size));
	
	while (addr < size)
	{
		bytes = size - addr;
		if (bytes > 16)
			bytes = 16;

		status = hsi_control(device, addr, bytes, firmware + addr);
		
		if (!NT_SUCCESS(status))
		{
			UsbSamp_DbgPrint(3, ("hsi_control ERROR\n"));
			break;
		}
		addr += bytes;
	}
	
	mode = START;
	hsi_control(device, 0xe600, 1, &mode);	
	UsbSamp_DbgPrint(3, ("End config_usb_if\n"));

	return err;
}

//----------------------------------------------------------------------------
//  image_upload
//
//  Uploads the firmware image to the dongle
//----------------------------------------------------------------------------

NTSTATUS
image_upload(
	IN WDFDEVICE device
	)
{
	
	int n;
	int err;
	unsigned long c;
	unsigned char *buf;
	PDEVICE_CONTEXT		hw_dev;	
	NTSTATUS 				status=STATUS_SUCCESS;
	
	c = 0;
	err = 0;
	buf = ExAllocatePool(NonPagedPool,64);
	if (buf == NULL)
		return STATUS_NO_MEMORY;

	hw_dev = GetDeviceContext(device);
	while (c < firmware_image_len)
	{
		n = min((firmware_image_len - c), (unsigned long)63);
	
		buf[0] = FWDATA;
		memcpy(buf + 1, firmware_image + c, n);

		err = hsi_write_bulk(device,USB_FIRMWARE_EP_WRITE ,buf, n + 1);	
		
		if (err < 0) break;
		c += (err - 1); 	
	}

	if (err < 0)
	{
		UsbSamp_DbgPrint(3,( "Firmware upload error %i\n", err));
		status = STATUS_SEVERITY_ERROR;
	}
	ExFreePool(buf);
	return status;
}

//----------------------------------------------------------------------------
//  firmware_upload
//
//  Uploads the CPU and USB firmware to the HSI dongle.
//----------------------------------------------------------------------------

NTSTATUS
firmware_upload(
	IN WDFDEVICE device
	)
{
	NTSTATUS			status = STATUS_SUCCESS;
	PDEVICE_CONTEXT		hw_dev;	
	char buf[0x200];
	int actual_len;
	
	hw_dev = GetDeviceContext(device);
	KeWaitForSingleObject(&hw_dev->Bulk_Event, Executive, KernelMode, FALSE, 0);
	UsbSamp_DbgPrint(3, ("Start firmware_upload usb_firmware=%d\n", usb_firmware_len));
	
	status = config_usb_if(device, usb_firmware, usb_firmware_len);
	if (status != STATUS_SUCCESS)
	{
		UsbSamp_DbgPrint(3, ("config_usb_if error\n"));
		goto end;
	}
	if (!JTAG)		
	{
		buf[0] = I_ARM;
		actual_len = hsi_write_bulk(device, USB_FIRMWARE_EP_WRITE ,buf, 1);
		if (actual_len < 0)
		{
			UsbSamp_DbgPrint(3, ("hsi_write_bulk I_ARM ERROR!!!\n"));
			goto end;
		}

		actual_len = hsi_read_bulk(device, USB_FIRMWARE_EP_READ, buf, 2);
		if (actual_len < 0)
		{
			UsbSamp_DbgPrint(3, ("hsi_read_bulk I_ARM ERROR!!!\n"));
			goto end;
		}

		if (buf[0] != 0x00) 
		{
			UsbSamp_DbgPrint(3, ("firmware_upload Read bulk I_ARM ERROR!!!\n"));
			status = STATUS_UNSUCCESSFUL;
			goto end;
		}

		buf[0] = I_FIRE;
		
		actual_len = hsi_write_bulk(device, USB_FIRMWARE_EP_WRITE , buf, 1);
		if (actual_len < 0)
		{
			UsbSamp_DbgPrint(3, ("hsi_write_bulk I_FIRE ERROR!!!\n"));
			goto end;
		}
		
		actual_len = hsi_read_bulk(device, USB_FIRMWARE_EP_READ, buf, 2);
		if (actual_len < 0)
		{
			UsbSamp_DbgPrint(3, ("hsi_write_bulk I_FIRE ERROR!!!\n"));
			goto end;
		}
		
		if (buf[0] != 0x00) 
		{
			status = STATUS_UNSUCCESSFUL;
			UsbSamp_DbgPrint(3, ("firmware_upload Read bulk I_FIRE ERROR!!!\n"));
			goto end;
		}


		status = image_upload(device);
		if (status != STATUS_SUCCESS)
		{
			status = STATUS_UNSUCCESSFUL;
			UsbSamp_DbgPrint(3, ("image_upload Error!!!\n"));
			goto end;
		}
	}
	buf[0] = I_DONE;
	actual_len = hsi_write_bulk(device, USB_FIRMWARE_EP_WRITE ,buf, 1);
	if (actual_len < 0) 
		goto error;
	actual_len = hsi_read_bulk(device, USB_FIRMWARE_EP_READ, buf, 2);
	if (actual_len < 0) 
		goto error;
	
	if ((buf[0] != 0x00) || (((buf[1] & 0x02) == 0x00) && !JTAG))
	{
		UsbSamp_DbgPrint(3, ("Firmware upload failed\n %02x %02x",buf[0],buf[1]));
		status = STATUS_SEVERITY_ERROR;
		goto end;
	}
	else
	{
		status = STATUS_SUCCESS;
		goto end;
	}
error:
	UsbSamp_DbgPrint(3, ("actual_len < 0\n"));
end:
	KeSetEvent(&hw_dev->Bulk_Event,IO_NO_INCREMENT,FALSE);
	return status;
}


//----------------------------------------------------------------------------
//  Clear_usb_hw_fifo
//
//  Clear an USB FIFO (on the dongle HW).
//  Endpoint parameter should have one of the values:
//  USB_WRITE_DATA_EP	(= 2)
//  USB_WRITE_CONF_EP	(= 4)
//  USB_READ_DATA_EP	(= 6)
//  USB_READ_CONF_EP	(= 8)
//----------------------------------------------------------------------------

NTSTATUS
clear_usb_hw_fifo(
	IN WDFDEVICE device, unsigned char endpoint
	)
{
	int err;
	unsigned char *buf;
	NTSTATUS 				status=STATUS_SUCCESS;
	
	return status;
}

//----------------------------------------------------------------------------
//  Status_usb_hw_fifo
//
//  Get the status of each EndPoint.
//  Returned to an array of usb-hw-fifo structures
//----------------------------------------------------------------------------


NTSTATUS
status_usb_hw_fifo(
	IN WDFDEVICE device, struct usb_hw_fifo *fifo
	)
{
	int err;
	int i;
	unsigned char *buf;
	NTSTATUS 				status=STATUS_SUCCESS;
	
	return status;
}


//----------------------------------------------------------------------------
//  init_to_defaults
//
//  Initializes a dongle to the default values
//  The default values are defined in hsi_default.h file.
//
//  Returns:
//  >= 0		: Success
//  -ETIMEDOUT	: Timeout error occured
//  -other		: Std kernel error code
//----------------------------------------------------------------------------

NTSTATUS
init_to_defaults(
	IN WDFDEVICE device
	)
{

	unsigned char *regs;
	PDEVICE_CONTEXT	hw_dev;

	hw_dev = GetDeviceContext(device);
	regs = hw_dev->hw_regs;

	// defaults from hsi_regs_defaults.h
		
	regs[RXCPR] = RXCPR_def; regs[TXCPR] = TXCPR_def; 
	regs[RXDRR] = RXDRR_def; regs[TXDRR] = TXDRR_def;
	regs[RXBCR] = RXBCR_def; regs[TXBCR] = TXBCR_def; 
	regs[RXFTR] = RXFTR_def; regs[RXWMR] = RXWMR_def;
	regs[RXTOR] = RXTOR_def; regs[RXFBR] = RXFBR_def; 
	regs[RXEFR] = RXEFR_def; regs[TXEFR] = TXEFR_def;
	regs[RXFOC] = RXFOC_def; regs[RXBDC] = RXBDC_def; 
	regs[RXTOC] = RXTOC_def; regs[SYSC0] = SYSC0_def;
	
	regs[IDR3]  =  IDR3_def; regs[IDR2]  =  IDR2_def; 
	regs[IDR1]  =  IDR1_def; regs[IDR0]  =  IDR0_def;
	regs[RXFC3] = RXFC3_def; regs[RXFC2] = RXFC2_def; 
	regs[RXFC1] = RXFC1_def; regs[RXFC0] = RXFC0_def;
	regs[TXFC3] = TXFC3_def; regs[TXFC2] = TXFC2_def; 
	regs[TXFC1] = TXFC1_def; regs[TXFC0] = TXFC0_def;
	regs[HSISR] = HSISR_def; regs[HSIBV] = HSIBV_def;
	regs[DIODE] = DIODE_def; regs[DIOPR] = DIOPR_def;

	UsbSamp_DbgPrint(3, ("Wrote HW defaults\n"));

	// Write to the dongle
	return set_hw_regs(device, 0, CONF_REGS);
	
}

//----------------------------------------------------------------------------
//  get_hw_regs
//
//  Retrieves the hardware information from a dongle.
//
//  Parameters:
//  *hw_dev	: Pointer to HW. The buffer too is availabe via this pointer
//  start_addr	: First register to read
//  count	: Number of registers to read
//
//  Returns:
//  -ETIMEDOUT	: Timeout error occured
//  -other	: Std kernel error code
//----------------------------------------------------------------------------

NTSTATUS
get_hw_regs(
	IN WDFDEVICE device, int start_addr, int count
	)
{
	int 				bytes;
	int 				retval;
	PDEVICE_CONTEXT 	hw_dev;
	NTSTATUS 			status=STATUS_SUCCESS;
	unsigned char 		loc_buf[CONF_REGS*2]={0};

	loc_buf[0] = 0xAC;
	loc_buf[1] = 0xDC;
	loc_buf[2] = 0x80 + start_addr;
	
	count = min(count, (int)(CONF_REGS - start_addr));
	loc_buf[3] = count;

	hw_dev	   = GetDeviceContext(device);

	retval = hsi_write_bulk(device, USB_WRITE_CONF_EP, loc_buf, 4);
	if (retval <= 0 || retval < 4){
		status = STATUS_SEVERITY_ERROR;
		goto quit;
	}
	
	retval = hsi_read_bulk(device, USB_READ_CONF_EP, loc_buf, count);
	if (retval <= 0){
		status = STATUS_SEVERITY_ERROR;
		goto quit;
	}

	retval = min(retval, (int)(CONF_REGS - start_addr));

	memcpy(hw_dev->hw_regs + start_addr, loc_buf, retval);
quit:	
	return status;
}


//----------------------------------------------------------------------------
//  set_hw_regs
//
//  Writes the hardware information to the dongle.
//
//  Parameters:
//  *hw_dev	: Pointer to HW. The buffer is availabe via this pointer too.
//  start_addr	: First register to be written
//  count	: Number of registers to to be written
//
//  Returns:
//  >= 0	: Bytes sent
//  < 0		: Error occured
//----------------------------------------------------------------------------

NTSTATUS 
set_hw_regs(
	IN WDFDEVICE device, 
	int start_addr, 	
	int count
	)
{
	NTSTATUS				status=STATUS_SUCCESS;
	int						err;
	int 					retval;
	unsigned char 			loc_buf[0x100]={0};
	PDEVICE_CONTEXT 		hw_dev;
	unsigned int			cmd_para;
	unsigned char 			command[4]={0};
	WDFMEMORY               reqMemory;
	WDFUSBPIPE 				pipe;
	BOOLEAN					ret;
	WDF_OBJECT_ATTRIBUTES  attributes;

	
	loc_buf[0] = 0xAC;
	loc_buf[1] = 0xDC;
	loc_buf[2] = 0x00 + start_addr;
	
	count = min(count, (int)(CONF_REGS - start_addr));
	loc_buf[3] = count;

	UsbSamp_DbgPrint(3, ("The commmand is %02x %02x %02x %02x\n",loc_buf[0],loc_buf[1],loc_buf[2],loc_buf[3]));
	
	hw_dev 	   = GetDeviceContext(device);
	
	KeWaitForSingleObject(&hw_dev->Bulk_Event, Executive, KernelMode, FALSE, 0);

	
	memcpy(loc_buf + 4, hw_dev->hw_regs + start_addr, count);
		
	retval = hsi_write_bulk(device, USB_WRITE_CONF_EP, loc_buf, count+4);

	if ( (start_addr <= TXCPR && TXCPR <= (start_addr + count)) ||		
		 (start_addr <= TXDRR && TXDRR <= (start_addr + count)) )	
	{
		pipe = hw_dev->Pipe_Data_Write;
		
		cmd_para = 0xF000 | 
		 		  (get_regbits(hw_dev->hw_regs, TXCHW) << 8)  |	
				  (get_regbits(hw_dev->hw_regs, TXTRM) << 11) |	
				  hw_dev->hw_regs[TXDRR];
	
		command[0] = 0xBE;
		command[1] = 0xEF;
		command[2] = (cmd_para & 0xFF00) >> 8;
		command[3] = cmd_para & 0xFF;
		
		
		hsi_write_bulk(device,USB_WRITE_DATA_EP,command,4);
	}
	
Exit:	
	KeSetEvent(&hw_dev->Bulk_Event,IO_NO_INCREMENT,FALSE); 
	UsbSamp_DbgPrint(3, ("End set_hw_regs\n"));

	return status;
}





//----------------------------------------------------------------------------
//  hw_init
//
//  Initializes the USB dongle. This job is placed on work queue when a
//  new dongle is attached.
//----------------------------------------------------------------------------

NTSTATUS 
hw_init(
	IN WDFDEVICE device
	)
{
	NTSTATUS			status = STATUS_SUCCESS;
	PDEVICE_CONTEXT		hw_dev;
	unsigned char 		*regs;
	int 				err;
	int 				count;
	
	do
	{
		hw_dev = GetDeviceContext(device);	
		if (hw_dev == NULL)
		{
			status = STATUS_SEVERITY_ERROR;
			UsbSamp_DbgPrint(1, ("hw_init hw_dev == NULL\n"));
			break;
		}

		regs = hw_dev->hw_regs;
	
		status = firmware_upload(device);	//Uploads firmware
		if (!NT_SUCCESS(status))			
		{
			UsbSamp_DbgPrint(1, ("hw_init hw_dev == NULL\n"));
			break;
		}

		
		
		hw_dev->hw_status = HW_INITED;	 // Declare that the dongle is ready


	}
	while (0); 

	if (status != STATUS_SUCCESS)
		hw_dev->hw_status = HW_INIT_FAILED;		// Dongle initialization failed
		
	return status;
}

