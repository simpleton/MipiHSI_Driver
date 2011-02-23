/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Queue.c

Abstract:

    This file contains dispatch routines for create,
    close, device-control, read & write.

Environment:

    Kernel mode

--*/

#include "private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MIPI_IoDeviceControl)
#pragma alloc_text(PAGE, MIPI_Read)
#pragma alloc_text(PAGE, MIPI_Write)
#pragma alloc_text(PAGE, GetPipeFromName)
#pragma alloc_text(PAGE, ResetPipe)
#pragma alloc_text(PAGE, ResetDevice)
#endif

VOID
MIPI_IoDeviceControl(
    IN WDFQUEUE   Queue,
    IN WDFREQUEST Request,
    IN size_t     OutputBufferLength,
    IN size_t     InputBufferLength,
    IN ULONG      IoControlCode
    )
/*++

Routine Description:

    This event is called when the framework receives IRP_MJ_DEVICE_CONTROL
    requests from the system.

Arguments:

    Queue - Handle to the framework queue object that is associated
            with the I/O request.
    Request - Handle to a framework request object.

    OutputBufferLength - length of the request's output buffer,
                        if an output buffer is available.
    InputBufferLength - length of the request's input buffer,
                        if an input buffer is available.

    IoControlCode - the driver-defined or system-defined I/O control code
                    (IOCTL) that is associated with the request.
Return Value:

    VOID

--*/
{
    WDFDEVICE          device;
    PVOID              ioBuffer;
    size_t             bufLength;
    NTSTATUS           status;
    PDEVICE_CONTEXT    hw_dev;
    PFILE_CONTEXT      pFileContext;
    ULONG              length = 0;
	WDFMEMORY          InputMem;
	WDFMEMORY          OutputMem;
	REG_STRUCT 		   m_reg[CONF_REGS] ; //the totally number of the register is 0x20
	unsigned int 	   i;
	unsigned char 	   outputBuffer[CONF_REGS];
	
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    UsbSamp_DbgPrint(3, ("Entered MIPI_IoDeviceControl\n"));

    PAGED_CODE();

    //
    // initialize variables
    //
    device = WdfIoQueueGetDevice(Queue);
    hw_dev = GetDeviceContext(device);

	if (InputBufferLength > 0){
		status = WdfRequestRetrieveInputMemory(Request, &InputMem);	
		if (!NT_SUCCESS(status)){
			UsbSamp_DbgPrint(3, ("WdfRequestRetrieveInputMemory ERROR %x\n",status));
		}
	}
	
	if (OutputBufferLength > 0){	
		status = WdfRequestRetrieveOutputMemory(Request, &OutputMem);
		if (!NT_SUCCESS(status)){
			UsbSamp_DbgPrint(3, ("WdfRequestRetrieveOutputMemory ERROR %x\n",status));
		}
	}

	
    switch(IoControlCode) {

    case IOCTL_MIPI_RESET:
		UsbSamp_DbgPrint(3, ("IOCTL_MIPI_RESET\n"));
		status = init_to_defaults(device,Request);
		OutputBufferLength = 0;
		break;


	case IOCTL_MIPI_READ_REG:
		UsbSamp_DbgPrint(3, ("IOCTL_MIPI_READ_REG length=\n",OutputBufferLength));
		
		status = WdfMemoryCopyToBuffer(
									   InputMem,
									   0,
									   (unsigned char*)m_reg,
									   InputBufferLength
									   );
		
		status = get_hw_regs(device, 0, CONF_REGS);
		if (!NT_SUCCESS(status)){		
			UsbSamp_DbgPrint(3, ("IOCTL_MIPI_READ_REG ERROR\n"));
			status = STATUS_SEVERITY_ERROR;
			goto exit ;
		}

		for (i=0; i<InputBufferLength/sizeof(REG_STRUCT); ++i){
			outputBuffer[i] = get_regbits(hw_dev->hw_regs,m_reg[i].loc, \
										  m_reg[i].pos,m_reg[i].width);
		}
		OutputBufferLength = i;
		status = WdfMemoryCopyFromBuffer(
							   OutputMem,
							   0,
							   outputBuffer,
							   OutputBufferLength
							   );
		break;

		
	case IOCTL_MIPI_WRITE_REG:
		UsbSamp_DbgPrint(3, ("IOCTL_MIPI_WRITE_REG length=%x\n",InputBufferLength));

		status = WdfMemoryCopyToBuffer(
									   InputMem,
									   0,
									   (unsigned char*)m_reg,
									   InputBufferLength
									   );
		if (!NT_SUCCESS(status)){
			UsbSamp_DbgPrint(3, ("WdfMemoryCopyToBuffer ERROR\n"));
			status = STATUS_SEVERITY_ERROR;
			goto exit ;
		}
		for(i=0; i<InputBufferLength/sizeof(REG_STRUCT); ++i){
			set_regbits(hw_dev->hw_regs, m_reg[i].loc, m_reg[i].pos, m_reg[i].width, \
						m_reg[i].value);
			UsbSamp_DbgPrint(3, ("WRITE_REG %x %x %x %x\n",m_reg[i].loc,m_reg[i].pos, \
								m_reg[i].width,m_reg[i].value));
		}
		status = set_hw_regs(device, 0, CONF_REGS,Request);
		OutputBufferLength = 0;
		break;

		
	case IOCTL_MIPI_DUMP_REG:
		UsbSamp_DbgPrint(3, ("IOCTL_MIPI_DUMP_REG\n"));
		
		status = get_hw_regs(device, 0, CONF_REGS);
		if (!NT_SUCCESS(status)){
			UsbSamp_DbgPrint(3, ("WdfMemoryCopyToBuffer ERROR\n"));
			status = STATUS_SEVERITY_ERROR;
			goto exit ;
		}

		for (i=0; i<CONF_REGS; ++i)
			UsbSamp_DbgPrint(3, ("reg[%x]=%x \n",i,hw_dev->hw_regs[i]));
		OutputBufferLength = (OutputBufferLength > CONF_REGS ? CONF_REGS : OutputBufferLength);
		status = WdfMemoryCopyFromBuffer(
							   OutputMem,
							   0,
							   (unsigned char*)hw_dev->hw_regs,
							   OutputBufferLength
							   );
		break;


    default :
		UsbSamp_DbgPrint(3, ("Can not recognize the iocontrol code %x\n",IoControlCode));
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }
exit:
    WdfRequestCompleteWithInformation(Request, status, OutputBufferLength);

    UsbSamp_DbgPrint(3, ("Exit MIPI_IoDeviceControl\n"));

    return;
}



