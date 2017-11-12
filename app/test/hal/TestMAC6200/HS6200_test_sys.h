
#ifndef HS6200_TEST_SYS_H
#define HS6200_TEST_SYS_H

#include "HS6200_types.h"

#define DEV_NONE       0x00           /*表示未接dev*/	//判断有无外接dev,是通过先对寄存器进行写操作然后在进行读操作，如果写的数据和读的数据一致说明外接dev,否则未接.
#define HS6200_DEV     0x01		      /*外接HS6200*/
#define MAC6200_DEV    0x01		      
#define nRF24L01_DEV   0x02		      /*外接nRF24L01*/
extern U8 Dev_Flag[2];

extern void Dev_Scan(void);

#define LED1_PIN          8
#define LED2_PIN          9
#define HS6200_CE         12
#define HS6200_IRQ        11
#define HS6200_CSN        5
#define HS6200_SCK        8
#define HS6200_MOSI       7
#define HS6200_MISO       6

#define HS6200_IRQ_LED    4

#define LED1_PORT         IOPORT1   //PB8 
#define LED2_PORT         IOPORT1   //PB9
#define HS6200_CE_PORT    IOPORT0   //PA12
#define HS6200_IRQ_PORT   IOPORT0   //PA11
#define HS6200_CSN_PORT   IOPORT0   //PA5
#define HS6200_SCK_PORT   IOPORT0   //PA8
#define HS6200_MOSI_PORT  IOPORT0   //PA7
#define HS6200_MISO_PORT  IOPORT0   //PA6

#define LED1_ON        {palClearPad(LED1_PORT, LED1_PIN);}
#define LED2_ON        {palClearPad(LED2_PORT, LED2_PIN);}
#define LED1_OFF       {palSetPad(LED1_PORT,LED1_PIN);}
#define LED2_OFF       {palSetPad(LED2_PORT,LED2_PIN);}

#define LED_ALL_ON()     do{LED1_ON;LED2_ON;}while(0)
#define LED_ALL_OFF()    do{LED1_OFF;LED2_OFF;}while(0)

#define LED1_SET(a)    {(a)?LED1_OFF:LED1_ON}
#define LED2_SET(a)    {(a)?LED2_OFF:LED2_ON}

#define INTERRUPT_NONE    0
#define INTERRUPT_HS6200  1
#define INTERRUPT_MAC6200 2


extern void LED_Disp_Err(void);	 /*LED1 LED2 交替闪烁几次表示错误*/
extern void Key_Scan(void);
extern void Init_Device(void);



#endif  /*HS6200_TEST_SYS_H*/


/*------------------------------------End Of File----------------------------------------*/
