/*
 * HS6200测试的main函数文件
 *
 * 说明：对STARTUP.A51文件进行修改，增加line 112 and 134, 其作用是关闭看门狗，防止xdata太大，
 *       而在STARTUP.A51文件进行初始化xdata时时间太长，导致的看门狗复位。（谨记） 
 *       
 *       上电顺序：先HS6200/nRF24L01,然后MCU，或者同时上电。 因为在LED Off后，系统会读外设，根据外设
 *                 确定所接是HS6200还是nRF24L01,一个MCU上挂接几个devive. 
 *
 * 配置：整个工程的配置在HS6200_Debug.h文件	
 */


#include "hal.h"
#include "ch.h"
#include "nRF24L01_X.h"
#include "HS6200_test_sys.h"   
#include "HS6200_test.h"

#include "C8051F_USB.h"

#include "HS6200Test_Application_Protocol.h" 
#include "HS6200_Debug.h"
#include "HS6200_Analog_Test.h"
#include "stdlib.h"
#include "usbsubdev_hs6200.h"


U8 g_interrupt_flag=INTERRUPT_NONE;

//static void HS6200_interrupt_disable(void)
//{
//  HS_GPIO_Type *pGpio1 = IOPORT1;
//  pGpio1->INTENCLR |= BIT5;  
//}
//
//static void HS6200_interrupt_able(void)
//{
//  HS_GPIO_Type *pGpio1 = IOPORT1;
//  pGpio1->INTENSET |= BIT5;   
//}
/*---------------------------------------------------Interrupt Service Routine-------------------------------------------------*/

/*****************************************************
Function: ISR_int0() interrupt 0;
 
Description:
if RX_DR=1 or TX_DS or MAX_RT=1,enter this subprogram;
if RX_DR=1,read the payload from RX_FIFO and set g_flag;
**************************************************/
void ISR_int0(void) //interrupt 0          // nRF24L01_0 中断
{
  HS_GPIO_Type *pGpio1 = IOPORT1;
  g_interrupt_flag = INTERRUPT_HS6200;
//  HS6200_interrupt_disable();
  pGpio1->INTSTATUS = pGpio1->INTSTATUS;
}

void HS6200_interrupt_process(void) 
{
	LED1_ON;
    g_interrupt_flag = INTERRUPT_NONE;
	HS6200_Bank0_Activate(DEV_1);
	HS6200_DEV1_Int();
    LED1_OFF;
}


/**************************************************/

/**************************************************
Function: ISR_int1() interrupt 2;
 
Description:
if RX_DR=1 or TX_DS or MAX_RT=1,enter this subprogram;
if RX_DR=1,read the payload from RX_FIFO and set g_flag;
**************************************************/
void ISR_int1(void) 
{
  g_interrupt_flag = INTERRUPT_MAC6200;
}

void MAC6200_interrupt_process(void) 
{
	LED2_ON;
    MAC6200_CE_Low();
    g_interrupt_flag = INTERRUPT_NONE;
	HS6200_Bank0_Activate(DEV_1);
	HS6200_DEV1_Int();
    MAC6200_CE_High(); 
    LED2_OFF;
}

/**************************************************/
// ISR for USB_API
void usb_recevice_process(void)//INTERRUPT(USB_API_TEST_ISR, INTERRUPT_USBXpress)
{
  U8 Temp_Buf[MAX_OUT_PACKET_LENGTH];
  U8 Temp_Buf_Length;
  
  if( (USB_Host_Out_Flag==USB_HOST_OUT_FLAG_C8051F_DISCARD) || (USB_Host_Out_Flag==USB_HOST_OUT_FLAG_C8051F_USED) )  //数据已经使用
  {
    USB_Host_Out_Packet_Length=Block_Read(USB_Host_Out_Packet, MAX_OUT_PACKET_LENGTH);  //read USB packet data
    USB_Host_Out_Flag=USB_HOST_OUT_FLAG_C8051F_RECEIVED;  //数据接收到
    
    //判断此时发送的命令是否是停止Test Pattern
    //命令：
    //0A xx	08 02 -->AT_CTT_TESTPAT_STOP
    if( (0x0A==USB_Host_Out_Packet[0]) && (0x02==USB_Host_Out_Packet[2]) && (0x08==USB_Host_Out_Packet[3]) &&  (0x02==USB_Host_Out_Packet[4]) )
    {
      if(DEV_0==USB_Host_Out_Packet[1])        //DEV_0
      {
        Test_Pattern_Psdo_Rdm_Num_Flag[DEV_0]=TEST_PAT_PSDO_RDM_NUM_FLG_STOP;	  //stop pseudo random number test
      }
    else if(DEV_1==USB_Host_Out_Packet[1])	 //DEV_1
      {
        Test_Pattern_Psdo_Rdm_Num_Flag[DEV_1]=TEST_PAT_PSDO_RDM_NUM_FLG_STOP;	  //stop pseudo random number test
      }
    }
    else if( (0x0E==USB_Host_Out_Packet[0]) && (0x01==USB_Host_Out_Packet[2]) && (USB_Host_Out_Packet[3]==DEBUG_CE_HIGH_LOW_STOP))  //debug stop
      {
        Debug_ce_High_Low_Stop_Flag=0x01;
      } 
    }
  else 
  {
    Temp_Buf_Length=Block_Read(Temp_Buf,MAX_OUT_PACKET_LENGTH);
    USB_Discard_Dev_Num=Temp_Buf[2];
    USB_Host_Out_Flag=USB_HOST_OUT_FLAG_C8051F_DISCARD;  //数据丢弃
  }
}

void usb_Transmitted_process(void)
{
  USB_Host_In_Flag=USB_HOST_IN_FLAG_COMPLETED;  
}

/*---------------------------------------------------------------------End Of File---------------------------------------------------------------------*/