VOID
MIPI_Read(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN size_t           Length
    )
/*++

Routine Description:

    Called by the framework when it receives Read requests.

Arguments:

    Queue - Default queue handle
    Request - Handle to the read/write request
    Lenght - Length of the data buffer associated with the request.
                 The default property of the queue is to not dispatch
                 zero lenght read & write requests to the driver and
                 complete is with status success. So we will never get
                 a zero length request.

Return Value:


--*/
{
    WDFUSBPIPE                  pipe;
    NTSTATUS                    status;
    WDFMEMORY                   reqMemory;
    PDEVICE_CONTEXT             pDeviceContext;
    BOOLEAN                     ret;
	WDF_REQUEST_SEND_OPTIONS 	syncReqOptions;
    
    UNREFERENCED_PARAMETER(Length);
	UsbSamp_DbgPrint(3, ("Start MIPI_Read\n"));
    pDeviceContext = GetDeviceContext(WdfIoQueueGetDevice(Queue));
    
    pipe = pDeviceContext->Pipe_Data_Read;
    
    status = WdfRequestRetrieveOutputMemory(Request, &reqMemory);
    if(!NT_SUCCESS(status)){
        goto Exit;
    }
   
    status = WdfUsbTargetPipeFormatRequestForRead(pipe,
                                        Request,
                                        reqMemory,
                                        NULL // Offsets
                                        ); 
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    WdfRequestSetCompletionRoutine(Request,
                            MIPI_ReadComletionRoutine,
                            pipe);
	
	WDF_REQUEST_SEND_OPTIONS_INIT(
								  &syncReqOptions,
								  0
								  );
	WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(
										 &syncReqOptions,
										 -100 // 10000* 100nanosecond 
										 );


    ret = WdfRequestSend(Request, 
                    WdfUsbTargetPipeGetIoTarget(pipe), 
                    &syncReqOptions);
    
    if (ret == FALSE) {
        status = WdfRequestGetStatus(Request);
        goto Exit;
    } else {
    	UsbSamp_DbgPrint(3, ("End MIPI_Read\n"));
        return;
    }
   
Exit:
    WdfRequestCompleteWithInformation(Request, status, 0);
	UsbSamp_DbgPrint(3, ("End MIPI_Read\n"));
    return;
}


