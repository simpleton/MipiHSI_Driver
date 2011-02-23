/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    private.h

Abstract:

    Contains structure definitions and function prototypes private to
    the driver.

Environment:

    Kernel mode

--*/

#pragma warning(disable:4200)  //
#pragma warning(disable:4201)  // nameless struct/union
#pragma warning(disable:4214)  // bit field types other than int

#include <initguid.h>
#include <ntddk.h>
#include <ntintsafe.h>
#include "usbdi.h"
#include "usbdlib.h"

#pragma warning(default:4200)
#pragma warning(default:4201)
#pragma warning(default:4214)

#include <wdf.h>
#include <wdfusb.h>
#include "public.h"

//HW head file
#include "hsi_firmware.h"
#include "hsi_default.h"
#include "hsi_usb_hw.h"
#include "hsi_usb_config.h"

#ifndef _H
#define _H

#define POOL_TAG (ULONG) 'SBSU'

#undef ExAllocatePool
#define ExAllocatePool(type, size) \
    ExAllocatePoolWithTag(type, size, POOL_TAG)

#define ExFreePool(aaa) \
	ExFreePoolWithTag (aaa,POOL_TAG); 

#if DBG

#define UsbSamp_DbgPrint(level, _x_) \
            if((level) <= DebugLevel) { \
                DbgPrint("MIPI_HSI_Dongle: "); DbgPrint _x_; \
            }

#else

#define UsbSamp_DbgPrint(level, _x_)

#endif


#define MAX_TRANSFER_SIZE   256
#define REMOTE_WAKEUP_MASK 0x20

#define DEFAULT_REGISTRY_TRANSFER_SIZE 65536

#define GetListHeadEntry(ListHead)  ((ListHead)->Flink)

#define IDLE_CAPS_TYPE IdleUsbSelectiveSuspend

//HW macro

typedef UINT32 hsi_frame_t;

//some enum status
enum hw_states				// HW states
{
	HW_DISABLED = 0,		// Dongle is not connected or will be soon shut down.
	HW_COLD,				// Dongle is attached, but no firmware yet. HSI func not available.
	HW_INIT_FAILED,			// Dongle initialization has failed (either URBs fail or upload fail)
	HW_INITED				// Dongle is up and running
};


//----------------------------------------------------------------------------
//  HSI Hardware
//
//  Each HW (i.e. dongle) is described with hsi_hw_dev structure.
//----------------------------------------------------------------------------

#define HSI_CHANNEL_COUNT	16		// Number of virtual channels / dongle

#define LOCK____HSI_RX_URB	spin_lock_irqsave(hw_dev->rx_urb_lock, urb_irqflags)
#define UNLOCK__HSI_RX_URB	spin_unlock_irqrestore(hw_dev->rx_urb_lock, urb_irqflags)
#define LOCK____HSI_RX_BUF	spin_lock_irqsave(hw_dev->rx_buf_lock, buf_irqflags)
#define UNLOCK__HSI_RX_BUF	spin_unlock_irqrestore(hw_dev->rx_buf_lock, buf_irqflags)

#define LOCK____HSI_CH		spin_lock_irqsave(ch_dev->ch_lock, ch_irqflags)
#define UNLOCK__HSI_CH		spin_unlock_irqrestore(ch_dev->ch_lock, ch_irqflags)

#define LOCK____HSI_TX_URB	spin_lock_irqsave(hw_dev->tx_urb_lock, urb_irqflags)
#define UNLOCK__HSI_TX_URB	spin_unlock_irqrestore(hw_dev->tx_urb_lock, urb_irqflags)


//----------------------------------------------------------------------------
//  Frame Header definitions
//
//  The communication between this driver and dongle's HW is kept in sync
//  with tag frames. Tags are added to the data stream in the driver and 
//  are removed before the data goes over the HSI PHY. Tag are again added
//  at receiver side dongle before moving data in to the driver, which
//  checks the validity and removes the tags before moving data on. 
//----------------------------------------------------------------------------

#define HSI_TAG			0xBEEF0000
#define HSI_IS_TAG(tag)		(((tag) & 0xFFFF0000) == HSI_TAG)
#define HSI_TAG_LEN(tag)	( (tag) & 0x000000FF)
#define HSI_TAG_CH(tag)		(((tag)>>8) & 0x0F)
#define HSI_MAX_TAG_FRAME_CNT	255


//
// A structure representing the instance information associated with
// this particular device.
//

