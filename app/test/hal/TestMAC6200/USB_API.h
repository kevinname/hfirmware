//*****************************************************************************
// (c) 2011 Silicon Laboratories, Inc.
// http://www.silabs.com/mcu
//-----------------------------------------------------------------------------
// Filename  : USB_API.h
// Project   : USBXpress Firmware Library
// Target    : C8051F320/1/6/7, 'F340/1/2/3/4/5/6/7/8/9/A/B/C/D
//             'F380/1/2/3/4/5/6/7, 'T320/1/2/3/6/7, 'T620/1/2/3
// Tool Chain: Keil / SDCC / Raisonance
// Version   : 3.6
//-----------------------------------------------------------------------------
// Revision History:
// =Rev3.6 (ES-02JUN2011)
//  -Added 'large' to Block_Write() for Keil and Raisonance
// =Rev3.5 (ES-11JAN2011)
//  -No changes; Incremented version number
// =Rev3.4 (ES-16FEB2010)
//  -Incremented version number
// =Rev3.3 (PKC-17NOV2009)
//  -Incremented version number
// =Rev3.2 (ES-27MAY2009)
//  -Added support for SDCC
// =Rev3.1 (TP-13NOV2007)
//  -Changed formatting to fit the firmware guidelines
// =Rev3.0 (PKC-20APR2006)
//  -No changes; version number incremented to match project version
// =Rev2.4 (PKC-06DEC2005)
//  -Added function USB_Get_Library_Version
// =Rev2.3 (PKC)
//  -No changes; version number incremented to match project version
// =Rev2.2 (PKC-17MAR2005)
//  -Added file description header
//  -Added new function declaration - USB_Clock_Start()
//-----------------------------------------------------------------------------
// Description:
//  Header file for the USBXpress firmware library. Includes function
//  prototypes, type definitions, and function return value macro definitions.
//*****************************************************************************

#ifndef  _USB_API_H_
#define  _USB_API_H_

// UINT type definition
#ifndef _UINT_DEF_
#define _UINT_DEF_
typedef unsigned int UINT;
#endif  /* _UINT_DEF_ */

// BYTE type definition
#ifndef _BYTE_DEF_
#define _BYTE_DEF_
typedef unsigned char BYTE;
#endif   /* _BYTE_DEF_ */

// Get_Interrupt_Source() return value bit masks
// Note: More than one bit can be set at the same time.
#define USB_RESET            0x01      // USB Reset Interrupt has occurred
#define TX_COMPLETE          0x02      // Transmit Complete Interrupt has
                                       // occurred
#define RX_COMPLETE          0x04      // Receive Complete Interrupt has
                                       // occurred
#define FIFO_PURGE           0x08      // Command received (and serviced) from
                                       // the host
                                       // to purge the USB buffers
#define DEVICE_OPEN          0x10      // Device Instance Opened on host side
#define DEVICE_CLOSE         0x20      // Device Instance Closed on host side
#define DEV_CONFIGURED       0x40      // Device has entered configured state
#define DEV_SUSPEND          0x80      // USB suspend signaling present on bus

#if defined (__RC51__) || defined (__C51__)

// Function prototypes
void USB_Clock_Start(void) large;
void USB_Init(UINT,UINT,BYTE*,BYTE*,BYTE*,BYTE,BYTE,UINT) large;
UINT Block_Write(BYTE*, UINT) large;
BYTE Block_Read(BYTE*, BYTE) large;
BYTE Get_Interrupt_Source(void) large;
void USB_Int_Enable(void) large;
void USB_Int_Disable(void) large;
void USB_Disable(void) large;
void USB_Suspend(void) large;
UINT USB_Get_Library_Version(void) large;

INTERRUPT_PROTO(USBXcore_USB_ISR, INTERRUPT_USB0);

#elif defined SDCC

// Function prototypes
void USB_Clock_Start(void);
void USB_Init(UINT,UINT,BYTE*,BYTE*,BYTE*,BYTE,BYTE,UINT);
UINT Block_Write(BYTE*, UINT);
BYTE Block_Read(BYTE*, BYTE);
BYTE Get_Interrupt_Source(void);
void USB_Int_Enable(void);
void USB_Int_Disable(void);
void USB_Disable(void);
void USB_Suspend(void);
UINT USB_Get_Library_Version(void);
void USBXcore_Jump_ISR (void);

INTERRUPT_PROTO(USBXcore_USB_ISR, INTERRUPT_USB0);

#endif

#endif  /* _USB_API _H_ */