VOID
MIPI_ReadComletionRoutine(
    IN WDFREQUEST                  Request,
    IN WDFIOTARGET                 Target,
    PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
    IN WDFCONTEXT                  Context
    )
{    
    NTSTATUS    status;
    size_t      bytesRead = 0;
    PWDF_USB_REQUEST_COMPLETION_PARAMS usbCompletionParams;

    UNREFERENCED_PARAMETER(Target);
    UNREFERENCED_PARAMETER(Context);
	UsbSamp_DbgPrint(3, ("Start MIPI_ReadComletionRoutine\n"));
    status = CompletionParams->IoStatus.Status;
    
    usbCompletionParams = CompletionParams->Parameters.Usb.Completion;
    
    bytesRead =  usbCompletionParams->Parameters.PipeRead.Length;
    
    if (NT_SUCCESS(status)){
        KdPrint(("Number of bytes read: %I64d\n", (INT64)bytesRead));  
    } else {
        KdPrint(("Read failed - request status 0x%x UsbdStatus 0x%x\n",
                status, usbCompletionParams->UsbdStatus));

    }

    WdfRequestCompleteWithInformation(Request, status, bytesRead);
	UsbSamp_DbgPrint(3, ("End MIPI_ReadComletionRoutine\n"));

    return;
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

VOID
MIPI_Write(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN size_t           Length
    )
/*++

Routine Description:

    Called by the framework when it receives Write requests.

Arguments:

    Queue - Default queue handle
    Request - Handle to the read/write request
    Lenght - Length of the data buffer associated with the request.
                 The default property of the queue is to not dispatch
                 zero lenght read & write requests to the driver and
                 complete is with status success. So we will never get
                 a zero length request.

Return Value:


--*/
{
    NTSTATUS                    status;
    WDFUSBPIPE                  pipe;
    WDFMEMORY                   reqMemory;
    PDEVICE_CONTEXT             pDeviceContext;
    BOOLEAN                     ret;
    int 						Buf_Size=0;
	
    UNREFERENCED_PARAMETER(Length);
	UsbSamp_DbgPrint(3, ("Start MIPI_Write\n"));

    pDeviceContext = GetDeviceContext(WdfIoQueueGetDevice(Queue));
    
    pipe = pDeviceContext->Pipe_Data_Write;

    status = WdfRequestRetrieveInputMemory(Request, &reqMemory);
    if(!NT_SUCCESS(status)){
        goto Exit;
    }

    status = WdfUsbTargetPipeFormatRequestForWrite(pipe,
                                              Request,
                                              reqMemory,
                                              NULL); // Offset
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

	WdfMemoryGetBuffer(reqMemory, &Buf_Size);
	
    WdfRequestSetCompletionRoutine(
                            Request,
                            MIPI_WriteComletionRoutine,
                            pipe);
    ret = WdfRequestSend(Request, 
                    WdfUsbTargetPipeGetIoTarget(pipe), 
                    WDF_NO_SEND_OPTIONS);
    if (ret == FALSE) {
        status = WdfRequestGetStatus(Request);
        goto Exit;
    } else {
    	UsbSamp_DbgPrint(3, ("End MIPI_Write\n"));
        return;
    }

Exit:    
    WdfRequestCompleteWithInformation(Request, status, 0);
	UsbSamp_DbgPrint(3, ("End MIPI_Write\n"));

    return;
}



VOID
MIPI_WriteComletionRoutine(
    IN WDFREQUEST                  Request,
    IN WDFIOTARGET                 Target,
    PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
    IN WDFCONTEXT                  Context
    )
{
    NTSTATUS    status;
    size_t      bytesWritten = 0;
    PWDF_USB_REQUEST_COMPLETION_PARAMS usbCompletionParams;

    UNREFERENCED_PARAMETER(Target);
    UNREFERENCED_PARAMETER(Context);
	UsbSamp_DbgPrint(3, ("Start MIPI_WriteComletionRoutine\n"));

    status = CompletionParams->IoStatus.Status;

    usbCompletionParams = CompletionParams->Parameters.Usb.Completion;
    
    bytesWritten =  usbCompletionParams->Parameters.PipeWrite.Length;
    
    if (NT_SUCCESS(status)){
        KdPrint(("Number of bytes written: %I64d\n", (INT64)bytesWritten));        
    } else {
        KdPrint(("Write failed: request Status 0x%x UsbdStatus 0x%x\n", 
                status, usbCompletionParams->UsbdStatus));
    }

    WdfRequestCompleteWithInformation(Request, status, bytesWritten);
	UsbSamp_DbgPrint(3, ("End MIPI_WriteComletionRoutine\n"));
    return;
}



NTSTATUS
ResetPipe(
    IN WDFUSBPIPE Pipe
    )
/*++

Routine Description:

    This routine resets the pipe.

Arguments:

    Pipe - framework pipe handle

Return Value:

    NT status value

--*/
{
    NTSTATUS status;

    PAGED_CODE();

    //
    //  This routine synchronously submits a URB_FUNCTION_RESET_PIPE
    // request down the stack.
    //
    status = WdfUsbTargetPipeResetSynchronously(Pipe,
                                WDF_NO_HANDLE, // WDFREQUEST
                                NULL  // PWDF_REQUEST_SEND_OPTIONS
                                );

    if (NT_SUCCESS(status)) {
        UsbSamp_DbgPrint(3, ("ResetPipe - success\n"));
        status = STATUS_SUCCESS;
    }
    else {
        UsbSamp_DbgPrint(1, ("ResetPipe - failed\n"));
    }

    return status;
}

VOID
StopAllPipes(
    IN PDEVICE_CONTEXT DeviceContext
    )
{
    UCHAR count,i;

    count = DeviceContext->NumberConfiguredPipes;
    for (i = 0; i < count; i++) {
        WDFUSBPIPE pipe;
        pipe = WdfUsbInterfaceGetConfiguredPipe(DeviceContext->UsbInterface,
                                                i, //PipeIndex,
                                                NULL
                                                );
        WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(pipe),
                        WdfIoTargetCancelSentIo);
    }
}


