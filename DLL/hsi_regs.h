
#define CONF_REGS		0x20	// Register space size

//---------------- Register Definitions ---------------------------------------

#define		RXCPR		0x00
#define		TXCPR		0x01
#define		RXDRR		0x02
#define		TXDRR		0x03
#define		RXBCR		0x04
#define		TXBCR		0x05
#define		RXFTR		0x06
#define		RXWMR		0x07
#define		RXTOR		0x08
#define		RXFBR		0x09
#define		RXEFR		0x0A
#define		TXEFR		0x0B
#define		RXFOC		0x0C
#define		RXBDC		0x0D
#define		RXTOC		0x0E
#define		SYSC0		0x0F
#define		IDR3		0x10
#define		IDR2		0x11
#define		IDR1		0x12
#define		IDR0		0x13
#define		RXFC3		0x14
#define		RXFC2		0x15
#define		RXFC1		0x16
#define		RXFC0		0x17
#define		TXFC3		0x18
#define		TXFC2		0x19
#define		TXFC1		0x1A
#define		TXFC0		0x1B
#define		HSISR		0x1C
#define		HSIBV		0x1D
#define		DIODE		0x1E
#define		DIOPR		0x1F

//---------------- Bit Definitions --------------------------------------------

// RXCPR
#define		RXCHW		RXCPR,0,3
#define		RXFLM		RXCPR,4,1
#define		RXTRM		RXCPR,5,1
#define		RXRTF		RXCPR,7,1

// TXCPR
#define		TXCHW		TXCPR,0,3
#define		TXFLM		TXCPR,4,1
#define		TXTRM		TXCPR,5,1

// RXBCR
#define		RXFEM		RXBCR,0,1
#define		RXDRE		RXBCR,1,1
#define		RXDTE		RXBCR,2,1
#define		RXDWC		RXBCR,3,1
#define		RXFBF		RXBCR,5,1
#define		RXFFF		RXBCR,6,1
#define		RXCLR		RXBCR,7,1

// TXBCR
#define		TXIBR		TXBCR,0,1
#define		TXSCN		TXBCR,1,1
#define		TXFBF		TXBCR,5,1
#define		TXFFE		TXBCR,6,1
#define		TXCLR		TXBCR,7,1

// RXFTR
#define		RXTBR		RXFTR,0,3
#define		RXSD8		RXFTR,4,4

// RXWMR
#define		RXWMP		RXWMR,0,3

// RXEFR
#define		RXFOE		RXEFR,0,1
#define		RXOWE		RXEFR,1,1
#define		RXTOE		RXEFR,2,1
#define		RXEZF		RXEFR,5,1
#define		RXBDF		RXEFR,6,1
#define		RXCOR		RXEFR,7,1

// TXEFR
#define		TXHSE		TXEFR,0,1
#define		TXIHE		TXEFR,1,1
#define		TXCOE		TXEFR,2,1
#define		TXRLL		TXEFR,3,1
#define		TXCOR		RXEFR,7,1

// SYSC0

#define		HSION		SYSC0,0,1
#define		USBHS		SYSC0,1,1
#define		DIOTE		SYSC0,4,1
#define		DIOTS		SYSC0,5,1
#define		HSIVE		SYSC0,7,1

// IDR3
#define		HW_DEV		IDR3,4,4
#define		HW_ID		IDR3,0,4

// HSISR
#define		TXWAK		HSISR,0,1
#define		TXRDY		HSISR,1,1
#define		TXFLG		HSISR,2,1
#define		TXDAT		HSISR,3,1
#define		RXWAK		HSISR,4,1
#define		RXRDY		HSISR,5,1
#define		RXFLG		HSISR,6,1
#define		RXDAT		HSISR,7,1




