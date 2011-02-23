//============================================================================
//  HSI-over-USB Driver for Linux kernel 2.6.x
//
//  hsi_usb_hw.h
//
//  This code is included to hsi.c in compile time.
//  These definitions are for hsi_usb_hw.c
//
//
//  NOTE: Code in this listing is formatted with tab width = 4
//  in order to yield printer friendlier layout.
//  If you use K-R or Linus preferred tab width, you may experience some
//  degradation of readibility of this code. 
//
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  Copyright Asamalab (2009)
// 
//  Author: Heikki Paananen <info@asamalab.com>
// 
//  Kudos to Pekka Korpinen at Nokia and Mikko Hurskainen at Notava.
// 
//  This program is derived from a program having a LGPL status. Contact
//  Asamalab if you prefer using the LGPL version.
//============================================================================

#ifndef __HSI_USB_HW_H__
#define __HSI_USB_HW_H__


#define HSI_USB_VENDOR_ID_DEF	0x04b4
#define HSI_USB_PRODUCT_ID_DEF	0x8613
#define HSI_USB_VENDOR_ID_ALT	0x2112
#define HSI_USB_PRODUCT_ID_ALT	0x0001
#define HSI_USB_VENDOR_ID_DIST	0x058B
#define HSI_USB_PRODUCT_ID_DIST	0x004B

#define HSI_BULK_TIMEOUT	HZ	// In jiffies. Approx. 1 sec

//----------------------------------------------------------------------------
// JTAG
//----------------------------------------------------------------------------

#define JTAG  	0

#if JTAG!=0
#	warning : Firmware upload via JTAG
#endif

//----------------------------------------------------------------------------
// USB end point mapping
//----------------------------------------------------------------------------

#define USB_FIRMWARE_EP_WRITE			0/*1*/
#define USB_FIRMWARE_EP_READ			1/*1*/


#define USB_WRITE_DATA_EP				2/*2*/
#define USB_WRITE_CONF_EP				3/*4*/
#define USB_READ_DATA_EP				4/*6*/
#define USB_READ_CONF_EP				5/*8*/

//----------------------------------------------------------------------------
// USB configuration commands
//----------------------------------------------------------------------------

//--------------------------------------------------------------------------------
//  Command	code	parameters		return		description
//--------------------------------------------------------------------------------

//  EPx_BC	0x01		none			Ret+FS+FC	Shows the status and byte count of each FIFO	
//  EPx_ST	0x02		none			Ret+FS		Shows the quick status of each EP	
//  EPx_CL	0x03		EPnum			Ret+FS		Clear endpoint (reject data)
//  EPx_FE	0x04		none			Ret+FS		Force packet end for endpoints (flush)
//  I_ARM 	0x05		none			Ret+CS		Set ports for fw upload (phase 1)
//  I_FIRE	0x06		none			Ret+CS		Set ports for fw upload (phase 2)
//  I_DONE	0x07		none			Ret+CS		Set ports for fw upload (done)
//  FWDATA	0x08		1..63 bytes		Ret+CS		Upload data to FPGA
//  CONFST	0x09     	none			Ret+CS		Shows configuration bits
//
//  EPnum = Number of the endpoint (2, 4, 6, 8)
//
//  Each command will return at least 2 parameter bytes
//  1st parameter: Ret		: Success = 00h, Failure = FFh, F0h = no data
//  2nd parameter: FS		: FIFO status (EP2468STAT)
//	             : CS		: Configuration status
//
//  Configuration bits for CS
//  bit 0 = USB speed		(Port A bit 0)
//  bit 1 = CONF_DONE state	(Port D bit 1)
//  bit 2 = nCONFIG			(Port D bit 2)
//  bit 3 = MSEL0			(Port D bit 3)
//  bit 4 = pin56			(Port D bit 4)
//  bit 5 = pin1			(Port D bit 5)
//  bit 6 = pin2			(Port D bit 6)
//  bit 7 = pin3			(Port D bit 7)
//
//  FIFO byte counts frame FC
//  EPxCS  : Endpoint status for each byte (4 bytes)
//  EPxBC  : FIFO byte count (current packet) (8 bytes)
//  EPxTBC : Total byte count for each FIFO (8 bytes)

#define EPx_BC	0x01
#define EPx_ST	0x02
#define EPx_CL	0x03
#define EPx_FE	0x04
#define I_ARM 	0x05
#define I_FIRE	0x06
#define I_DONE	0x07
#define FWDATA	0x08
#define CONFST	0x09

unsigned char get_regbits(unsigned char *regs, int loc, int pos, int width);
void set_regbits(unsigned char *regs, int loc, int pos, int width, int value);
NTSTATUS hsi_control(IN WDFDEVICE device, unsigned long addr, int bytes, char *data);
int hsi_write_bulk(IN WDFDEVICE device, unsigned int Pipe_index ,unsigned char *buf, int bytes);
int hsi_read_bulk(IN WDFDEVICE device, unsigned int Pipe_index ,unsigned char *buf, int bytes);
NTSTATUS config_usb_if(IN WDFDEVICE device, char *firmware,int size);
NTSTATUS image_upload(IN WDFDEVICE device);
NTSTATUS firmware_upload(IN WDFDEVICE device);
NTSTATUS clear_usb_hw_fifo(IN WDFDEVICE device, unsigned char endpoint);
NTSTATUS status_usb_hw_fifo(IN WDFDEVICE device, struct usb_hw_fifo *fifo);
NTSTATUS init_to_defaults(IN WDFDEVICE device);
NTSTATUS get_hw_regs(IN WDFDEVICE device, int start_addr, int count);
NTSTATUS  set_hw_regs(IN WDFDEVICE device, int start_addr, int count);
NTSTATUS hw_init(IN WDFDEVICE device);


#endif // __HSI_USB_HW_H__

