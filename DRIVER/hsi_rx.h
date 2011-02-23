#ifndef __HSI_RX_H__
#define __HSI_RX_H__

#define HSI_INVERT			(1<<4)
#define HSI_UNOPEN_ONLY			(1<<5)
#define HSI_ALL_CH			(1<<6)



NTSTATUS init_rx(IN WDFDEVICE device);

#endif // __HSI_RX_H__


