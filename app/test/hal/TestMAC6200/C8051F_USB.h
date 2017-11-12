

#ifndef C8051F_USB_H
#define C8051F_USB_H

#include "HS6200_types.h"
#include "USB_API.h"
#include "HS6200Test_Application_Protocol.h"

#define INTERRUPT_USBXpress 17


extern U8 Tx_By_CE_High;
extern U8 USB_Discard_Dev_Num;
extern U8 Nop_After_W_Tx_Payload;

#define USB_HOST_OUT_FLAG_C8051F_USED         0x00
#define USB_HOST_OUT_FLAG_C8051F_RECEIVED     0x01
#define USB_HOST_OUT_FLAG_C8051F_DISCARD      0x02

#define USB_HOST_IN_FLAG_NOT_COMPLETED        0x03
#define USB_HOST_IN_FLAG_COMPLETED            0x04 

extern U8 USB_Host_In_Flag;
extern U8 USB_Host_Out_Flag;

extern U8 USB_Host_Out_Packet_Length;
extern U16 USB_Host_In_Packet_Length;


extern U8 USB_Host_Out_Packet[MAX_OUT_PACKET_LENGTH];      // Last packet received from host
extern U8 USB_Host_In_Packet[MAX_IN_PACKET_LENGTH];        // Next packet to sent to host

extern U8 Ack_Pipe_Num;

extern void USB_Patch(void); 

extern void USB_ACK_2_Host(U8 DevNum, U8 ACK_Type, U8 *ACK_Buf,U8 ACK_Buf_Length );
extern void USB_NAK_2_Host(U8 DevNum);
#define DATA_NO_PIPE      0xFF 
extern void USB_DATA_2_Host(U8 DevNum,U8 PipeNum, U8 *Data_Buf,U8 Data_Buf_Length );
extern void USB_Protocol_Resolut(void);

#endif  /*C8051F_USB_H*/


/*--------------------------------End Of File---------------------------------*/
