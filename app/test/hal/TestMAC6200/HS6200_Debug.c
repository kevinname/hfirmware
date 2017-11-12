
/* 
 * 说明： 有关Debug需要的函数。HS6200_Debug.h中有关于配置是否输出的宏
 *
 *  Yao  08/05/2013
 *  HunterSun Electronics Co., Ltd.
 */

#include "C8051F_USB.h"
#include "HS6200_Debug.h"
#include "HS6200_test_sys.h"
#include "nRF24L01_common.h"
#include "usbsubdev_hs6200.h"

//软件更新日期
U8 SoftWare_Date[6]={0x20,0x14,0x05,0x30,0x41,0x0};		//20xx/xx/xx/V4.1.00

//软件版本号
U8 SoftWare_Ver=0x40;  //Vx.x


U8 Debug_ce_High_Low_Stop_Flag=0x01;

void LED_Out_Err_Info(U8 Err_Info)  //LED输出错误信息
{
//	DISP_OUT_ERR=Err_Info;	
	LED_Disp_Err();  //display error by led
}
void USB_Out_Err_Info(U8 DevNum,U8 Err_Info)  //USB输出错误信息---->其实和ACK包格式一致
{
	if( (USB_Host_In_Flag==USB_HOST_IN_FLAG_COMPLETED) )  
	{
		USB_Host_In_Packet[0]=PID_ACK;
		USB_Host_In_Packet[1]=DevNum;
		USB_Host_In_Packet[2]=0x02;  //length
		USB_Host_In_Packet[3]=TYPE_ERR;
		USB_Host_In_Packet[4]=Err_Info;
		Block_Write(USB_Host_In_Packet,5);
	
		USB_Host_In_Flag=USB_HOST_IN_FLAG_NOT_COMPLETED;
	}	
}

void USB_ACK_Diagnose_Info(U8 DevNum)
{
    U8 pTemp[10];
    pTemp[0]=SDS_DIAGNOSTIC;
    pTemp[1]=nRF24L01_X_nop(DevNum);
    pTemp[2]=nRF24L01_X_read_reg(DevNum,nRF24L01_FIFO_STATUS);
    pTemp[3]=nRF24L01_X_read_reg(DevNum,nRF24L01_OBSERVE_TX);
    pTemp[4]=nRF24L01_X_read_reg(DevNum,nRF24L01_RPD);
    if( pTemp[1] & nRF24L01_RX_DR )          //接收中断 
    	pTemp[5]=nRF24L01_X_read_rx_payload_width(DevNum);
    else pTemp[5]=0x00;   
    USB_ACK_2_Host(DevNum,TYPE_SDS,pTemp,6);
}



/*---------------------------------------End Of File---------------------------------------*/


