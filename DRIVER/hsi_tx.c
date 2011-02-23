#include "Private.h"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// *                                                                           *
// *                          TX operations                                    *
// *                                                                           *
// *                                                                           *
// *  These methods are for sending data to HSI bridge.                        *
// *  Both user space write and kernel space write share these methods.        *
// *                                                                           *
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
//  Copyright Asamalab (2009, 2010)
// 
//  Author: Heikki Paananen <info@asamalab.com>
// 
//  Kudos to Pekka Korpinen at Nokia and Mikko Hurskainen at Notava.
// 
//  This program is derived from a program having a LGPL status. Contact
//  Asamalab if you prefer using the LGPL version.
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// clear_tx_buffers
//
// Interrupts all pending writes that are queueing in send_tx_frames() and
// not yet submitted to URBs.
// hw_dev->block_tx_activity needs to be true while calling this to prevent
// new write attempts while here.
//----------------------------------------------------------------------------

static void clean_tx_buffers(struct hsi_hw_dev *hw_dev, int para)
{
	
}

//----------------------------------------------------------------------------
//  kill_tx_urbs
//
// Loop for submitted TX URBs and kill all them
// Note that same URB may be tried to kill several times as it is the callback
// which will actually move the URB to the free urb list.
//----------------------------------------------------------------------------

static void kill_tx_urbs(struct hsi_hw_dev *hw_dev)
{
	
}

//----------------------------------------------------------------------------
//  free_tx
//
//  Deallocates the TX URBs allocated in init_tx.
//  Called at device shutdown, i.e. hw_dev->hw_status != HW_INITED.
//  Do not call while URB or BUF locks are held.
//----------------------------------------------------------------------------

static void free_tx(struct hsi_hw_dev *hw_dev)
{
	
}

//----------------------------------------------------------------------------
//  init_tx
//
//  Allocates the TX URBs, initializes the urb array. 
//----------------------------------------------------------------------------

NTSTATUS 
init_rx(IN WDFDEVICE device);

{
	return 0;
}

//----------------------------------------------------------------------------
//  send_tx_frames
//
//  The set of frames is wrapped to a superframe, containing a header and
//  max HSI_MAX_TAG_FRAME_CNT frames. The header structure is:
//
//     MSB       MSB-1    LSB+1     LSB
//  -----------------------------------------
//  |  0xBE  |   0xEF  |  para1  |  para2   |
//  -----------------------------------------
//	
// para1 & 0xF0	: header type 
//---------------------------------------------------------
//: 0x00	: Header describes a superframe
//			: para1 = channel
// 			: para2 = number of frames in this superframe
//: 0xF0	: Header is a command: Describes TX channel properties: 
//			- Transmission mode
//			- Channel Descriptor Bit Field Width
//			- Data rate
//			: para1[3] = Transmission mode (0=stream, 1=frame)
//			: para1[2..0] = Width (legal values are 0..4)
//			: para2 = TX data rate = 192 / (para2 + 1) Mbps.
//
// Examples:
// 0xBE, 0xEF, 0xF6, 0x01	: Transmission mode is frame transmission.
//							  The channel descriptor bit field width will be 2.
//							  The TX data rate will be 96Mbps.
//
// TX channel properties that affect to the structure of the TX frame are
// modified by inserting a command header in to the frame stream instead of using configuration 
// endpoint. This ensures that modifications are in sync with the TX data flow.
//
//
//  Call parameters:
//  *ch_dev        : Pointer to channel device structure. Contains the channel information.
//  n              : Number of frames
//  *psource       : Pointer to the source buffer (can be NULL with command)
//  *actual_frames : Pointer to a integer that will hold the number of frames
//                   successfully sent at the return.
//
//  If ch_dev->channel is COMMAND_CHANNEL, it is a request for sending a command
//  to change TX bit rate, channel description field width or transmission mode.
//  Those parameters should be sent in sync with link layer control data.
//  If a command, ch_dev->cmd_para contains the (para1 << 8 | para2) data.
//
//  Returns:
//  0       : No errors. All frames sent.
//  -ENODEV : No HW
//  -EINTR  : Write interrupted by signal (user space app terminated)
//  -EWOULDBLOCK : No URBs available. Timed out.
//  -EFAULT : Failed to submit URB.
//
//  Note that this function will consider data being successfully sent if the URB submission
//  succeeds. If the USB subsystem or the bridge looses the frames, that is not communicated
//  back to the caller of this function.
//  
//  Nota also that although commands are sent in sync with the data, any link layer
//  arbitration should be done from same calling thread as where the commands originate from.
//  That is due to that any data intented to be sent before a command may end up being sent
//  after the command if there were no URBs available at that time. Running command from
//  same thread ensures that the commands and data are submitted over USB in correct order.
//
//  The data to be transmitted will be split into 255 frame blocks before submit.
//  For best TX performance the data block to be sent should ideally be a multiple
//  of 255 frames (or close below). Note that Linux shell split all data
//  to 4096 byte blocks, thus each block is sent over USB in 5 packets, first
//  four containing header + 255 frames, and the last one header + 4 frames.
//  That mean that every fifth USB packet is almost empty. 
//----------------------------------------------------------------------------

static ssize_t send_tx_frames(struct hsi_ch_dev *ch_dev, size_t n,
                              hsi_frame_t *psource, int *actual_frames)
{
	
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//                              CALLBACK                                     *
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

//----------------------------------------------------------------------------
//  tx_complete
//
//  The callback after USB bulk tx transfer completes.
//  This frees the URB that was used for this tx transfer
//----------------------------------------------------------------------------

static void tx_complete(HSI_USB_CALLBACK_PARA)
{
	
}