typedef struct _DEVICE_CONTEXT {

    USB_DEVICE_DESCRIPTOR           UsbDeviceDescriptor;
    PUSB_CONFIGURATION_DESCRIPTOR   UsbConfigurationDescriptor;
    WDFUSBDEVICE                    WdfUsbTargetDevice;
    ULONG                           WaitWakeEnable;
    BOOLEAN                         IsDeviceHighSpeed;
    WDFUSBINTERFACE                 UsbInterface;
    UCHAR                           NumberConfiguredPipes;
    ULONG                           MaximumTransferSize;    
    WDFQUEUE                        IsochReadQ;
    WDFQUEUE                        IsochWriteQ;

	enum hw_states 					hw_status;			// HW state
	unsigned char 					hw_regs[CONF_REGS];	// HW status registers

	KEVENT  						Bulk_Event;
	LONG							bulk_count;

	//USB_PIPE_INFO
	WDFUSBPIPE	 					Pipe_Firmware_Write	;		
	WDFUSBPIPE						Pipe_Firmware_Read	;	
	WDFUSBPIPE						Pipe_Data_Write;
	WDFUSBPIPE						Pipe_Conf_Write;
	WDFUSBPIPE						Pipe_Data_Read;
	WDFUSBPIPE						Pipe_Conf_Read;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT,
                                                          GetDeviceContext)
//
// This context is associated with every open handle.
//
typedef struct _FILE_CONTEXT {

    WDFUSBPIPE Pipe;

} FILE_CONTEXT, *PFILE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FILE_CONTEXT,
                                                        GetFileContext)

//
// This context is associated with every request recevied by the driver
// from the app.
//
typedef struct _REQUEST_CONTEXT {

    WDFMEMORY         UrbMemory;
    PMDL              Mdl;
    ULONG             Length;         // remaining to xfer
    ULONG             Numxfer;        // cumulate xfer
    ULONG_PTR         VirtualAddress; // va for next segment of xfer.
    WDFCOLLECTION     SubRequestCollection; // used for doing Isoch
    WDFSPINLOCK     SubRequestCollectionLock; // used to sync access to collection at DISPATCH_LEVEL
    BOOLEAN           Read; // TRUE if Read
} REQUEST_CONTEXT, * PREQUEST_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REQUEST_CONTEXT            ,
                                                GetRequestContext)

typedef struct _WORKITEM_CONTEXT {
    WDFDEVICE       Device;
    WDFUSBPIPE      Pipe;
} WORKITEM_CONTEXT, *PWORKITEM_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WORKITEM_CONTEXT,
                                                GetWorkItemContext)

//
// Required for doing ISOCH transfer. This context is associated with every
// subrequest created by the driver to do ISOCH transfer.
//
typedef struct _SUB_REQUEST_CONTEXT 
{

    WDFREQUEST  UserRequest;
    PURB        SubUrb;
    PMDL        SubMdl;
    LIST_ENTRY  ListEntry; // used in CancelRoutine

} SUB_REQUEST_CONTEXT, *PSUB_REQUEST_CONTEXT ;


WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(SUB_REQUEST_CONTEXT, GetSubRequestContext)

extern ULONG DebugLevel;

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD MIPI_DeviceAdd;

EVT_WDF_DEVICE_PREPARE_HARDWARE MIPI_DevicePrepareHardware;

EVT_WDF_IO_QUEUE_IO_READ MIPI_Read;
EVT_WDF_IO_QUEUE_IO_WRITE MIPI_Write;

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL MIPI_IoDeviceControl;

EVT_WDF_REQUEST_CANCEL UsbSamp_EvtRequestCancel;
EVT_WDF_REQUEST_COMPLETION_ROUTINE MIPI_ReadComletionRoutine;
EVT_WDF_REQUEST_COMPLETION_ROUTINE MIPI_WriteComletionRoutine;



WDFUSBPIPE
GetPipeFromName(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PUNICODE_STRING FileName
    );


NTSTATUS
ResetPipe(
    IN WDFUSBPIPE             Pipe
    );

NTSTATUS
ResetDevice(
    IN WDFDEVICE Device
    );

VOID
CancelSelectSuspend(
    IN PDEVICE_CONTEXT DeviceContext
    );


NTSTATUS
ReadAndSelectDescriptors(
    IN WDFDEVICE Device
    );

NTSTATUS
ConfigureDevice(
    IN WDFDEVICE Device
    );

NTSTATUS
SelectInterfaces(
    IN WDFDEVICE Device
    );

NTSTATUS
SetPowerPolicy(
        __in WDFDEVICE Device
    );




VOID
DbgPrintRWContext(
    PREQUEST_CONTEXT                 rwContext
    );

NTSTATUS
AbortPipes(
    IN WDFDEVICE Device
    );


#endif

