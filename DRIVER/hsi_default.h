#ifndef _HSI_DEFAULT_H
#define _HSI_DEFAULT_H

//----------------------------------------------------------------------------
//  Defaults.
//  Modify this file to change driver defaults 
//----------------------------------------------------------------------------

// Device defaults

#define HSI_DEV_COUNT           	2		// Maximum number of attached dongles
#define HSI_UDEV_SUPPORT           	0		// UDEV support
#define HSI_DEBUG		0		// Debug messages: 0 = Disabled, 1 = Enabled

// TX defaults

#define HSI_TX_URB_SIZE_FRAMES   	1024		// TX buffer size / URB [frames]
#define HSI_TX_URB_COUNT           	4		// Number of TX URBs

// RX defaults

#define HSI_RX_URB_COUNT           	8		// Number of RX URBs
#define HSI_RX_BUF_COUNT		10		// Number of RX buffers for HW
#define HSI_RX_DMA_MAP_BUF_COUNT	10		// How many RX buffers are statically DMA mapped
#define HSI_REJECT_DATA    		0		// 0 = Accept data to unopened channel

//----------------------------------------------------------------------------
// Register Default values
// 
//  Edit these values to the initial values that suit best for the application
//  These values are applied to the dongle after it is USB enumerated.
//  These are reloadable any time with a procfs or ioctl command
//----------------------------------------------------------------------------

#define		RXCPR_def		0x03	// RX Channel bit field width = 3, stream, synchronized
#define		TXCPR_def		0x03	// TX Channel bit field width = 3, stream
#define		RXDRR_def		0x01	// RX speed will be detected automatically
#define		TXDRR_def		0x01	// TX 96Mbps
#define		RXBCR_def		0x00	// Source synch front-end
#define		TXBCR_def		0x00	// 
#define		RXFTR_def		0x82	// SD8 = 8, tailing bits = 2
#define		RXWMR_def		0x04	// Wait multiplier = 4 
#define		RXTOR_def		0x50	// Timeout count value = 0x50 
#define		RXFBR_def		0x0F	// Frame count for burst 0x0F
#define		RXEFR_def		0x80	// Clear RX error flags on every read
#define		TXEFR_def		0x80	// Clear TX error flags on every read
#define		RXFOC_def		0x00	// Clear error counters
#define		RXBDC_def		0x00
#define		RXTOC_def		0x00
#define		SYSC0_def		0x01	// HSI bus is on DIO = reg
#define		IDR3_def		0x00	// ID (read only)
#define		IDR2_def		0x00
#define		IDR1_def		0x00
#define		IDR0_def		0x00
#define		RXFC3_def		0x00	// RX frame count
#define		RXFC2_def		0x00
#define		RXFC1_def		0x00
#define		RXFC0_def		0x00
#define		TXFC3_def		0x00	// TX frame count
#define		TXFC2_def		0x00
#define		TXFC1_def		0x00
#define		TXFC0_def		0x00
#define		HSISR_def		0x01	// TX_wake is on
#define		HSIBV_def		0x00	
#define		DIODE_def		0x00	// All DIO pins are inputs
#define		DIOPR_def		0x00

#endif
