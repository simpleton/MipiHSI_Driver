#include "Private.h"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// *                                                                           *
// *                          RX operations                                    *
// *                                                                           *
// *                                                                           *
// *  These methods are for RX.                                                *
// *  Common methods for read access from user space or kernel space.          *
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
// clear_rx_buffers
//
// Loop for each channel and clears the RX buffer for those channels that
// match to the parameter para.
// Cleared buffers are moved to free buffer list.
// No pending reads are awakened, they will keep on waiting data.
// If a reader is transferring data while cleaning up takes action,
// the returned data may be corrupted (which should not be a problem).
//
// The parameter "para" can have the following values (all OR-able)
// 0..(HSI_CHANNEL_COUNT-1)	: Clear that channel
// HSI_INVERT			: Clear all but the above channel (OR this with the channel number)
// HSI_UNOPEN_ONLY		: Clear all unopened channels
// HSI_ALL_CH			: Clear all channels
//----------------------------------------------------------------------------

static void clean_rx_buffers(IN WDFDEVICE device, int para)
{
	
}

//----------------------------------------------------------------------------
//  free_rx
//
//  Deallocates the RX URBs and RX buffers allocated in init_rx.
//  Called at device shutdown, i.e. hw_dev->hw_status != HW_INITED.
//  Do not call while URB or BUF locks are held.
//  Submitted urbs were killed before this by the usb unregister procedure
//----------------------------------------------------------------------------

static void free_rx(IN WDFDEVICE device)
{

	
}

//----------------------------------------------------------------------------
//  init_rx
//
//  Allocates the RX URBs and RX buffers
//  Do not call while URB or BUF locks are held.
//----------------------------------------------------------------------------

NTSTATUS 
init_rx(IN WDFDEVICE device)
{	
	
	return 0;
}

//----------------------------------------------------------------------------
//  submit_free_rx_urbs
//
//  Checks if free urbs list has members && there are free buffers.
//  If yes, submits an URB with a free buffer.
//  Make sure HSI_RX_URB and HSI_RX_BUF locks are NOT held when calling this.
//  Returns the number for urbs that were submitted.
//----------------------------------------------------------------------------

static int submit_free_rx_urbs(IN WDFDEVICE device)
{
	
}

//----------------------------------------------------------------------------
//  read_rx_frames
//
//  This is a common function for both userland originated read 
//  and kernel originated read.
//  Reads the frames from the received frame buffer and copies to
//  the client's space (i.e. userland buffer or a kernel memory location, according
//  to who had opened the channel. Note: Kernel mode is for future use)
//
//  Returns :
//  >=0     : Success. Number of frames read.
//  -ENODEV : No HW
//  -EWOULDBLOCK : No data (if doing nonblocking read)
//  -EINTR  : Blocking read interrupted by signal
//----------------------------------------------------------------------------

static ssize_t read_rx_frames(IN WDFDEVICE device, 
			      size_t frames, hsi_frame_t *pframe, 
			      const bool nonblock)
{
	ssize_t frames_copied;

	return frames_copied;
}

//----------------------------------------------------------------------------
//  rx_complete
//
//  The callback from kernel after USB bulk rx transfer completes.
//  This is in interrupt context.
//----------------------------------------------------------------------------

static void rx_complete(void)
{
	

}