VOID
StartAllPipes(
    IN PDEVICE_CONTEXT DeviceContext
    )
{
    NTSTATUS status;
    UCHAR count,i;

    count = DeviceContext->NumberConfiguredPipes;
    for (i = 0; i < count; i++) {
        WDFUSBPIPE pipe;
        pipe = WdfUsbInterfaceGetConfiguredPipe(DeviceContext->UsbInterface,
                                                i, //PipeIndex,
                                                NULL
                                                );
        status = WdfIoTargetStart(WdfUsbTargetPipeGetIoTarget(pipe));
        if (!NT_SUCCESS(status)) {
            UsbSamp_DbgPrint(1, ("StartAllPipes - failed pipe #%d\n", i));
        }
    }
}

NTSTATUS
ResetDevice(
    IN WDFDEVICE Device
    )
/*++

Routine Description:

    This routine calls WdfUsbTargetDeviceResetPortSynchronously to reset the device if it's still
    connected.

Arguments:

    Device - Handle to a framework device

Return Value:

    NT status value

--*/
{
    PDEVICE_CONTEXT pDeviceContext;
    NTSTATUS status;

    UsbSamp_DbgPrint(3, ("ResetDevice - begins\n"));

    PAGED_CODE();

    pDeviceContext = GetDeviceContext(Device);
    
    //
    // A reset-device
    // request will be stuck in the USB until the pending transactions
    // have been canceled. Similarly, if there are pending tranasfers on the BULK
    // IN/OUT pipe cancel them.
    // To work around this issue, the driver should stop the continuous reader
    // (by calling WdfIoTargetStop) before resetting the device, and restart the
    // continuous reader (by calling WdfIoTargetStart) after the request completes.
    //
    StopAllPipes(pDeviceContext);
    
    //
    // It may not be necessary to check whether device is connected before
    // resetting the port.
    //
    status = WdfUsbTargetDeviceIsConnectedSynchronous(pDeviceContext->WdfUsbTargetDevice);

    if(NT_SUCCESS(status)) {
        status = WdfUsbTargetDeviceResetPortSynchronously(pDeviceContext->WdfUsbTargetDevice);
    }

    StartAllPipes(pDeviceContext);
    
    UsbSamp_DbgPrint(3, ("ResetDevice - ends\n"));

    return status;
}

