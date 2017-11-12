/*
 * 说明：主要包含C8051FUSB相关的操作函数，及上下文机通信的函数，通信协议解析等
 *		 协议解析请参考《HS6200Test 主机与设备之间的应用协议.one》文件。
 *
 * Yao	  20/04/2013
 * HunterSun Electronics Co., Ltd.
 */

#include "HS6200_types.h"
#include "HS6200Test_Application_Protocol.h"
#include "nRF24L01_X.h"
#include "HS6200_test_sys.h"
#include "HS6200_test.h"
#include "USB_API.h" 
#include "stdlib.h"
#include "C8051F_USB.h"
#include "HS6200_Analog_Test.h"
#include "HS6200_Debug.h"
#include "HS6200_Reg.h"
#include "usbsubdev_hs6200.h"


U8 Tx_By_CE_High=0x00;
U8 USB_Discard_Dev_Num=0x00;    //记录需要扔掉的DEV_NUM号
U8 Nop_After_W_Tx_Payload=0x00;

U8 USB_Host_Out_Flag=USB_HOST_OUT_FLAG_C8051F_USED;  //usb host out data to C8051F 
U8 USB_Host_In_Flag=USB_HOST_IN_FLAG_COMPLETED;     //to usb host data   

U8 USB_Host_Out_Packet_Length;
U16 USB_Host_In_Packet_Length;

U8 USB_Host_Out_Packet[MAX_OUT_PACKET_LENGTH];     // packet received from host
U8 USB_Host_In_Packet[MAX_IN_PACKET_LENGTH];       // packet to sent to host

U8 Address_Pipe0[5];    //用于存储pipe0的地址
U8 Ack_Pipe_Num=0x00;    //用于TX 接收到ACK Payload 后，在给上位机回复Payload时，连着应答的通道一起返回。

/*---------------------------------------------------------MCU to USB Host acknowledge------------------------------------------------------------*/ 
//ACK packet
/*
 * USB_Host_In_Packet[0]   USB_Host_In_Packet[1]     USB_Host_In_Packet[2]   USB_Host_In_Packet[3...n]  
 *   PID_ACK                     DevNum                USB_ACK_Buf length		  ACK_Buf
 *	  1Byte	                     1Byte                       1Byte				  0-nByte
 */

void USB_ACK_2_Host(U8 DevNum, U8 ACK_Type, U8 *ACK_Buf, U8 ACK_Buf_Length )
{
	U8 i;
// 	if( (USB_Host_In_Flag==USB_HOST_IN_FLAG_COMPLETED))  //USB Host Has read USB_Host_In_Packet Buffer
// 	{
		USB_Host_In_Packet[0]=PID_ACK;   //PID
		USB_Host_In_Packet[1]=DevNum;    //DEV
		if(ACK_Buf_Length==0x00)
		{
			USB_Host_In_Packet[2]=0;         //packet context length
			Block_Write(USB_Host_In_Packet,3);
		}
	    else 
		{
		        USB_Host_In_Packet[2]=ACK_Buf_Length+0x01;
				USB_Host_In_Packet[3]=ACK_Type;
				for(i=0x00;i<ACK_Buf_Length;i++)
				USB_Host_In_Packet[4+i]=ACK_Buf[i];	
				Block_Write(USB_Host_In_Packet,4+ACK_Buf_Length);
		}
		USB_Host_In_Flag=USB_HOST_IN_FLAG_NOT_COMPLETED;  
// 	}
}
/*
 *    P[0]       P[1]      P[2]     P[3]        P[4..35]  
 *  PID_DATA	devnum	 length	  pipe_num	  ACK payload
 *   0x05	  0x00/0x01	   xx	 PIPE0-PIPE5      xx
 *  1Byte	    1Byte    1Byte 	   1Byte	   1-32Bytes
 *
 */

void USB_DATA_2_Host(U8 DevNum,U8 PipeNum, U8 *Data_Buf,U8 Data_Buf_Length )
{
	U8 i;
	if( (USB_Host_In_Flag==USB_HOST_IN_FLAG_COMPLETED))  //USB Host Has read USB_Host_In_Packet Buffer
	{
		USB_Host_In_Packet[0]=PID_DATA;   //PID
		USB_Host_In_Packet[1]=DevNum;     //DEV
		if(PipeNum<=0x05)
			USB_Host_In_Packet[2]=Data_Buf_Length+1;   //length+pipe=length+1		
		else
			USB_Host_In_Packet[2]=Data_Buf_Length;
		if(Data_Buf_Length==0x00)
		{	
			Block_Write(USB_Host_In_Packet,3);
		}
	    else 
		{
		    if(PipeNum<=0x05)
			{
				USB_Host_In_Packet[3]=PipeNum; 			    //pipe num
				for(i=0x00;i<Data_Buf_Length;i++)USB_Host_In_Packet[4+i]=Data_Buf[i];	
				Block_Write(USB_Host_In_Packet,4+Data_Buf_Length); //PID+DEV+length+pipe num=4Bytes
			}
			else
			{
				for(i=0x00;i<Data_Buf_Length;i++)USB_Host_In_Packet[3+i]=Data_Buf[i];	
				Block_Write(USB_Host_In_Packet,3+Data_Buf_Length); //PID+DEV+length=4Bytes
			}	
		}
		USB_Host_In_Flag=USB_HOST_IN_FLAG_NOT_COMPLETED;  
	}
}

//busy, device is busy.
void USB_NAK_2_Host(U8 DevNum)
{
	if(USB_Host_In_Flag==USB_HOST_IN_FLAG_COMPLETED)  //USB Host Has read USB_Host_In_Packet Buffer
	{
		USB_Host_In_Packet[0]=PID_NAK;   //PID
		USB_Host_In_Packet[1]=DevNum;      //DEV 
		USB_Host_In_Packet[2]=0x00;         //packet context length
		Block_Write(USB_Host_In_Packet,3);
		USB_Host_In_Flag=USB_HOST_IN_FLAG_NOT_COMPLETED;  
	}
}

/*----------------------------------------------------------1.Communication Test------------------------------------------------------------*/
//make resolution protocol between PC and MCU(CPU) communication 
/*
 * Communication 
 *  Packet[0]     Packet[1]            Packet[2]            Packet[3]       Packet[4..n] 
 *    PID            Dev                  Bytes               pipe        payload/ACK payload
 *  PID_COMM      Dev_0/Dev_1      packet context length    pipe index        
 *   1Byte           1Byte               1+1——1+32Bytes       1Byte           32Bytes   
 *   0x01			                                                                               
 */

void Comn_Mode(U8 DevNum,U8* Cmd_Argument, U8 Cmd_Argument_Length)
{	
	U8 Addr_Temp[5];
	U8 i; 
	U8 Addr_Width;
	 
	switch(Cmd_Argument[0])
	{
		/*
		 * Cmd_Argument[0]        Cmd_Argument[1]            Cmd_Argument[2..6]	     Cmd_Argument[n]
		 *	  TXPL			  Pipe Num width/Addr width		Pipe Num/Addr Width			 Payload
		 *    0x00				0x01/0x03/0x04/0x05 		       xx 					   xx
		 *    1Byte     	           1Byte				    1-5Bytes				1-32Bytes
		 *
		 */

		case TXPL:
		    /*  Cmd_Argument[0]	 Cmd_Argument[1]    Cmd_Argument[2]	   Cmd_Argument[3..34]
			 *	   TXPL			 Pipe Num width		    Pipe Num			 payload
			 *	   0x00			     0x01		 	  PIPE0-PIPE5			    xx
			 *	  1Byte			    1Byte			      1Byte				 1-32Bytes
			 */	
			if(Cmd_Argument[1]==COMM_PIPE)	  //给定通道
			{				
// 				if(DevNum==DEV_0)	  //DEV_0
// 				{
// 					Addr_Width=nRF24L01_X_get_aw(DEV_1);
// 					nRF24L01_X_set_address_width(DEV_0,Addr_Width);	   //设置address width

// 					if(Dev_Flag[DEV_1]==nRF24L01_DEV)  //nRF24L01 dev
// 					{
// 						if(Cmd_Argument[2]>PIPE1)  //PIPE2 PIPE3 PIPE4 PIPE5
// 						{
// 							nRF24L01_X_read_pipe_addr(DEV_1,HS6200_BANK0_RX_ADDR_P1,Addr_Temp,Addr_Width);  //读地址
// 							nRF24L01_X_read_pipe_addr(DEV_1,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,1);  //读地址	
// 						} 
// 						else 	//PIPE0 PIPE1
// 							nRF24L01_X_read_pipe_addr(DEV_1,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,Addr_Width);  //读地址
// 						nRF24L01_X_write_pipe_addr(DEV_0, HS6200_BANK0_TX_ADDR,Addr_Temp,Addr_Width);   //写地址
// 						if( nRF24L01_X_is_auto_ack_px(DEV_0,0) )
// 							nRF24L01_X_write_pipe_addr(DEV_0, HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width);   //写地址
// 					}
// 					else if(Dev_Flag[DEV_0]==HS6200_DEV)	 //HS6200 dev
// 					{
// 						if(Cmd_Argument[2]>PIPE0) //PIPE1 PIPE2 PIPE3 PIPE4 PIPE5 
// 						{
// 							nRF24L01_X_read_pipe_addr(DEV_1,HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width);  //读地址
// 							nRF24L01_X_read_pipe_addr(DEV_1,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,1);  //读地址								
// 						}
// 						else	//PIPE0
// 						{
// 							nRF24L01_X_read_pipe_addr(DEV_1,HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width);  //读地址
// 						}
// 						nRF24L01_X_write_pipe_addr(DEV_0, HS6200_BANK0_TX_ADDR,Addr_Temp,Addr_Width);   //写地址
// 						if( nRF24L01_X_is_auto_ack_px(DEV_0,0) ) 
// 							nRF24L01_X_write_pipe_addr(DEV_0, HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width);   //写地址
// 					}					
// 				}
// 				else if(DevNum==DEV_1) 		//DEV_1
// 				{
// 					Addr_Width=nRF24L01_X_get_aw(DEV_0);
// 					nRF24L01_X_set_address_width(DEV_1,Addr_Width);
// 					if(Dev_Flag[DEV_0]==nRF24L01_DEV)  //nRF24L01 dev
// 					{
// 						if(Cmd_Argument[2]>PIPE1)  //PIPE2 PIPE3 PIPE4 PIPE5
// 						{
// 							nRF24L01_X_read_pipe_addr(DEV_0,HS6200_BANK0_RX_ADDR_P1,Addr_Temp,Addr_Width);  //读地址
// 							nRF24L01_X_read_pipe_addr(DEV_0,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,1);  //读地址	
// 						} 
// 						else 	//PIPE0 PIPE1
// 							nRF24L01_X_read_pipe_addr(DEV_0,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,Addr_Width);  //读地址
// 						nRF24L01_X_write_pipe_addr(DEV_1, HS6200_BANK0_TX_ADDR,Addr_Temp,Addr_Width);   //写地址
// 						if( nRF24L01_X_is_auto_ack_px(DEV_1,0) ) 
// 							nRF24L01_X_write_pipe_addr(DEV_1, HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width);   //写地址
// 					}
// 					else if(Dev_Flag[DEV_0]==HS6200_DEV)	 //HS6200 dev
// 					{
// 						if(Cmd_Argument[2]>PIPE0) //PIPE1 PIPE2 PIPE3 PIPE4 PIPE5 
// 						{
// 							nRF24L01_X_read_pipe_addr(DEV_0,HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width);  //读地址
// 							nRF24L01_X_read_pipe_addr(DEV_0,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,1);  //读地址								
// 						}
// 						else	//PIPE0
// 						{
// 							nRF24L01_X_read_pipe_addr(DEV_0,HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width);  //读地址
// 						}
// 						nRF24L01_X_write_pipe_addr(DEV_1, HS6200_BANK0_TX_ADDR,Addr_Temp,Addr_Width);   //写地址
// 						if( nRF24L01_X_is_auto_ack_px(DEV_1,0) ) 
// 							nRF24L01_X_write_pipe_addr(DEV_1, HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width);   //写地址
// 					}					
// 				}

				if(Dev_Flag[DevNum]!=DEV_NONE)   //DEV_NONE
				{
					Addr_Width=nRF24L01_X_get_aw(DevNum);
					if(Dev_Flag[DevNum]==HS6200_DEV)
					{						
						//nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width);  //读地址
                       			 for(i=0x00;i<Addr_Width;i++)Addr_Temp[i]=Address_Pipe0[i];
						nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,1);  //读地址--覆盖地址的低字节部分	
                        if(Cmd_Argument[2]==PIPE0)
                        {
                             for(i=0;i<Addr_Width;i++)Addr_Temp[i]=Address_Pipe0[i];
                        }
                        else
                            nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,1);  //读地址--覆盖地址的低字节部分	                             
					}
					else if(Dev_Flag[DevNum]==nRF24L01_DEV)
					{
						if( Cmd_Argument[2]==PIPE0 )        //PIPE0 
						{
							//nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,Addr_Width);  //读地址
                            for(i=0;i<Addr_Width;i++)Addr_Temp[i]=Address_Pipe0[i];
						}
                        else if (Cmd_Argument[2]==PIPE1)	// PIPE1			
                        {
                            nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,Addr_Width);  //读地址
                        }
                        else if (Cmd_Argument[2]>PIPE1)     //PIPEx 
						{
							nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P1,Addr_Temp,Addr_Width);  //读地址
							nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,1);  //读地址--覆盖地址的低字节部分	
						}
					}
	//				nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_TX_ADDR,Addr_Temp,Addr_Width);      //写发送地址   
    //              nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width);   //写PIPE0地址                  
				 }			
//				 if ( (!nRF24L01_X_Busy[DevNum])  )		 //not busy began to tx
				 {
				 	 nRF24L01_X_Busy[DevNum] = 1;
					 if(nRF24L01_X_is_tx_fifo_full(DevNum))   //Tx fifo Full
					 {
						 	
					 }
					 nRF24L01_X_write_tx_payload(DevNum,&Cmd_Argument[3],Cmd_Argument_Length-0x03);					
//					 if(Nop_After_W_Tx_Payload)nRF24L01_X_nop(DevNum);
//					 if(Tx_By_CE_High) nRF24L01_X_ce_high(DevNum); 
//					 else nRF24L01_X_ce_high_pulse(DevNum); 
					 
					 Ack_Pipe_Num=Cmd_Argument[2];  //应答通道号,发送了才记住通道号  
				 }
//                  else 
//                  {
//                      if(  ( (DevNum==DEV_0) && (nRF24L01_0_EI==0) && (Dev_Flag[DEV_0]!=DEV_NONE)) ||  ( (DevNum==DEV_1) && (nRF24L01_1_EI==0) && (Dev_Flag[DEV_1]!=DEV_NONE) )  )
//                      {
//                         nRF24L01_X_write_tx_payload(DevNum,&Cmd_Argument[3],Cmd_Argument_Length-0x03);					
//                         if(Nop_After_W_Tx_Payload)nRF24L01_X_nop(DevNum);
//                         if(Tx_By_CE_High) nRF24L01_X_ce_high(DevNum); 
//                         else nRF24L01_X_ce_high_pulse(DevNum); 
//                      }
//                  }
                     
			}
			/* 
			 * Cmd_Argument[0]	   Cmd_Argument[1]   Cmd_Argument[2..6]         Cmd_Argument[5/7]  
			 *	  TXPL				 Addr width			  address		  packet context(32Bytes Max)
			 *	  1Byte				   1Byte	          3-5Bytes 				   1-32Bytes
			 * 
			 */
			else if( (Cmd_Argument[1]==COMM_ADDR_WIDTH3) || (Cmd_Argument[1]==COMM_ADDR_WIDTH4) || (Cmd_Argument[1]==COMM_ADDR_WIDTH5) )	 //给定地址
			{
				Addr_Width=Cmd_Argument[1];  //addr width
				nRF24L01_X_set_address_width(DevNum,Addr_Width);  //set Tx address width
				for(i=0x00;i<Addr_Width;i++)Addr_Temp[i]=Cmd_Argument[Addr_Width+0x01-i];   //0x02+Cmd_Arguemnt[1]-0x01-i
				nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_TX_ADDR,Addr_Temp,Addr_Width);  //set Tx address
				if( nRF24L01_X_is_auto_ack_px(DevNum, 0) ) 	    //auto ACK
				{
                    nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width); 		
                    for(i=0x00;i<Addr_Width;i++)Address_Pipe0[i]=Addr_Temp[i];                            //存储Pipe0的地址
                }
			
//				if (!nRF24L01_X_Busy[DevNum])
			    {
			        nRF24L01_X_Busy[DevNum] = 1;  			
					if(nRF24L01_X_is_tx_fifo_full(DevNum))   //Tx fifo Full
					{
						
					}					
					nRF24L01_X_write_tx_payload(DevNum,&Cmd_Argument[2+Cmd_Argument[1]],Cmd_Argument_Length-0x02-Cmd_Argument[1]);
					if(Nop_After_W_Tx_Payload)nRF24L01_X_nop(DevNum);  //此处必须有SPI操作，否则 HS6200内部的FIFO写指针不会移动，就会造成发空包的问题
			        if(Tx_By_CE_High) nRF24L01_X_ce_high(DevNum); //CE 为高时发送， 因为ce为低时内部PLL无时钟
					else nRF24L01_X_ce_high_pulse(DevNum);       //pulse 发送
			    }
//                 else 
//                 {
//                      if(  ( (DevNum==DEV_0) && (nRF24L01_0_EI==0) && (Dev_Flag[DEV_0]!=DEV_NONE)) ||  ( (DevNum==DEV_1) && (nRF24L01_1_EI==0) && (Dev_Flag[DEV_1]!=DEV_NONE) )  )
//                      {
//                         nRF24L01_X_write_tx_payload(DevNum,&Cmd_Argument[2+Cmd_Argument[1]],Cmd_Argument_Length-0x02-Cmd_Argument[1]);				
//                         if(Nop_After_W_Tx_Payload)nRF24L01_X_nop(DevNum);
//                         if(Tx_By_CE_High) nRF24L01_X_ce_high(DevNum); 
//                         else nRF24L01_X_ce_high_pulse(DevNum); 
//                      }
//                 }
			}
			else			  //data err
			{
				#ifdef HS6200_DEBUG
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_COMM_DATA);
					#else
				    	USB_Out_Err_Info(DevNum,ERR_USB_PACKET_COMM_DATA);	
					#endif
				#endif  /*HS6200_DEBUG*/
			}		
		break;
		case TXPLNAK:
			if(Cmd_Argument[1]==COMM_PIPE)	  //给定通道
			{				
				if(Dev_Flag[DevNum]!=DEV_NONE)   //DEV_NONE
				{
					Addr_Width=nRF24L01_X_get_aw(DevNum);
					if(Dev_Flag[DevNum]==HS6200_DEV)
					{								
                        if(Cmd_Argument[2]==PIPE0)
                        {
                             for(i=0;i<Addr_Width;i++)Addr_Temp[i]=Address_Pipe0[i];
                        }
                        else
                            nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,1);  //读地址--覆盖地址的低字节部分	          
					}
					else if(Dev_Flag[DevNum]==nRF24L01_DEV)
					{
                        if( Cmd_Argument[2]==PIPE0 )        //PIPE0 
						{
							//nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,Addr_Width);  //读地址
                            for(i=0;i<Addr_Width;i++)Addr_Temp[i]=Address_Pipe0[i];
						}
                        else if (Cmd_Argument[2]==PIPE1)	// PIPE1			
                        {
                            nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,Addr_Width);  //读地址
                        }
                        else if (Cmd_Argument[2]>PIPE1)     //PIPEx 
						{
							nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P1,Addr_Temp,Addr_Width);  //读地址
							nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,1);  //读地址--覆盖地址的低字节部分	
						}
// 						if(Cmd_Argument[2]>PIPE1)
// 						{
// 							nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P1,Addr_Temp,Addr_Width);  //读地址
// 							nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,1);  //读地址--覆盖地址的低字节部分
// 						}
// 						else if( ((Cmd_Argument[2]==PIPE0) || (Cmd_Argument[2]==PIPE1)) )
// 						{
// 							nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0+Cmd_Argument[2],Addr_Temp,Addr_Width);  //读地址
// 						}
					}
					nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_TX_ADDR,Addr_Temp,Addr_Width);   //写地址
				 }
//				if(!nRF24L01_X_Busy[DevNum])
				{
					if(nRF24L01_X_is_tx_fifo_full(DevNum))   //Tx fifo Full
					{
					
					}
					nRF24L01_X_Busy[DevNum] = 1;  
					nRF24L01_X_write_tx_payload_noack(DevNum,&Cmd_Argument[3],Cmd_Argument_Length-0x03);
					if(Nop_After_W_Tx_Payload)nRF24L01_X_nop(DevNum);
					if(Tx_By_CE_High) nRF24L01_X_ce_high(DevNum); 
					else nRF24L01_X_ce_high_pulse(DevNum);
				}
//                else 
//                {
//                     if(  ( (DevNum==DEV_0) && (nRF24L01_0_EI==0) && (Dev_Flag[DEV_0]!=DEV_NONE)) ||  ( (DevNum==DEV_1) && (nRF24L01_1_EI==0) && (Dev_Flag[DEV_1]!=DEV_NONE) )  )
//                    {
//                        nRF24L01_X_write_tx_payload_noack(DevNum,&Cmd_Argument[3],Cmd_Argument_Length-0x03);					
//                        if(Nop_After_W_Tx_Payload)nRF24L01_X_nop(DevNum);
//                        if(Tx_By_CE_High) nRF24L01_X_ce_high(DevNum); 
//                        else nRF24L01_X_ce_high_pulse(DevNum); 
//                     }
//                 }				
			}
			/*
			 * Cmd_Argument[0]   Cmd_Argument[1]   Cmd_Argument[2..6]   Cmd_Argument[5/7..36/38]
			 *    TXPLNAK		   Addr Width		      addr	                context
			 *     1Byte		     1Byte				3/5Bytes			   1-32Bytes
			 *
			 */
			else if( (Cmd_Argument[1]==COMM_ADDR_WIDTH3) || (Cmd_Argument[1]==COMM_ADDR_WIDTH4) || (Cmd_Argument[1]==COMM_ADDR_WIDTH5) )	 //给定地址
			{
				Addr_Width=Cmd_Argument[1];
				nRF24L01_X_set_address_width(DevNum,Addr_Width);  //set Tx address width
				for(i=0x00;i<Addr_Width;i++)Addr_Temp[i]=Cmd_Argument[Addr_Width+0x01-i];   //0x02+Addr_Width-0x01-i
				nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_TX_ADDR,Addr_Temp,Addr_Width);  //set Tx address
				if( (nRF24L01_X_read_reg(DevNum, nRF24L01_EN_AA) & nRF24L01_EN_AA_P0) ) // 自动应答条件下
						nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_RX_ADDR_P0,Addr_Temp,Addr_Width);  //set Rx Pipe0 address
				
				if(!nRF24L01_X_Busy[DevNum])
				{
					nRF24L01_X_Busy[DevNum] = 1;  
					if(nRF24L01_X_is_tx_fifo_full(DevNum))   //Tx fifo Full
					{
						
					}
					   	
					nRF24L01_X_write_tx_payload_noack(DevNum,&Cmd_Argument[2+Cmd_Argument[1]],Cmd_Argument_Length-0x02-Cmd_Argument[1]);  //Payload length=Cmd_Arguemnt_Length-TXPL cmd(1Byte)-addr width arguemnt(1Byte)-Addr width  
					if(Nop_After_W_Tx_Payload)nRF24L01_X_nop(DevNum);
					if(Tx_By_CE_High) nRF24L01_X_ce_high(DevNum); 
					else nRF24L01_X_ce_high_pulse(DevNum);
				}
                else
                {
 //                    if(  ( (DevNum==DEV_0) && (nRF24L01_0_EI==0) && (Dev_Flag[DEV_0]!=DEV_NONE)) ||  ( (DevNum==DEV_1) && (nRF24L01_1_EI==0) && (Dev_Flag[DEV_1]!=DEV_NONE) )  )
                     {
                       nRF24L01_X_write_tx_payload_noack(DevNum,&Cmd_Argument[2+Cmd_Argument[1]],Cmd_Argument_Length-0x02-Cmd_Argument[1]);				
                        if(Nop_After_W_Tx_Payload)nRF24L01_X_nop(DevNum);
                        if(Tx_By_CE_High) nRF24L01_X_ce_high(DevNum); 
                        else nRF24L01_X_ce_high_pulse(DevNum); 
                     }
                }
			}
			else
			{
				 #ifdef HS6200_DEBUG
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_COMM_DATA);
					#else
				    	USB_Out_Err_Info(DevNum,ERR_USB_PACKET_COMM_DATA);	
					#endif
				#endif  /*HS6200_DEBUG*/
			}
		break;

		/*
		 * Cmd_Argument[0]	 Cmd_Argument[1] Cmd_Argument[2]  Cmd_Argument[3..34]
		 *	  ACKPL			   Pipe width       Pipe Idx	   ACK Payload
		 *	  0x02			     0x01          Pipe0-Pipe5   	1-32Bytes
		 *    1Byte			    1Byte            1Byte			   xx
		 *
		 */
		case ACKPL:	         //ACK包只支持给定通道的应答
			if(Cmd_Argument[1]==COMM_PIPE)	  //给定通道
			{
				if(Cmd_Argument[2]<=PIPE5)	     //pipe right
				{
					nRF24L01_X_write_ack_payload(DevNum,Cmd_Argument[2],&Cmd_Argument[3],Cmd_Argument_Length-0x03);  //write ACK payload 
					USB_ACK_2_Host(DevNum,0,NULL,0);
				}
				else			  //pipe err
				{
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_COMM_PIPE);
						#else
		    				USB_Out_Err_Info(DevNum,ERR_USB_PACKET_COMM_PIPE);	
						#endif
					#endif  /*HS6200_DEBUG*/	
				} 				
			}
			else  //data err
			{
				#ifdef HS6200_DEBUG
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_COMM_DATA);
					#else
				    	USB_Out_Err_Info(DevNum,ERR_USB_PACKET_COMM_DATA);	
					#endif
				#endif  /*HS6200_DEBUG*/
			}
		break;
		case COMM_INIT:
			nRF24L01_X_ptx_init(DEV_0);  //PTX init
    		nRF24L01_X_prx_init(DEV_1);  //PRX init
			USB_ACK_2_Host(DEV_0,0,NULL,0);
		break;
		default:
		break;
	}
}
void Comm_Protocol_Resolut(void) 
{
	U8 i;	
	U8 DevNum;
	U8 Cmd_Argument[39];  	// Cmd(1Byte)+ Address_Length(1Byte)+Address(Max 5Bytes)+Payload(32Bytes)=39Bytes (Max length)	
	U8 Cmd_Argument_Length=0x00;

	HS6200_Bank0_Activate(DEV_0);  //Bank0 status
	HS6200_Bank0_Activate(DEV_1);  //Bank0 status
	
	DevNum=USB_Host_Out_Packet[1];
	Cmd_Argument_Length=USB_Host_Out_Packet[2];										   
	if( (Cmd_Argument_Length<=39) || (Cmd_Argument_Length>=0x04))  // Cmd(1Byte)+ Address_Length(1Byte)+Address(Max 5Bytes)+Payload(Max 32Bytes)=39Bytes  
	{															   // Cmd(1Byte)+ pipe_cmd(1Byte) + pipe num(1Byte)+Payload(Min 1Byte)=4Bytes
		for(i=0x00;i<Cmd_Argument_Length;i++) Cmd_Argument[i]=USB_Host_Out_Packet[3+i];	
		
		if(DevNum==RF_DEV_0)Comn_Mode(DEV_0,Cmd_Argument,Cmd_Argument_Length); 
		else if(DevNum==RF_DEV_1)Comn_Mode(DEV_1,Cmd_Argument,Cmd_Argument_Length); 
		else  //dev err
	    {
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_COMM_RF_DEV);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_COMM_RF_DEV);
				#endif
			#endif	
		}					
	}
	else   //length err
	{
		#ifdef HS6200_DEBUG
			#if (DEBUG_OUT_ERR==LED_OUT_ERR)
				LED_Out_Err_Info(ERR_USB_PACKET_COMM_LENGTH);
			#else
				USB_Out_Err_Info(DevNum,ERR_USB_PACKET_COMM_LENGTH);
			#endif
		#endif	
	}
}

/*--------------------------------------------------------Setup Test-------------------------------------------------------------*/
/*
 * 
 *
 */
void SetupMode(U8 DevNum,U8* Cmd_Argument,U8 Cmd_Argument_Length)
{
	U8 Temp[5];  //用于更改地址
	U8 i=0x00;

	switch(Cmd_Argument[0])
	{
		
		case SET_POWER_UP:
			nRF24L01_X_set_powerup(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_PWOER_DOWN:
			nRF24L01_X_set_powerdown(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0); 
		break;
	    case SET_CRC:
			if( Cmd_Argument_Length==0x02 ) 
			{
				if( (Cmd_Argument[1]==CRC_0) || (Cmd_Argument[1]==CRC_1) || (Cmd_Argument[1]==CRC_2))
				{
					nRF24L01_X_set_crc(DevNum,Cmd_Argument[1]);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				}
				else
				{
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_SETUP_DATA);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_DATA);
						#endif
				    #endif
				}
			}
			else 
			{
				#ifdef HS6200_DEBUG
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_SETUP_LENGTH);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_LENGTH);
					#endif
				#endif
			}
		break;
	    /*
		 * Argument=0x00   250us
		 * Argument=0x01   500us
		 * Argument=0x02   750us
		 * ...
		 * Argument=0x0F   4000us
		 *
		 *deylay=(Argument+0x01)*250us
		 *
		 */
		case SET_ARD:        /*set auto retransmit delay*/
			if(Cmd_Argument_Length==0x02)
			{
				if(Cmd_Argument[1]<=0x0F)
				{			
					nRF24L01_X_set_auto_retr_delay(DevNum,Cmd_Argument[1]);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				}
				else
				{
					 #ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_SETUP_DATA);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_DATA);
						#endif
					#endif
				}
			}
			else
			{
				#ifdef HS6200_DEBUG
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_SETUP_LENGTH);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_LENGTH);
					#endif
				#endif
			}
		break;

		/*
		 * Argument=0x00   retransmit disable
		 * Argument=0x01   1
		 * Argument=0x02   2
		 * ...
		 * Argument=0x0F   15
 		 */	
 		case SET_ARC:        /*set auto retransmit count*/
			if(Cmd_Argument_Length==0x02)
			{
				if(Cmd_Argument[1]<=0x0F)
				{
					nRF24L01_X_set_auto_retr_count(DevNum,Cmd_Argument[1]);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				}
				else
				{
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_SETUP_DATA);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_DATA);
						#endif
					#endif
				}
			}
			else
			{
				#ifdef HS6200_DEBUG
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_SETUP_LENGTH);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_LENGTH);
					#endif
				#endif
			}
		break;
		case SET_RF_CH:
			if(Cmd_Argument[1]<=125)		   //channal:0-125
			{
				nRF24L01_X_set_rf_ch(DevNum,Cmd_Argument[1]);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			}
			else
			{
				#ifdef HS6200_DEBUG
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_SETUP_DATA);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_DATA);
					#endif
				#endif	
			}
		break;
		case SET_RF_AIR_DATARATE:  
			if(Dev_Flag[DevNum]==HS6200_DEV)   //HS6200
			{
				HS6200_Data_Rate(DevNum,Cmd_Argument[1]);
			}
			else							  //nRF24L01
				nRF24L01_X_set_rf_datarate(DevNum,Cmd_Argument[1]);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_RF_OUTPUT_POWER:
			if(Dev_Flag[DevNum]==HS6200_DEV)	//HS6200 dev
			{
				if(Cmd_Argument[1]<=0x04)
				{
					HS6200_PA_power(DevNum,Cmd_Argument[1]);
					USB_ACK_2_Host(DevNum,0,NULL,0);				
				}
				else
				{
					  #ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_SETUP_DATA);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_DATA);
						#endif
					#endif
				}
			}
			else				//nRF24L01 dev
			{
				if(Cmd_Argument[1]<=0x03)
				{
					nRF24L01_X_set_rf_output_power(DevNum,Cmd_Argument[1]);
					USB_ACK_2_Host(DevNum,0,NULL,0);			
				}
				else
				{
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_SETUP_DATA);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_DATA);
						#endif
					#endif
				}
			}
		break;
		case SET_INT_MASK_RX_DR:
			 nRF24L01_X_set_mask_rx_dr(DevNum,Cmd_Argument[1]);
			 USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_INT_MASK_TX_DS:
			 nRF24L01_X_set_mask_tx_ds(DevNum,Cmd_Argument[1]);
			 USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_INT_MASK_MAX_RT:
			 nRF24L01_X_set_mask_max_rt(DevNum,Cmd_Argument[1]);
			 USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_PIPE_ADDR_WIDTH:
			nRF24L01_X_set_address_width(DevNum,Cmd_Argument[1]);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_PIPE_TX_ADDR:
			for(i=0x00;i<Cmd_Argument_Length-0x01;i++)Temp[i]=Cmd_Argument[Cmd_Argument_Length-0x01-i];	   //Cmd_Argument_Length-0x01-0x01+i+0x01
			nRF24L01_X_write_pipe_addr(DevNum,nRF24L01_TX_ADDR,Temp,Cmd_Argument_Length-0x01);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_PIPE_EN: 
			if(Cmd_Argument_Length==0x03)
			{
				if( (Cmd_Argument[2]==0) || (Cmd_Argument[2]==1) )
				{
					nRF24L01_X_set_en_rx_pipe(DevNum,Cmd_Argument[1],Cmd_Argument[2]);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				}
				else
				{
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_SETUP_DATA);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_DATA);
						#endif
					#endif
				}
			}
			else   //length err
			{
				#ifdef HS6200_DEBUG
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_SETUP_LENGTH);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_LENGTH);
					#endif
				#endif
			}
		break;
		case SET_PIPE_AUTO_ACK:
			nRF24L01_X_set_en_auto_ack_pipe(DevNum,Cmd_Argument[1],Cmd_Argument[2]);	  //DevNum/pipe/en
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_PIPE_DPL:		//dynamic payload length
			if(Cmd_Argument[2])		  
			{
				nRF24L01_X_set_en_dpl_px(DevNum,Cmd_Argument[1],1);
			}
			else
			{
				nRF24L01_X_set_en_dpl_px(DevNum,Cmd_Argument[1],0);				
			}
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_PIPE_ADDR:
			for(i=0x00;i<Cmd_Argument_Length-0x02;i++)Temp[i]=Cmd_Argument[Cmd_Argument_Length-i-0x01];	   //Cmd_Argument_Length-0x02-0x01+i+0x02
            if(Cmd_Argument[1]==PIPE0) for(i=0x00;i<Cmd_Argument_Length-0x02;i++)Address_Pipe0[i]= Temp[i];  //存储Pipe0的地址              
            nRF24L01_X_write_pipe_addr(DevNum,nRF24L01_RX_ADDR_P0+Cmd_Argument[1],Temp,Cmd_Argument_Length-0x02);  //minus cmd code(1Byte)			 
            USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_PIPE_STATIC_PLW:
			nRF24L01_X_set_rx_pipe_static_pay_load_width(DevNum,Cmd_Argument[1],Cmd_Argument[2]);	//设置PAYLOAD length
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;		
		case SET_PTX:
			nRF24L01_X_set_ptx(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break; 
		case SET_PRX:
			nRF24L01_X_set_prx(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_SPL:
			nRF24L01_X_set_en_dpl_feature(DevNum,0);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_DPL:
			nRF24L01_X_set_en_dpl_feature(DevNum,1);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_SA:
			nRF24L01_X_sa_mode(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_DA:
			nRF24L01_X_da_mode(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_NAK:
			nRF24L01_X_noack_mode(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_ACK:
			nRF24L01_X_ack_mode(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_AE:    //ACK empty mode
			nRF24L01_X_ack_empty_mode(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_AP:
			nRF24L01_X_ack_pay_mode(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_NAR:		 //no auto retransmit
//			nRF24L01_X_noretr_mode(DevNum); 
//			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_AR:		 //auto retransmits
//			if(Cmd_Argument_Length==0x02)
//			{
//				nRF24L01_X_ar_mode(DevNum,Cmd_Argument[1]);		 //根据参数进行调整
//			}
//			else
//			{
//				 nRF24L01_X_ar_mode(DevNum,10);        //10次重发
//			}
//			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_NMR:
			nRF24L01_X_mr_mode(DevNum,0);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_MR:
			nRF24L01_X_mr_mode(DevNum,1);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_COMMODE:
			if(Cmd_Argument_Length==0x02)
			{
				HS6200_Mode_Config(DevNum,Cmd_Argument[1]);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			}
			else
			{
				#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SETUP_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}			
		break;
		case SET_CE_LOW_BEFORE_W_REG:
			if(Cmd_Argument[1]==0x01)
				ce_low_befor_write=1;
			else
				ce_low_befor_write=0; 
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_CAL_AFTER_ACK:
			if(Cmd_Argument[1]==0x01)
				Cal_after_Ack=1;
			else
				Cal_after_Ack=0; 
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SET_MCU_MASK_EX0:
			if(Cmd_Argument_Length==0x02)
			{
//				if(Cmd_Argument[1]==0x01)
//					nRF24L01_0_EI=0;
//				else if(Dev_Flag[DEV_0]==DEV_NONE)	  // 如果是DEV_NONE 则不能打开xx_EI，因为打开后，程序进入中断，就会死在中断里
//					nRF24L01_0_EI=0;
//				else nRF24L01_0_EI=1;
				USB_ACK_2_Host(DevNum,0,NULL,0);
			}
			else		//length  err
			{
				#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SETUP_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}
		break;
		case SET_MCU_MASK_EX1:
			if(Cmd_Argument_Length==0x02)
			{
//				if(Cmd_Argument[1]==0x01)
//					nRF24L01_1_EI=0;
//				else if(Dev_Flag[DEV_1]==DEV_NONE)	  // 如果是DEV_NONE 则不能打开xx_EI，因为打开后，程序进入中断，就会死在中断里
//					 nRF24L01_1_EI=0;
//				else nRF24L01_1_EI=1;	 //有dev的时候，才打开中断
				USB_ACK_2_Host(DevNum,0,NULL,0);
			}
			else		  //length err
			{
				#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SETUP_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}
		break;
		default:     //cmd err
		{
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_SETUP_CMD);
				#else
				    USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_CMD);	
				#endif
			#endif  /*HS6200_DEBUG*/
		}
		break;
	}
}

/*
 *	 P[0]      P[1]            P[2]                 P[3]		P[4]
 *	 PID	   dev		 Packet Context Length	  cmd code	  argument
 *	0x02	DEV_0/DEV_1		    xx					xx		    xx
 *	1Byte	  1Byte			  1Byte				   1Byte
 *
 */

//setup protocol resolution
void Setup_Protocol_Resolut(void)  //make resolution protocol of between PC and MCU(CPU) communication 
{ 
	U8 i;
	U8 DevNum;
	U8 Cmd_Argument[10];	
	U8 Cmd_Argument_Length=0x00;
	DevNum=USB_Host_Out_Packet[1];
	Cmd_Argument_Length=USB_Host_Out_Packet[2];		   //cmd argument length
	if(Cmd_Argument_Length<=10)
	{
		for(i=0x00;i<Cmd_Argument_Length;i++)Cmd_Argument[i]=USB_Host_Out_Packet[3+i];	 //P[3..n]-->cmd & argument
		
		if(USB_Host_Out_Packet[1]==RF_DEV_0) 
		{
			HS6200_Bank0_Activate(DEV_0);
			SetupMode(DEV_0,Cmd_Argument,Cmd_Argument_Length);
		}
		else if(USB_Host_Out_Packet[1]==RF_DEV_1)
		{
		    HS6200_Bank0_Activate(DEV_1);
			SetupMode(DEV_1,Cmd_Argument,Cmd_Argument_Length);
		}
		else        //rf dev err
		{
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_SETUP_RF_DEV);
				#else
				    USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_RF_DEV);
				#endif	
			#endif  /*HS6200_DEBUG*/
		}
	}	
	else
	{	
		#ifdef HS6200_DEBUG
			#if (DEBUG_OUT_ERR==LED_OUT_ERR)
				LED_Out_Err_Info(ERR_USB_PACKET_SETUP_LENGTH);
			#else
			    USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SETUP_LENGTH);
			#endif	
		#endif  /*HS6200_DEBUG*/		
	}	
}

/*---------------------------------------------SPI Test-------------------------------------------------*/
void SPI_Mode(U8 DevNum,U8 Cmd,U8 *Argument,U8 Argument_Length)
{
	U8 pTemp[33];  //story payload and so on
	U8 i;
	switch(Cmd)        //Cmd: SPI Cmd -->similar to nRF24L01 SPI command 
	{
		case nRF24L01_R_REGISTER:     //read register
		/*
		 *    Argument[0]                 Argument[1]                    
		 *   Register Addr    Bytes(1= read 1Byte ,other= n bytes)  
         *  
         *  Argument_Length=2 	
		 * Register Addr: Bank1 register address's MSBit is 1,while Bank0 is 0.	
		 */
            if(Argument_Length==0x02)
			{	
				pTemp[0]=Argument[0];	     //reg addr
				pTemp[1]=Argument[1];		 //length
				if( (Argument[0]& HS6200_BANK_BIT)==HS6200_BANK1 )	   //read Bank1
				{	
					HS6200_Bank1_Activate(DevNum);
					Argument[0]&=~HS6200_BANK_BIT;
				}
				else HS6200_Bank0_Activate(DevNum);		      //read bank0
			
				/*  reg addr     xx Bytes    reg val
				 *  1Byte         1Byte       1Byte 
				 *
				 */
				if(Argument[1]>0x01) //multibytes  				  
				{
					nRF24L01_X_read_reg_multibytes(DevNum,Argument[0],(pTemp+2),Argument[1]);  //read multibytes
					USB_ACK_2_Host(DevNum,TYPE_REG,pTemp,Argument[1]+2);	
				}
				else 	              //1Byte
				{	
					pTemp[2]=nRF24L01_X_read_reg(DevNum,Argument[0]); //read one byte
					USB_ACK_2_Host(DevNum,TYPE_REG,pTemp,3);
				}
			}
			else		//arguement length err
			{
				#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/	
			}
		break;
		case nRF24L01_W_REGISTER:
		  /*
		   *  Argument[0]               Argument[1]                      Argument[2...n]
		   *  Register Addr    Bytes(1= write 1Byte ,n=multibytes)       register value 
		   *
		   * Argument_Length=n 
		   */
		    if(Argument_Length>=0x03)
			{
				if( (Argument[0]& HS6200_BANK_BIT)==HS6200_BANK1 )	   //write Bank1
				{	
					HS6200_Bank1_Activate(DevNum);
					Argument[0]&=~HS6200_BANK_BIT;
				}
				else     //bank 0
                {
                    HS6200_Bank0_Activate(DevNum);		   //write bank0
                    if(Argument[0]==nRF24L01_RX_ADDR_P0) for(i=0x00;i<Argument[1];i++)Address_Pipe0[i]=Argument[2+i];   //存储Pipe0的地址
                }
				if(Argument[1]>0x01)nRF24L01_X_write_reg_multibytes(DevNum,Argument[0],&Argument[2],Argument[1]);  //write multibytes
				else nRF24L01_X_write_reg(DevNum,Argument[0],Argument[2]);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			}
			else 
			{
				#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/		
			}
		break;
		case nRF24L01_R_RX_PAYLOAD:
		  /*
		   *  Argument[0]
		   *   xxBytes
		   *  read rx payload bytes
		   *  
		   *  Argument_Length=1 
		   */
		    if(Argument_Length==0x01)
			{
				pTemp[0]=0x00;    //这个地方通道号设为0                  
		    	nRF24L01_X_read_rx_payload(DevNum,&pTemp[1],Argument[0]);		  
				USB_ACK_2_Host(DevNum,TYPE_PLD,pTemp,Argument[0]+1);  
			}
			else
			{
				#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}	
		break;
		case nRF24L01_W_TX_PAYLOAD:
		  /*
		   *           Argument[0]                  Argument[1...n]
		   *  xxBytes (write tx payload bytes)          payload
		   *          
		   * Argument_Length=n;
		   */
		   if( (Argument_Length>=0x02) && (Argument_Length<=0x33) )
		   {
				 for(i=0x00;i<(Argument_Length-0x01);i++)pTemp[i]=Argument[1+i];	 //minus Argument[0] (xxBytes)
				 nRF24L01_X_write_tx_payload(DevNum,pTemp,Argument[0]);
			     USB_ACK_2_Host(DevNum,0,NULL,0);
		   }
		   else 
		   {
		   		#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}	
		break;
		case nRF24L01_FLUSH_TX:
		  /*
		   *    Argument[0]              
		   *      NULL
		   * 
           *  Argument_Length=0		
		   */
		   if(Argument_Length==0x00)
		   {  	
		   		nRF24L01_X_flush_tx(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
		   }
 		   else 
		   {
		   		#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}	
		break;
		case nRF24L01_FLUSH_RX:
			/*
		   *    Argument[0]              
		   *      NULL
		   * 
           *  Argument_Length=0		
		   */
		   if(Argument_Length==0x00)
		   {
				nRF24L01_X_flush_rx(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
		   }
		   else 
		   {
		   		#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}		
		break;
		case nRF24L01_REUSE_TX_PL:
		  /*
		   *    Argument[0]              
		   *      NULL
		   * 
           *  Argument_Length=0		
		   */
		   if(Argument_Length==0x00)
		   {
				nRF24L01_X_reuse_tx_payload(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
		   }
		   else 
		   {
		   		#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}	
		break;
		case nRF24L01_ACTIVATE:
			/*
		   *    Argument[0]              
		   *      NULL
		   * 
           *  Argument_Length=0		
		   */
		   if(Argument_Length==0x00)
		   {
				HS6200_Activate(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
		   }
		   else 
		   {
		   		#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}	
		break;
		case nRF24L01_R_RX_PL_WID:
		  /*
		   *    Argument[0]              
		   *      NULL
		   * 
           *  Argument_Length=0		
		   */
		   if(Argument_Length==0x00)
		   {
				pTemp[0]=SDS_R_RX_PL_WID; 
				pTemp[1]=nRF24L01_X_read_rx_payload_width(DevNum);
				USB_ACK_2_Host(DevNum,TYPE_SDS,pTemp,2);
		   }
		   else 
		   {
		   		#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}	
		break;
		case nRF24L01_W_ACK_PAYLOAD:
		  /*
		   * Argument[0]          Argument[1]                  Argument[2...n]                  
		   *  pipe num      Bytes (write ACK payload bytes)      ACK payload
		   *          
		   * Argument_Length=n
		   */
		   	if( (Argument_Length>=0x03)&& (Argument_Length<=0x34) )
			{
				for(i=0x00;i<Argument_Length-0x02;i++) pTemp[i]=Argument[i+2];    //minus pipe num and xxBytes(-2Bytes)
				nRF24L01_X_write_ack_payload(DevNum,Argument[0],pTemp,Argument[1]);  //(write ACK payload bytes)
				USB_ACK_2_Host(DevNum,0,NULL,0);
			}
		    else 
		   {
		   		#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}	
    	break;
    	case nRF24L01_W_TX_PAYLOAD_NOACK:
		  /*
		   *           Argument[0]                          Argument[1...n]
		   *  Bytes (write tx payload no ACK  bytes)           payload no ACK
		   *          
		   * Argument_Length=n;
		   */
		    if( (Argument_Length>=0x02) && (Argument_Length<=0x33) )
			{
				for(i=0x00;i<Argument_Length-0x01;i++)pTemp[i]=Argument[i+1];	//minus xxBytes
				nRF24L01_X_write_tx_payload_noack(DevNum,pTemp,Argument[0]);
				USB_ACK_2_Host(DevNum,0,NULL,0);	
			}
		    else 
		   {
		   		#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}	
		break;	
		case nRF24L01_NOP:
		  /*
		   * Argument[0] 
		   *    NULL
		   *          
		   * Argument_Length=0;
		   */
		   if(Argument_Length==0x00)
		   {
			   pTemp[0]=HS6200_BANK0_STATUS;
			   pTemp[1]=0x01;		
			   pTemp[2]=nRF24L01_X_nop(DevNum);	//nop
			   USB_ACK_2_Host(DevNum,TYPE_REG,pTemp,3);	
		   }
		   else 
		   {
		   		#ifdef HS6200_DEBUG
	            	#if (DEBUG_OUT_ERR==LED_OUT_ERR)
			        	LED_Out_Err_Info(ERR_USB_PACKET_SPI_LENGTH);
					#else
		    			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}	
		break;
		default:
		{
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR) 
					LED_Out_Err_Info(ERR_USB_PACKET_SPI_CMD); 
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_CMD);
				#endif
			#endif
		}	
		break;
	}
}

/*
 * SPI 通讯协议
 *
 *  P[0]  P[1]   P[2]        P[3]     P[4]    
 *  xx     xx     xx          xx       xx         
 *  PID    Dev  PC length   CmdCode  Argument1  
 * 1Byte  1Byte  1Bytes     1Byte    0-32 Byte      
 *
 *
 */
void SPI_Protocol_Resolut(void)
{
	U8 SPI_cmd;
	U8 *SPI_Argument;
	U8 SPI_Argument_Length;
	U8 DevNum;
	DevNum=USB_Host_Out_Packet[1];
	
	SPI_cmd=USB_Host_Out_Packet[3];
	SPI_Argument=&USB_Host_Out_Packet[4];
	SPI_Argument_Length=USB_Host_Out_Packet[2]-0x01;  //minus CmdCode (1 byte)

	if(USB_Host_Out_Packet[1]==RF_DEV_0)
	{
		SPI_Mode(RF_DEV_0,SPI_cmd,SPI_Argument,SPI_Argument_Length);
	}
	else if	(USB_Host_Out_Packet[1]==RF_DEV_1)
	{
		SPI_Mode(RF_DEV_1,SPI_cmd,SPI_Argument,SPI_Argument_Length);
	}
	else
	{
	    #ifdef HS6200_DEBUG
			#if (DEBUG_OUT_ERR==LED_OUT_ERR)
				LED_Out_Err_Info(ERR_USB_PACKET_SPI_RF_DEV); 
			#else
				USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPI_RF_DEV); 
			#endif
		#endif
	}
}
/*
 *   P[0]       P[1]              P[2]              P[3]         P[4]
 * PID_SPEC	    Dev		  Packet Context Length   cmd code	   Argument
 *  0x04	DEV_0/DEV_1			  xx				 xx			  xx
 *  1Byte	   1Byte		     1Byte			   1Bytes		  xx
 *
 */
void Spec_Mode(U8 DevNum,U8 Cmd,U8 *Argument,U8 Argument_Length)
{	
	U8 pTemp[0x40];

	switch(Cmd)
	{
		case SC_RF_NOP:			//返回status寄存器
			pTemp[0]=nRF24L01_X_nop(DevNum);
			USB_ACK_2_Host(DevNum,TYPE_CMD,pTemp,1);
		break;
		case SC_TEST_REG_RST_VAL:
			 #ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif
			#endif
		break;
		case SC_TEST_REG_RW_CMP:
			 #ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif
			#endif
		break;
		case SC_POWERDOWN:
			  nRF24L01_X_powerdown(DevNum);
			  USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SC_POWERUP:
			  nRF24L01_X_powerup(DevNum);
			  USB_ACK_2_Host(DevNum,0,NULL,0);	
		break;
		case SC_STANDBYI:
			 nRF24L01_X_standby_I(DevNum);
			 USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SC_STANDBYII:
			nRF24L01_X_set_ptx(DevNum);	    	//切换到PTX模式
		  	nRF24L01_X_standby_II(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SC_CE_HIGH:
			  nRF24L01_X_ce_high(DevNum);
			  USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SC_CE_LOW:
			  nRF24L01_X_ce_low(DevNum);
			  USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SC_CE_HIGH_PULSE:
			 nRF24L01_X_ce_high_pulse(DevNum);
			 USB_ACK_2_Host(DevNum,0,NULL,0);				
		break;
		case SC_REUSE_TX_PL:
			  nRF24L01_X_reuse_tx_payload(DevNum);
			  USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SC_FLUSH_TX_FIFO:
			nRF24L01_X_flush_tx(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);	
		break;
		case SC_FLUSH_RX_FIFO:
			nRF24L01_X_flush_rx(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);				
		break;
		case SC_W_TX_PAYLOAD:
			nRF24L01_X_write_tx_payload(DevNum,Argument,Argument_Length);
			USB_ACK_2_Host(DevNum,0,NULL,0);	
		break;
		case SC_W_TX_PAYLOAD_NOACK:
			nRF24L01_X_write_tx_payload_noack(DevNum,Argument,Argument_Length);
			USB_ACK_2_Host(DevNum,0,NULL,0);	
		break;
		case SC_W_ACK_PAYLOAD:
			nRF24L01_X_write_ack_payload(DevNum,Argument[0],&Argument[1],Argument_Length-0x01);	  //minus pipe num(Argumentp[0])
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case SC_ACTIVATE:		    //not finish
			 #ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif
			#endif
		break;
		case SC_RD_PLOS_CNT:
			pTemp[0]=nRF24L01_X_get_plos_cnt(DevNum);
			USB_ACK_2_Host(DevNum,TYPE_CMD,pTemp,1);
		break;
		case SC_RD_ARC_CNT:
			pTemp[0]=nRF24L01_X_get_arc_cnt(DevNum);
			USB_ACK_2_Host(DevNum,TYPE_CMD,pTemp,1);				
		break;
		case SC_RD_RPD:
			pTemp[0]=nRF24L01_X_is_rpd(DevNum);
			USB_ACK_2_Host(DevNum,TYPE_CMD,pTemp,1);	
		break;
		case SC_RD_FIFO_STATUS:
			pTemp[0]=nRF24L01_X_read_fifo_status(DevNum);
			USB_ACK_2_Host(DevNum,TYPE_CMD,pTemp,1);
		break;
		case SC_RD_CURRENT_CONFIG_WORD:
			pTemp[0]=nRF24L01_X_read_cfg_word(DevNum);
			USB_ACK_2_Host(DevNum,TYPE_CMD,pTemp,1);
		break;
		case SC_TEST_PRESSURE:
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif
			#endif
		break;
		case SC_TEST_DATA_COMP_DPL:
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif
			#endif
		break;
		case SC_TEST_DATA_COMP_SPL:
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif
			#endif
		break;
		case SC_TEST_CONT_WAVE_MODE:
			if(Argument_Length==0x01)
			{	
			nRF24L01_X_set_cont_wave(DevNum,Argument[0]);
			USB_ACK_2_Host(DevNum,0,NULL,0);
			}
		break;
		case SC_TEST_SEARCH_DIRTY_RF_CH:
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif
			#endif
		break;
		case SC_TEST_SEARCH_CLEAN_RF_CH:
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif
			#endif
		break;	
		default: 
	   			{
	   				#ifdef HS6200_DEBUG
            			#if (DEBUG_OUT_ERR==LED_OUT_ERR)
		        			LED_Out_Err_Info(ERR_USB_PACKET_SPEC_CMD);
						#else
	    					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPEC_CMD);
						#endif	
					#endif  /*HS6200_DEBUG*/
				}	
		break;
	}
}


/*
 *   P[0]       P[1]              P[2]              P[3]         P[4]
 * PID_SPEC	    Dev		  Packet Context Length   cmd code	   Argument
 *  0x04	DEV_0/DEV_1			  xx				 xx			  xx
 *  1Byte	   1Byte		     1Byte			   1Bytes		  xx
 *
 */
void Spec_Protocol_Resolut(void)
{
	U8 Spec_Cmd;
	U8 *Spec_Argument=NULL;
	U8 Spec_Argument_Length=0x00;
        U8 DevNum;
	DevNum=USB_Host_Out_Packet[1];
	
	HS6200_Bank0_Activate(DEV_0);  //Bank0 status
	HS6200_Bank0_Activate(DEV_1);  //Bank0 status
	
	Spec_Cmd=USB_Host_Out_Packet[3];

	if(USB_Host_Out_Packet[2]>=0x01)	  
		Spec_Argument_Length=USB_Host_Out_Packet[2]-0x01;  //minus CmdCode (1 byte)

	if(Spec_Argument_Length>=0x01)				//have argument 
		Spec_Argument=&USB_Host_Out_Packet[4];    

	if(USB_Host_Out_Packet[1]==RF_DEV_0)
	{
		Spec_Mode(DEV_0,Spec_Cmd,Spec_Argument,Spec_Argument_Length);
	}
	else if	(USB_Host_Out_Packet[1]==RF_DEV_1)
	{
		Spec_Mode(DEV_1,Spec_Cmd,Spec_Argument,Spec_Argument_Length);
	}
	else
	{
		#ifdef HS6200_DEBUG
			#if (DEBUG_OUT_ERR==LED_OUT_ERR)
				LED_Out_Err_Info(ERR_USB_PACKET_SPEC_RF_DEV);
			#else
				USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SPEC_RF_DEV);
			#endif
		#endif
	}  	
}

/*----------------------------------------------09. Digital Test---------------------------------------------*/
/*
 *
 */
void Digital_Test_Mode(U8 DevNum,U8 *Cmd_Argument,U8 Cmd_Argument_Length)
{
	U8 i=0x00;
	U8 j=0x00;
	U8 Reg_Addr=0x00;
	U8 Bytes=0x00;
	U8 Reg_Val[210];   //读所有寄存器时，0x072+92=114+92=206， 这里取210
	U8 Reg_Val_Length=0x00;
	U8 Temp[5];
    U8 CH_temp=0;

	switch(Cmd_Argument[0])
	{
		case DT_SPI_TEST: 
		{
			switch(Cmd_Argument[1])
			{
				case DT_SPI_R_REGISTER:		//cmd not finish
					 #ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
						#endif
					#endif
				break;
				default:
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_DT_CMD);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DT_CMD);
						#endif
					#endif
				break;
			}
		}
		break;
			/*
			 * Cmd_Argument[0]   Cmd_Argument[1]  Cmd_Argument[2]  
			 *	DT_REG_TEST  	       xx			   ...
			 *   1Byte				 1Byte			  1Byte
			 */
		case DT_REG_TEST:
			switch(Cmd_Argument[1])
			{

			/*
			 * Cmd_Argument[0]   Cmd_Argument[1]  Cmd_Argument[2]  Cmd_Argument[3]  ...    Cmd_Argument[2n]   Cmd_Argument[2n+1]  
			 *	DT_REG_TEST  	   DT_REG_READ		   Addr			  xxBytes		...			Addr			  xxBytes
			 *   1Byte				 1Byte			  1Byte			  1Byte			...		   1Byte			   1Byte
			 *
			 * 说明：1.	Addr   :  Register Addr. for Bank1 Register:Register Address+0x80
			 *       2. xxBytes:  为读取的字节数
			 */
				case DT_REG_READ:
					i=0x00;
					Reg_Val_Length=0x00;
					while(i<(Cmd_Argument_Length-0x02))	   //minus DT_REG_TEST(1Byte) minus  DT_REG_READ(1Byte)
					{
						Reg_Addr=Cmd_Argument[2+i];
						Bytes=Cmd_Argument[3+i];   
						i+=0x02;  
						Reg_Val[Reg_Val_Length]=Reg_Addr;		   //存放寄存器地址 1Byte
						Reg_Val[Reg_Val_Length+0x01]=Bytes;	   //存放读取寄存器的宽度 1Byte

						if(	(Reg_Addr & HS6200_BANK_BIT)==HS6200_BANK1 )  //BANK1
						{
							HS6200_Bank1_Activate(DevNum);
							Reg_Addr&=~HS6200_BANK_BIT;	
						}
						else HS6200_Bank0_Activate(DevNum);		          //Bank0           
                            
						if(Bytes>0x01)	   
						{
							nRF24L01_X_read_reg_multibytes(DevNum,Reg_Addr,Temp,Bytes);  //read multibytes	 LSByte First,MSB last
							for(j=0x00;j<Bytes;j++)Reg_Val[(0x01+0x01+Reg_Val_Length)+j]=Temp[Bytes-0x01-j];	//Addr + Bytes total 2 Bytes  -->out: MSB First,LSB last 
						}
						else 
							Reg_Val[(0x01+0x01+Reg_Val_Length)]=nRF24L01_X_read_reg(DevNum,Reg_Addr); //read one byte
								
					   	Reg_Val_Length+=(0x01+0x01+Bytes);  //Addr+Bytes+(register data)Bytes
					}
					USB_ACK_2_Host(DevNum,TYPE_REG,Reg_Val,Reg_Val_Length);	
				break;
				case DT_REG_READ_ALL:	   
					Reg_Val_Length=HS6200_Read_All_Register(DevNum,Reg_Val);
					USB_ACK_2_Host(DevNum,TYPE_REG,Reg_Val,Reg_Val_Length);
				break;
				case DT_REG_WRITE:		  
					i=0x00;
					Bytes=0x00;
					while(i<(Cmd_Argument_Length-0x02))     //minus DT_REG_TEST(1Byte) minus  DT_REG_WRITE(1Byte). total 2Bytes
					{
						Reg_Addr=Cmd_Argument[0x02+i];       //register addr . 	DT_REG_TEST + DT_REG_WRITE  total 2Bytes
						Bytes=Cmd_Argument[0x02+0x01+i];	 //register data width 		DT_REG_TEST + DT_REG_WRITE+ Reg_Addr. total 3Bytes  
						for(j=0x00;j<Bytes;j++)Reg_Val[j]=Cmd_Argument[0x02+0x01+0x01+i+Bytes-0x01-j];   //register context   DT_REG_TEST + DT_REG_WRITE + register Address + Bytes  	
						if ( (Reg_Addr & HS6200_BANK_BIT) == HS6200_BANK1 )
						{
						    HS6200_Bank1_Activate(DevNum);
							Reg_Addr&=~HS6200_BANK_BIT;	
						}
						else //Bank0
                        {
                            HS6200_Bank0_Activate(DevNum);		          
						
                            if(Reg_Addr==nRF24L01_RX_ADDR_P0) for(j=0x00;j<Bytes;j++)Address_Pipe0[j]=Reg_Val[j];         //存储Pipe0地址
                        }
                        if(Bytes>0x01)			  //write multibytes
						{
							nRF24L01_X_write_reg_multibytes(DevNum,Reg_Addr,Reg_Val,Bytes);			
						}
						else					  //write one byte
						{
							 nRF24L01_X_write_reg(DevNum,Reg_Addr,Reg_Val[0]);
						}	
						i+=(0x01+0x01+Bytes);	   //(reg addr+Bytes+context)'Bytes=1Byte+1Byte+Bytes    
					}
                    if((Dev_Flag[DevNum]==MAC6200_DEV))
                    {
                       HS6200_Bank0_Activate(DevNum);
                       CH_temp = nRF24L01_X_read_reg(DEV_0, nRF24L01_RF_CH);
                       MAC6200_set_freq(2400+CH_temp); 
                    }
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case DT_REG_WRITE_ALL_0:    //cmd not finish
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
						#endif
					#endif
				break;
				case DT_REG_WRITE_ALL_1:   //cmd not finish
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
						#endif
					#endif
				break;
				case DT_REG_RESET: 	        //software reset
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
						#endif
					#endif
				break;
				case DT_REG_RST_VAL:	    //reset register value
					if(0x02==Cmd_Argument_Length)
					{
						nRF24L01_X_flush_rx(DevNum);
						nRF24L01_X_flush_tx(DevNum);
						HS6200_Rst_Bank0_All_Register(DevNum);
						HS6200_Rst_Bank1_All_Register(DevNum);
						USB_ACK_2_Host(DevNum,0,NULL,0);
					}
					else
					{
						 #ifdef HS6200_DEBUG
							#if (DEBUG_OUT_ERR==LED_OUT_ERR)
								LED_Out_Err_Info(ERR_USB_PACKET_DT_LENGTH);
							#else
								USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DT_LENGTH);
							#endif
						#endif
					}
				break;
				case DT_REG_R_VS_W:				  //cmd not finish
						 #ifdef HS6200_DEBUG
							#if (DEBUG_OUT_ERR==LED_OUT_ERR)
								LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
							#else
								USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
							#endif
						#endif
				break;
				default:
					 #ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_DT_CMD);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DT_CMD);
						#endif
					#endif
				break;
			}
		break;
		case DT_MISC_TEST:
		{
			switch(Cmd_Argument[1])
			{
				case DT_MISC_CE_HIGH:
					nRF24L01_X_ce_high(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case DT_MISC_CE_LOW:
					nRF24L01_X_ce_low(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case DT_MISC_CE_HIGH_PULSE:
					nRF24L01_X_ce_high_pulse(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case DT_MISC_SOFT_RESET:
					HS6200_Soft_Rst(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				default:
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_DT_CMD);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DT_CMD);
						#endif
					#endif
				break;
			}
		}
		break;
		case DT_PIPE_TEST:			  //cmd not finish
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
					#endif
			#endif
		break;
		default:						        //cmd err
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_DT_CMD);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DT_CMD);
				#endif
			#endif
		break;
	}	
}


/*
 *	     P[0]            P[1]            P[2]			   	   P[3]
 *	PID_Digital_Test	 Dev	  packet context length		DT_Cmd Code
 *      0x08          0x00/0x01		      xx				    xx
 *     1Byte            1Byte           1Byte			      1Byte
 *
 */

void Ditital_Test_Protocol_Resolut(void)
{
	U8 DevNum=0x00;
	U8 Cmd_Argument[MAX_OUT_PACKET_LENGTH-0x03];   //minus PID_Digital_Test(1Byte)-Dev(1Byte)-PCL(1Byte)
	U8 Cmd_Argument_Length=0x00;
	U8 i;
	Cmd_Argument_Length=USB_Host_Out_Packet[2];

	if( (Cmd_Argument_Length>=0x01) && (Cmd_Argument_Length<=MAX_OUT_PACKET_LENGTH-0x03) )   //length is err?
	{	
		if( (RF_DEV_0!=USB_Host_Out_Packet[1]) && (RF_DEV_1!=USB_Host_Out_Packet[1]) )	  //dev err
		{
			#ifdef HS6200_DEBUG
				#if(DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_DT_RF_DEV);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DT_RF_DEV);
				#endif
			#endif  /*HS6200_DEBUG*/
		}
		else																		//dev is right
		{ 
			DevNum=USB_Host_Out_Packet[1];         //get dev num               
			Cmd_Argument_Length=USB_Host_Out_Packet[2];			   //get cmd argument length
			for(i=0x00;i<Cmd_Argument_Length;i++) Cmd_Argument[i]=USB_Host_Out_Packet[3+i];	 //get cmd argument
			Digital_Test_Mode(DevNum,Cmd_Argument,Cmd_Argument_Length);
		}
	} 	
	else	     // length err
	{
		#ifdef HS6200_DEBUG
			#if(DEBUG_OUT_ERR==LED_OUT_ERR)
				LED_Out_Err_Info(ERR_USB_PACKET_DT_LENGTH);
			#else
				USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DT_LENGTH);
			#endif
		#endif  /*HS6200_DEBUG*/
	}
}

/*-----------------------------------------------Analog Test----------------------------------------------------*/
/*
 *       P[0]         P[1]            P[2]			    p[3]         p[4]
 *	PID_ANALOG_TEST	  Dev     packet context length     mode       Argument 
 *		0x09		0x00/0x01		   xx			   0x01-0x08      xx
 *      1Byte		 1Byte			  1Byte				1Byte	    n Byte
 */
void Analog_Test_Mode(U8 DevNum,U8 *Mode_Argument, U8 Mode_Argument_Length)
{
	U8 Temp[5];   //used to send data
	U8 Test_Mode=0x00;
	U8 i=0x00;

	if(Mode_Argument_Length>=0x02)
	{
		Test_Mode=Mode_Argument[0];
		switch(Test_Mode)
		{
		//------------------------1.Analog Test Test point configuration---------------------------
		case AT_TEST_PT_CFG:	             
			if(0x02==Mode_Argument_Length)
			{
				switch(Mode_Argument[1])    
				{
				case AT_TPC_NONE:
					 HS6200_Test_Disable(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_LNA_bias:
					 HS6200_Test_Point_Config_Lna_Bias(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_Mixer_CM_voltage: 
					HS6200_Test_Point_Config_Mix_CM_Vol(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_TIA_I_CM_voltage:
					HS6200_Test_Point_Config_TIA_I_CM_Vol(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_TIA_Q_CM_voltage:
					 HS6200_Test_Point_Config_TIA_Q_CM_Vol(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_TIA_output:
					 HS6200_Test_Point_Config_TIA_Output(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_1_2v_LDO_of_LO:
					 HS6200_Test_Point_Config_1V2_LDO(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_1_3v_LDO_of_LO:
					 HS6200_Test_Point_Config_1V3_LDO(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_DIV2_CM_voltage:
					 HS6200_Test_Point_Config_Div2_CM_Vol(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_LO_control_voltage:
					 HS6200_Test_Point_Config_LO_Ctrl_Vol(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_FBCLK_of_LO:
					 HS6200_Test_Point_Config_Fbclk_LO(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_Crystal_output:
					 HS6200_Test_Point_Config_Crystal_Output(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_TPC_Filter_output:
					 HS6200_Test_Point_Config_Filter_Output(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
                break;
                     
/* HS6200B1 wangs 2013-12-25*/
                case HS6200B1_AT_TPC_NONE:
					 HS6200_Test_Disable(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Fbclk_b:
                     HS6200B1_Test_Point_Config_Fbclk_b(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Crystal_clock:
                     HS6200B1_Test_Point_Config_Crystal_clock(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Old_pkdet_ip:
                     HS6200B1_Test_Point_Config_Old_pkdet_ip(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Old_pkdet_qp:
                     HS6200B1_Test_Point_Config_Old_pkdet_qp(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_New_pkdet_1:
                     HS6200B1_Test_Point_Config_New_pkdet_1(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_New_pkdet_0:
                     HS6200B1_Test_Point_Config_New_pkdet_0(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Filter_ip:
                     HS6200B1_Test_Point_Config_Fiter_ip(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Filter_in:
                     HS6200B1_Test_Point_Config_Fiter_in(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Filter_qp:
                     HS6200B1_Test_Point_Config_Fiter_qp(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Filter_qn:
                     HS6200B1_Test_Point_Config_Fiter_qn(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Mixer_ip:
                     HS6200B1_Test_Point_Config_Mixer_ip(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Mixer_in:
                     HS6200B1_Test_Point_Config_Mixer_in(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Mixer_qp:
                     HS6200B1_Test_Point_Config_Mixer_qp(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Mixer_qn:
                     HS6200B1_Test_Point_Config_Mixer_qn(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_VCO_LDO:
                     HS6200B1_Test_Point_Config_VCO_LDO(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_LO_BUF_LDO:
                     HS6200B1_Test_Point_Config_LO_BUF_LDO(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Bandgap:
                     HS6200B1_Test_Point_Config_Bnadgap(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Vctrl:
                     HS6200B1_Test_Point_Config_Vctrl(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_LNA_bias:
                     HS6200B1_Test_Point_Config_LNA_bias_B1(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Mixer_bias:
                     HS6200B1_Test_Point_Config_Mixer_bias(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Mixer_LDO:
                     HS6200B1_Test_Point_Config_Mixer_LDO(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_LNA_LDO:
                     HS6200B1_Test_Point_Config_LNA_LDO(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_Filter_LDO:
                     HS6200B1_Test_Point_Config_Filter_LDO(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                case HS6200B1_AT_TPC_XTAL_BUF_LDO:
                     HS6200B1_Test_Point_Config_XTAL_BUF_LDO(DevNum);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
/*END  wangs 2013-12-25*/                              
                
				default:
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
						#endif
					#endif
				break; 
				}
			}
			else
			{
				#ifdef HS6200_DEBUG
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_AT_LENGTH);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_LENGTH);
					#endif
				#endif
			} 		
		break;
		//--------------------------2.Analog Test frequency configuration------------------------------
		case AT_FREQ_CFG:	  
		{
		 	switch(Mode_Argument[1])
			{
			case AT_FC_SET_FREQ:
				 /*	    Mode_Argument[2]   Mode_Argument[3]    Mode_Argument[4]      Mode_Argument[5]--Mode_Argument[6]
				  * 	 integer MSByte	    integer LSByte	   fraction MSByte                 fraction LSByte			   
				  */
				 HS6200_Set_Freq(DevNum,&Mode_Argument[2],Mode_Argument_Length-0x02);   //minus Mode_Argument[0] and Mode_Argument[1], 2Bytes  
				 USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_SDM_ON:
				 HS6200_Freq_Cfg_SDM_On(DevNum);
				 USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_SDM_OFF:
				HS6200_Freq_Cfg_SDM_Off(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_DITHER_ON:
				HS6200_Freq_Cfg_SDM_Dither_On(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_DITHER_OFF:
				HS6200_Freq_Cfg_SDM_Dither_Off(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_DITHER_STAGE_1:
				HS6200_Freq_Cfg_SDM_Dither_Stage(DevNum,0x01);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_DITHER_STAGE_2:
				HS6200_Freq_Cfg_SDM_Dither_Stage(DevNum,0x02);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_FO_160K:
				HS6200_Freq_Cfg_Freq_Offset_160KHz(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_FO_250K:
				HS6200_Freq_Cfg_Freq_Offset_250KHz(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_FO_320K:
				HS6200_Freq_Cfg_Freq_Offset_320KHz(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_RF_DR_250K:
				HS6200_Data_Rate_250K(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_RF_DR_500K:
				HS6200_Data_Rate_500K(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_RF_DR_1M:
				HS6200_Data_Rate_1M(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_RF_DR_2M:
				HS6200_Data_Rate_2M(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_IF:
				if(Mode_Argument_Length==0x05)	  //中频占三个字节 + L1 Cmd(1Byte) + L2 Cmd(1Byte) = 5Bytes
				{
					for(i=0x00;i<(Mode_Argument_Length-0x02);i++) Temp[i]=Mode_Argument[2+i];
					HS6200_IF_Freq(DevNum,Temp,Mode_Argument_Length-0x02);   //minus Mode_Argument[0] and Mode_Argument[1]	 2Bytes
			    	USB_ACK_2_Host(DevNum,0,NULL,0);
				}
				else 							//length err
				{
					#ifdef HS6200_DEBUG
						#if(DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
						#endif
					#endif  /*HS6200_DEBUG*/	
				}
			break;
			case AT_FC_AB_CNT_REG:
			/*
			 * first A_Cnt register value,then B_Cnt register value
			 */
				if(Mode_Argument_Length==0x04)	//A_Cnt(1Byte)+ B_Cnt(1Byte)+ L1 Cmd + L2 Cmd = 4Bytes 
				{
					HS6200_AB_Cnt(DevNum,Mode_Argument[2],Mode_Argument[3]);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				}
				else 	   //length err
				{
					#ifdef HS6200_DEBUG
						#if(DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
						#endif
					#endif  /*HS6200_DEBUG*/	
				}
			break;
			case AT_FC_PLL_TEST_EN_ON:
				HS6200_PLL_Test_En_On(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_FC_PLL_TEST_EN_OFF:	 
			     HS6200_PLL_Test_En_Off(DevNum);
				 USB_ACK_2_Host(DevNum,0,NULL,0);
			default:
				#ifdef HS6200_DEBUG
					#if(DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
					#endif
				#endif  /*HS6200_DEBUG*/
			break;
			}
		}		
		break;
		//---------------------------3.Analog Test Rx Gain configuration-----------------------------
		/*
		 * 	mode_argument[1]    mode_argument[2]
		 *	   LNA gain             filter gain
		 *    1Byte					  1Byte
		 */
		case AT_RX_GAIN_CFG:	    //3.Rx Gain Config
			{
				switch(Mode_Argument[1])
				{
				case AT_RXGC_AGC_GAIN_MN_ON:
					HS6200_Agc_Gain_Mn_On(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_RXGC_AGC_GAIN_MN_OFF:
					HS6200_Agc_Gain_Mn_Off(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
			 	case AT_RXGC_LNA_GAIN_4dB:
					HS6200_LNA_Gain_4dB(DevNum);		    //LNA gain
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_RXGC_LNA_GAIN_16dB:
					HS6200_LNA_Gain_16dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_RXGC_LNA_GAIN_28dB:
					HS6200_LNA_Gain_28dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_RXGC_LNA_GAIN_40dB:
					HS6200_LNA_Gain_40dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
                
/*HS6200B1  wangs 2013-12-25*/
                case HS6200B1_AT_RXGC_LNA_GAIN_4dB:
					HS6200B1_LNA_Gain_4dB(DevNum);		    //HS6200B1 LNA gain
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case HS6200B1_AT_RXGC_LNA_GAIN_16dB:
					HS6200B1_LNA_Gain_16dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case HS6200B1_AT_RXGC_LNA_GAIN_28dB:
					HS6200B1_LNA_Gain_28dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case HS6200B1_AT_RXGC_LNA_GAIN_40dB:
					HS6200B1_LNA_Gain_40dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
 /*END wangs 2013-12-25*/
                
				case AT_RXGC_FILTER_GAIN_6dB:
					HS6200_Gain_Filter_6dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_RXGC_FILTER_GAIN_8dB:
					HS6200_Gain_Filter_8dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_10dB:
					HS6200_Gain_Filter_10dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_12dB:
					HS6200_Gain_Filter_12dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_14dB:
					HS6200_Gain_Filter_14dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_16dB:
					HS6200_Gain_Filter_16dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_18dB:
					HS6200_Gain_Filter_18dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_RXGC_FILTER_GAIN_20dB:
					HS6200_Gain_Filter_20dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_22dB:
					HS6200_Gain_Filter_22dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_24dB:
					HS6200_Gain_Filter_24dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 						
				case AT_RXGC_FILTER_GAIN_26dB:
					HS6200_Gain_Filter_26dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 					
				case AT_RXGC_FILTER_GAIN_28dB:
					HS6200_Gain_Filter_28dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 				 
				case AT_RXGC_FILTER_GAIN_30dB:
					HS6200_Gain_Filter_30dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);	
				break; 
				case AT_RXGC_FILTER_GAIN_32dB:
					HS6200_Gain_Filter_32dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_34dB:
					HS6200_Gain_Filter_34dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_36dB:
					HS6200_Gain_Filter_36dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_38dB:
					HS6200_Gain_Filter_38dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_40dB:
					HS6200_Gain_Filter_40dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_42dB:
					HS6200_Gain_Filter_42dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_44dB:
					HS6200_Gain_Filter_44dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				case AT_RXGC_FILTER_GAIN_46dB:
					HS6200_Gain_Filter_46dB(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break; 
				default:
					#ifdef HS6200_DEBUG
						#if(DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
						#endif
					#endif  /*HS6200_DEBUG*/
				break;
				}
			}
		break;
		//------------------------4.Analog Test Peak Detector threshold---------------------------------
		case AT_PEAK_DTT_THR:             
            switch(Mode_Argument[1])
            {
                case AT_PDT_50mV:
                    HS6200_Peak_Detector_Power_50mV(DevNum);
                    USB_ACK_2_Host(DevNum,0,NULL,0);
                break;
                case AT_PDT_100mV:
                    HS6200_Peak_Detector_Power_100mV(DevNum);
                    USB_ACK_2_Host(DevNum,NULL,0,0);
                break;
                case AT_PDT_150mV:
                    HS6200_Peak_Detector_Power_150mV(DevNum);
                    USB_ACK_2_Host(DevNum,NULL,0,0);
                break;
                case AT_PDT_200mV:
                    HS6200_Peak_Detector_Power_200mV(DevNum);
                    USB_ACK_2_Host(DevNum,0,NULL,0);
                break;
                case AT_PDT_250mV:
                    HS6200_Peak_Detector_Power_250mV(DevNum);
                    USB_ACK_2_Host(DevNum,0,NULL,0);
                break;
                case AT_PDT_300mV:
                    HS6200_Peak_Detector_Power_300mV(DevNum);
                    USB_ACK_2_Host(DevNum,0,NULL,0);
                break;
                case AT_PDT_350mV:
                    HS6200_Peak_Detector_Power_350mV(DevNum);
                    USB_ACK_2_Host(DevNum,0,NULL,0);
                break;
                case AT_PDT_400mV:
                    HS6200_Peak_Detector_Power_400mV(DevNum);
                    USB_ACK_2_Host(DevNum,0,NULL,0);
                break;
//HS6200B1   wangs 2013-12-27
                case HS6200B1_AT_PDT_AGC_SEL_OLD:
                     HS6200B1_Old_Peak_Detector_Activate(DevNum);     //Activate Old Peak Detector : agc_sel=0
                     USB_ACK_2_Host(DevNum,0,NULL,0);
                break;
                case HS6200B1_AT_PDT_AGC_SEL_NEW:
                     HS6200B1_New_Peak_Detector_Activate(DevNum);     //Activate New Peak Detector : agc_sel=1
                     USB_ACK_2_Host(DevNum,0,NULL,0);
                break;
                case HS6200B1_AT_PDT_VH_VL:                           
                     HS6200B1_New_Peak_Detector_VH_VL_Configuration(DevNum,Mode_Argument[2]);
                     USB_ACK_2_Host(DevNum,0,NULL,0);
                break;
//END wangs  2013-12-27               
                default:
                    #ifdef HS6200_DEBUG
                        #if(DEBUG_OUT_ERR==LED_OUT_ERR)
                            LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
                        #else
                            USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
                        #endif
                    #endif  /*HS6200_DEBUG*/
                break;
            }
		break;
		//---------------------------5.Analog Test PA power configuration-----------------------------
		case AT_PA_PWR_CFG:
		switch(Mode_Argument[1])
		{
		case AT_PAPC_n18dBm:
			HS6200_PA_Power_n18dBm(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case AT_PAPC_n12dBm:
			HS6200_PA_Power_n12dBm(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case AT_PAPC_n6dBm:
			HS6200_PA_Power_n6dBm(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case AT_PAPC_0dBm:
			HS6200_PA_Power_0dBm(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case AT_PAPC_5dBm:
			HS6200_PA_Power_5dBm(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
        
// HS6200B1  wangs 2013-12-27
        case HS6200B1_AT_PAPC_n18dBm:
			HS6200B1_PA_Power_n18dBm(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case HS6200B1_AT_PAPC_n12dBm:
			HS6200B1_PA_Power_n12dBm(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case HS6200B1_AT_PAPC_n6dBm:
			HS6200B1_PA_Power_n6dBm(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case HS6200B1_AT_PAPC_0dBm:
			HS6200B1_PA_Power_0dBm(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
		case HS6200B1_AT_PAPC_5dBm:
			HS6200B1_PA_Power_5dBm(DevNum);
			USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
        case HS6200B1_AT_PAPC_PA_VOLTAGE_1V8:
            HS6200B1_PA_Power_Voltage_1V8(DevNum);
            USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
        case HS6200B1_AT_PAPC_PA_VOLTAGE_3V:
            HS6200B1_PA_Power_Voltage_3V(DevNum);
            USB_ACK_2_Host(DevNum,0,NULL,0);
		break;
//END wangs 2013-12-27

		default:
			#ifdef HS6200_DEBUG
				#if(DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
				#endif
			#endif  /*HS6200_DEBUG*/
		break; 
		}		
		break;
		//-----------------------------6. Analog Test Calibration manual mode----------------------------- 
		/*
		 * Mode_Argument[1]	    Mode_Argument[2]
		 * 
		 *   1Byte			    	1Byte
		 */
		case AT_CAL_MAN_MODE:
			 switch(Mode_Argument[1])
			 {
			 	case AT_CMM_CTUNING_MN_ON:
					 HS6200_VCO_Ctuning_Calb_Mn_On(DevNum);
					 USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CMM_CTUNING_MN_OFF:
					HS6200_VCO_Ctuning_Calb_Mn_Off(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case  AT_CMM_FTUNING_MN_ON:
					HS6200_VCO_Ftuning_Calb_Mn_On(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CMM_FTUNING_MN_OFF:
					HS6200_VCO_Ftuning_Calb_Mn_Off(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CMM_VCO_LDO_CAL_MN_ON:
					HS6200_VCO_LDO_Calb_Mn_On(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CMM_VCO_LDO_CAL_MN_OFF:
					HS6200_VCO_LDO_Calb_Mn_Off(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CMM_DAC_RANGE_MN_ON:
					HS6200_DAC_Range_Mn_On(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CMM_DAC_RANGE_MN_OFF:
					HS6200_DAC_Range_Mn_Off(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CMM_DOC_DAC_MN_ON:
					HS6200_DOC_DAC_Mn_On(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CMM_DOC_DAC_MN_OFF:
					HS6200_DOC_DAC_Mn_On(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CMM_CARRIER_CAL_MN:
					if(0x03==Mode_Argument_Length)	    //3Byte length=L1 Cmd + L2 Cmd + Argument
					{
						HS6200_Carrier_Calb(DevNum,Mode_Argument[2]);
						USB_ACK_2_Host(DevNum,0,NULL,0);
					}
					else if(0x02==Mode_Argument_Length)	 //2Byte length=L1 Cmd + L2 Cmd 
					{
						HS6200_Calibration(DevNum);
					}
					else 
					{
						#ifdef HS6200_DEBUG
							#if(DEBUG_OUT_ERR==LED_OUT_ERR)
								LED_Out_Err_Info(ERR_USB_PACKET_AT_LENGTH);
							#else
								USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_LENGTH);
							#endif
						#endif  /*HS6200_DEBUG*/					
					} 
				break;
				case AT_CMM_CTUNING_REG_R:
					 Temp[0]=HS6200_Read_VCO_Ctuning(DevNum);
					 USB_ACK_2_Host(DevNum,TYPE_CMD,Temp,1);
				break;
				case AT_CMM_CTUNING_REG_W:
					if(0x03==Mode_Argument_Length)	 //L1Cmd(1Byte)+L2Cmd(1Byte)+set argument(1Byte)=3Bytes
					{
						HS6200_Write_VCO_Ctuning(DevNum,Mode_Argument[2]);
						USB_ACK_2_Host(DevNum,0,NULL,0);
					}
					else
					{
						#ifdef HS6200_DEBUG
							#if(DEBUG_OUT_ERR==LED_OUT_ERR)
								LED_Out_Err_Info(ERR_USB_PACKET_AT_LENGTH);
							#else
								USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_LENGTH);
							#endif
						#endif  /*HS6200_DEBUG*/	
					}
				break;
				case AT_CMM_FTUNING_REG_R:
					Temp[0]=HS6200_Read_VCO_Ftuning(DevNum);
					USB_ACK_2_Host(DevNum,TYPE_CMD,Temp,1);
				break;
				case AT_CMM_FTUNING_REG_W:
					if(0x03==Mode_Argument_Length)	  //L1Cmd(1Byte)+L2Cmd(1Byte)+set argument(1Byte)=3Bytes
					{
						HS6200_Write_VCO_Ftuning(DevNum,Mode_Argument[2]);
						USB_ACK_2_Host(DevNum,0,NULL,0);
					}
					else			//length err
					{
						#ifdef HS6200_DEBUG
							#if(DEBUG_OUT_ERR==LED_OUT_ERR)
								LED_Out_Err_Info(ERR_USB_PACKET_AT_LENGTH);
							#else
								USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_LENGTH);
							#endif
						#endif  /*HS6200_DEBUG*/	
					}
				break;
				case AT_CMM_VCO_LDO_CAL_REG_R:
					Temp[0]=HS6200_Read_VCO_LDO_Calb(DevNum);
					USB_ACK_2_Host(DevNum,TYPE_CMD,Temp,1);
				break;
				case AT_CMM_VCO_LDO_CAL_REG_W:
					if(0x03==Mode_Argument_Length) //L1Cmd(1Byte)+L2Cmd(1Byte)+set argument(1Byte)=3Bytes
					{
						HS6200_Write_VCO_LDO_Calb(DevNum,Mode_Argument[2]);
						USB_ACK_2_Host(DevNum,0,NULL,0);
					}
					else
					{
						#ifdef HS6200_DEBUG
							#if(DEBUG_OUT_ERR==LED_OUT_ERR)
								LED_Out_Err_Info(ERR_USB_PACKET_AT_LENGTH);
							#else
								USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_LENGTH);
							#endif
						#endif  /*HS6200_DEBUG*/	
					}
				break;
				case AT_CMM_DAC_RANGE_R:
					Temp[0]=HS6200_Read_DAC_Gain(DevNum);
					USB_ACK_2_Host(DevNum,TYPE_CMD,Temp,1);
				break;
				case AT_CMM_DAC_RANGE_W:
					if(0x03==Mode_Argument_Length) //L1Cmd(1Byte)+L2Cmd(1Byte)+set argument(1Byte)=3Bytes
					{
						HS6200_Write_DAC_Gain(DevNum,Mode_Argument[2]);
						USB_ACK_2_Host(DevNum,0,NULL,0);
					}
					else
					{
						#ifdef HS6200_DEBUG
							#if(DEBUG_OUT_ERR==LED_OUT_ERR)
								LED_Out_Err_Info(ERR_USB_PACKET_AT_LENGTH);
							#else
								USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_LENGTH);
							#endif
						#endif  /*HS6200_DEBUG*/	
					}
				break;
				case AT_CMM_DOC_DACI_R:
					Temp[0]=HS6200_Read_DC_Offset_I(DevNum);
					USB_ACK_2_Host(DevNum,TYPE_CMD,Temp,1);
				break;
				case AT_CMM_DOC_DACI_W:
					if(0x03==Mode_Argument_Length) //L1Cmd(1Byte)+L2Cmd(1Byte)+set argument(1Byte)=3Bytes
					{
						HS6200_Write_DC_Offset_I(DevNum,Mode_Argument[2]);
						USB_ACK_2_Host(DevNum,0,NULL,0);
					}
					else
					{
						#ifdef HS6200_DEBUG
							#if(DEBUG_OUT_ERR==LED_OUT_ERR)
								LED_Out_Err_Info(ERR_USB_PACKET_AT_LENGTH);
							#else
								USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_LENGTH);
							#endif
						#endif  /*HS6200_DEBUG*/
					}
				break;
				case AT_CMM_DOC_DACQ_R:
					Temp[0]=HS6200_Read_DC_Offset_Q(DevNum);
					USB_ACK_2_Host(DevNum,TYPE_CMD,Temp,1);
				break;
				case AT_CMM_DOC_DACQ_W:
					if(0x03==Mode_Argument_Length) //L1Cmd(1Byte)+L2Cmd(1Byte)+set argument(1Byte)=3Bytes
					{
						HS6200_Write_DC_Offset_Q(DevNum,Mode_Argument[2]);
						USB_ACK_2_Host(DevNum,0,NULL,0);
					}
					else
					{
						#ifdef HS6200_DEBUG
							#if(DEBUG_OUT_ERR==LED_OUT_ERR)
								LED_Out_Err_Info(ERR_USB_PACKET_AT_LENGTH);
							#else
								USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_LENGTH);
							#endif
						#endif  /*HS6200_DEBUG*/
					}
				break;
				default:
					#ifdef HS6200_DEBUG
						#if(DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
						#endif
					#endif  /*HS6200_DEBUG*/
				break;
			 }
		break;
	
		//--------------------7.Analog Test Continuous carrier Transmit------------------------- 
		/*
		 * Mode_Argument[1]    
		 *
		 */
		case AT_CONT_CARRIER_TX:
			switch(Mode_Argument[1])
			{
				case AT_CCT_CONT_WAVE_START:
					HS6200_Cont_Wave_Start(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_CONT_WAVE_STOP:
					HS6200_Cont_Wave_Stop(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);   
				break;
				case AT_CCT_PLL_DAC_ON:
					HS6200_PLL_DAC_On(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_PLL_DAC_OFF:
					HS6200_PLL_DAC_Off(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_BP_CP_DIO_ON:
					HS6200_Bp_Cp_Diox_On(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_BP_CP_DIO_OFF:
					HS6200_Bp_Cp_Diox_Off(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_VC_DET_ON:
					HS6200_Vc_Det_On(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_VC_DET_OFF:
					HS6200_Vc_Det_Off(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_PLL_ICP_SEL_80uA:
					HS6200_PLL_Icp_Sel_80uA(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_PLL_ICP_SEL_120uA:
					HS6200_PLL_Icp_Sel_120uA(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_PLL_ICP_SEL_160uA:
					HS6200_PLL_Icp_Sel_160uA(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_PLL_ICP_SEL_200uA:
					HS6200_PLL_Icp_Sel_200uA(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;									
				case AT_CCT_PLL_VDIV2_SEL_550mV:
					HS6200_PLL_Vdiv2_Sel_550mV(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_PLL_VDIV2_SEL_600mV:
					HS6200_PLL_Vdiv2_Sel_600mV(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_PLL_VDIV2_SEL_650mV:
					HS6200_PLL_Vdiv2_Sel_650mV(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case AT_CCT_PLL_VDIV2_SEL_700mV:
					HS6200_PLL_Vdiv2_Sel_700mV(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				default:
					#ifdef HS6200_DEBUG
						#if(DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
						#endif
					#endif  /*HS6200_DEBUG*/
				break; 
			}	
		break;
		//-----------------------8.continuous test partten tx -----------------------------
		case AT_CONT_TESTPAT_TX:     
		{
			switch(Mode_Argument[1])
			{
			case AT_CTT_TESTPAT_START:
				HS6200_Cont_Test_Pat_Tx_Start(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_CTT_TESTPAT_STOP:
				HS6200_Cont_Test_Pat_Tx_Stop(DevNum);
				USB_ACK_2_Host(DevNum,0,NULL,0);
			break;
			case AT_CTT_TESTPAT:
				if(Mode_Argument_Length==0x03) //L1Cmd(1Byte)+L2Cmd(1Byte)+set argument(1Byte)=3Bytes
				{
					HS6200_Cont_Test_Pat(DevNum,Mode_Argument[2]);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				}
				else 
				{
					 #ifdef HS6200_DEBUG
						#if(DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_AT_LENGTH);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_LENGTH);
						#endif
					#endif  /*HS6200_DEBUG*/
				}
			break;
			case AT_CTT_TESTPAT_PN:
				USB_ACK_2_Host(DevNum,0,NULL,0);	//AT_CTT_TEST_PN 的ACK
				USB_Host_Out_Flag=USB_HOST_OUT_FLAG_C8051F_USED;   //置标志位，可以接收AT_CTT_TESTPAT_STOP命令
				HS6200_Test_Pattern_Pseudo_Random_Num(DevNum);
				HS6200_Cont_Test_Pat_Tx_Stop(DevNum);   //发送停止命令后，才能执行到此
				USB_ACK_2_Host(DevNum,0,NULL,0);	//AT_CTT_TESTPAT_STOP的ACK
			break;
			default:
				#ifdef HS6200_DEBUG
					#if(DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
					#endif
				#endif  /*HS6200_DEBUG*/
			break; 
			}
		}
		break;
		default:
			#ifdef HS6200_DEBUG
				#if(DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
				#endif
			#endif  /*HS6200_DEBUG*/
		break;
		}
	}
	else
	{
		#ifdef HS6200_DEBUG
			#if(DEBUG_OUT_ERR==LED_OUT_ERR)
				LED_Out_Err_Info(ERR_USB_PACKET_AT_LENGTH);
			#else
				USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_LENGTH);
			#endif
		#endif  /*HS6200_DEBUG*/
	}
}
/*
 *       P[0]         P[1]            P[2]			    p[3]         p[4]
 *	PID_ANALOG_TEST	  Dev     packet context length     mode       Argument 
 *		0x09		0x00/0x01		   xx			   0x01-0x08      xx
 *      1Byte		 1Byte			  1Byte				1Byte	    n Byte
 */
void Analog_Test_Protocol_Resolut(void)
{	
	U8 Mode_Argument[10];
	U8 Mode_Argument_Length=0x00;
	U8 DevNum;
	U8 i=0x00;
	DevNum=USB_Host_Out_Packet[1];

	if( (DevNum==DEV_0) || (DevNum==DEV_1)  ) //dev right
	{
		Mode_Argument_Length=USB_Host_Out_Packet[2];
		if(Mode_Argument_Length<10)
		{
	    	for(i=0x00;i<Mode_Argument_Length;i++)Mode_Argument[i]=USB_Host_Out_Packet[3+i];
			if(Mode_Argument[0]<=0x08)Analog_Test_Mode(DevNum,Mode_Argument,Mode_Argument_Length);   //analog test 8 comand code totle
			else 				   //cmd code err
			{
				#ifdef HS6200_DEBUG
	    			#if (DEBUG_OUT_ERR==LED_OUT_ERR)
	        			LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
					#endif	
				#endif  /*HS6200_DEBUG*/
			}
		}
		else 			  //length err
		{
				#ifdef HS6200_DEBUG
	    			#if (DEBUG_OUT_ERR==LED_OUT_ERR)
	        			LED_Out_Err_Info(ERR_USB_PACKET_AT_LENGTH);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/	
		}
	}
	else	    //dev err
	{
	 	#ifdef HS6200_DEBUG
			#if (DEBUG_OUT_ERR==LED_OUT_ERR)
    			LED_Out_Err_Info(ERR_USB_PACKET_AT_RF_DEV);
			#else
				USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_RF_DEV);
			#endif	
		#endif  /*HS6200_DEBUG*/	
	}	
}
void System_Protocol_Resolut(void)	    //没有实现
{
	U8 DevNum;
	DevNum=USB_Host_Out_Packet[1];
	#ifdef HS6200_DEBUG
		#if (DEBUG_OUT_ERR==LED_OUT_ERR)
    		LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
		#else
			USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
		#endif	
	#endif  /*HS6200_DEBUG*/		
}

void Set_Mode(U8 DevNum,U8 *Cmd_Argument,U8 Cmd_Argument_Length)
{
	U8 Temp[5];
	U8 i;
	U8 Addr_Width;
	Cmd_Argument_Length=Cmd_Argument_Length;
	switch(Cmd_Argument[0])
	{
		case  TYPE_REG :     //not finish
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif	
	        #endif  /*HS6200_DEBUG*/		
		break;
		case TYPE_PLD:      //not finish
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif	
			#endif  /*HS6200_DEBUG*/		
		break;
		case TYPE_SDS:      
			switch(Cmd_Argument[1])
			{
				case SDS_CONFIG:
					HS6200_Bank0_Activate(DevNum);
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_CONFIG,Cmd_Argument[2]); 	
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_SETUP_RETR,Cmd_Argument[3]);
				    nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_CH,Cmd_Argument[4]);
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Cmd_Argument[5]);
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FEATURE,Cmd_Argument[6]);
					
					if( Cmd_Argument[7] & BIT7 )ce_low_befor_write=1;
					else ce_low_befor_write=0;
					if( Cmd_Argument[7] & BIT6) Cal_after_Ack=1;
					else Cal_after_Ack=0;
					if(Cmd_Argument[7] & BIT5) flush_tx_when_max_rt=1;
					else flush_tx_when_max_rt=0;
					if( Cmd_Argument[7]& BIT4) nRF24L01_X_ce_high(DevNum);
					else nRF24L01_X_ce_low(DevNum);	
                    if( Cmd_Argument[7] & BIT3)Tx_By_CE_High=1;
					else Tx_By_CE_High=0x00;
					if(Cmd_Argument[7]&BIT2)Nop_After_W_Tx_Payload=0x01;
					else Nop_After_W_Tx_Payload=0x00;
								
//					if (Cmd_Argument[8] & BIT7) nRF24L01_1_EI=0;
//				    else if(Dev_Flag[DEV_1]==DEV_NONE)	  // 如果是DEV_NONE 则不能打开xx_EI，因为打开后，程序进入中断，就会死在中断里
//						nRF24L01_1_EI=0;
//					else nRF24L01_1_EI=1;
//					if(Cmd_Argument[8] & BIT6)	nRF24L01_0_EI=0;
//					else if(Dev_Flag[DEV_0]==DEV_NONE)
//						nRF24L01_0_EI=0;
//					else nRF24L01_0_EI=1;
					 
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_SETUP_AW,Cmd_Argument[9]);
                   
					switch(Cmd_Argument[9])
					{
						case nRF24L01_AW_5_BYTES: 
							Addr_Width=0x05;
							for(i=0x00;i<Addr_Width;i++)Temp[i]=Cmd_Argument[10+Addr_Width-0x01-i]; 
							nRF24L01_X_write_pipe_addr(DevNum,HS6200_BANK0_TX_ADDR,Temp,Addr_Width);
						break;
						case nRF24L01_AW_4_BYTES:
							Addr_Width=0x04;
							for(i=0x00;i<Addr_Width;i++)Temp[i]=Cmd_Argument[10+Addr_Width-0x01-i]; 
							nRF24L01_X_write_pipe_addr(DevNum,HS6200_BANK0_TX_ADDR,Temp,Addr_Width);
						break;
						case nRF24L01_AW_3_BYTES:
							Addr_Width=0x03;
						    for(i=0x00;i<Addr_Width;i++)Temp[i]=Cmd_Argument[10+Addr_Width-0x01-i]; 
							nRF24L01_X_write_pipe_addr(DevNum,HS6200_BANK0_TX_ADDR,Temp,Addr_Width); 
						break;
						default:
							#ifdef HS6200_DEBUG
								#if (DEBUG_OUT_ERR==LED_OUT_ERR)
									LED_Out_Err_Info(ERR_USB_PACKET_SET_DATA);
								#else
									USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SET_DATA);
								#endif	
							#endif  /*HS6200_DEBUG*/				
						break;
					}									
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_RXADDR,Cmd_Argument[15]);
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_AA,Cmd_Argument[16]);
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_DYNPD,Cmd_Argument[17]);
					
					
					for(i=0x00;i<Addr_Width;i++)
                    {
                        Temp[i]=Cmd_Argument[18+Addr_Width-0x01-i]; 
                        Address_Pipe0[i]=Temp[i];  //存储Pipe0
                    }
					nRF24L01_X_write_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0,Temp,Addr_Width);
                    
					
					if(Dev_Flag[DevNum]==nRF24L01_DEV)                  //nRF24L01 dev
					{
						for(i=0x00;i<Addr_Width;i++)Temp[i]=Cmd_Argument[23+Addr_Width-0x01-i]; 
						nRF24L01_X_write_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P1,Temp,Addr_Width);
					}	
					else if(Dev_Flag[DevNum]==HS6200_DEV)               //HS6200 
					{
						nRF24L01_X_write_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P1,&Cmd_Argument[23],1);
					}
					nRF24L01_X_write_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P2,&Cmd_Argument[28],1);
					nRF24L01_X_write_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P3,&Cmd_Argument[29],1);
					nRF24L01_X_write_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P4,&Cmd_Argument[30],1);
					nRF24L01_X_write_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P5,&Cmd_Argument[31],1);
					
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P0,Cmd_Argument[32]);
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P1,Cmd_Argument[33]);
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P2,Cmd_Argument[34]);
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P3,Cmd_Argument[35]);
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P4,Cmd_Argument[36]);
					nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P5,Cmd_Argument[37]);
					
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case SDS_MCU:    //not finish
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
						#endif	
					#endif  /*HS6200_DEBUG*/						
				break;
				case SDS_FLUSH_TX_WHEN_MAX_RT:
					if(Cmd_Argument[2])	flush_tx_when_max_rt=0x01;
					else flush_tx_when_max_rt=0x00;
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case SDS_TRANSMIT_BY_CE_HIGH:   //发送方式
					if(Cmd_Argument[2])Tx_By_CE_High=0x01;   //CE为高发送
					else Tx_By_CE_High=0x00;                 //CE为低发送
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case SDS_NOP_AFTER_W_TX_PAYLOAD:
					if(Cmd_Argument[2])Nop_After_W_Tx_Payload=0x01;
					else Nop_After_W_Tx_Payload=0x00;
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;	
				default:
				break;
			} 
		break;
		case TYPE_CMD:
			switch(Cmd_Argument[1])
			{
				case CMD_MANUAL_RETR:
					nRF24L01_X_reuse_tx_payload(DevNum);
					if(Cal_after_Ack)nRF24L01_X_ce_high(DevNum);
					else nRF24L01_X_ce_high_pulse(DevNum);
				    USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				case CMD_RF_RESET:
					HS6200_Rst_Bank0_All_Register(DevNum);
					if(Dev_Flag[DevNum]==HS6200_DEV)HS6200_Rst_Bank1_All_Register(DevNum);
					USB_ACK_2_Host(DevNum,0,NULL,0);
				break;
				default:
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
						#endif	
					#endif  /*HS6200_DEBUG*/	
				break;
			}
		break;
		default:
			#ifdef HS6200_DEBUG 
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_SET_CMD);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SET_CMD);
					#endif	
			#endif  /*HS6200_DEBUG*/	
		break;
	}
	
}
/*
 *    P[0]       P[1]                P[2]             P[3]    P[4]     P[5..n]
 *    PID       DevNum       Packet context length    Type    Cmd      Argument
 *  PID_SET    DEV_0/DEV_1            xx               xx      xx         xx
 *   1Byte       1Byte              1Byte             1Byte   1Byte     nBytes
 */
void Set_Protocol_Resolut(void)
{
	U8 DevNum;
	U8 Cmd_Argument[0x28];  
	U8 Cmd_Argument_Length;
	U8 i;
	DevNum=USB_Host_Out_Packet[1];
	Cmd_Argument_Length=USB_Host_Out_Packet[2];
	
	if( (DevNum==RF_DEV_0) || (DevNum==RF_DEV_1) )
	{
			if( (Cmd_Argument_Length<= ( sizeof(Cmd_Argument)/sizeof(Cmd_Argument[0]) ) ) && (Cmd_Argument_Length>=0x01) )
			{
				for(i=0x00;i<Cmd_Argument_Length;i++)Cmd_Argument[i]=USB_Host_Out_Packet[3+i];
				Set_Mode(DevNum,Cmd_Argument,Cmd_Argument_Length);
			}
			else    //length err
			{
				#ifdef HS6200_DEBUG 
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_SET_LENGTH);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SET_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/	
			}
	}
	else   //RF_DEV err
	{
		#ifdef HS6200_DEBUG
			#if (DEBUG_OUT_ERR==LED_OUT_ERR)
    			LED_Out_Err_Info(ERR_USB_PACKET_SET_RF_DEV);
			#else
				USB_Out_Err_Info(DevNum,ERR_USB_PACKET_SET_RF_DEV);
			#endif	
		#endif  /*HS6200_DEBUG*/	
	}
}


void Get_Mode(U8 DevNum,U8 *Cmd_Argument,U8 Cmd_Argument_Length)
{
	SDS_CONFIG_t Config;
	U8 i;
	U8 Temp[5];
	U8 Addr_Width;
	U8 pTemp[0x30];
	switch(Cmd_Argument[0])
	{
		case TYPE_REG:   //not finish
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif	
	        #endif  /*HS6200_DEBUG*/	
		break;
		case TYPE_PLD:   //not finish
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
				#endif	
	        #endif  /*HS6200_DEBUG*/	
		break;
		case TYPE_SDS :
			if(Cmd_Argument_Length==0x02)
			{
				switch(Cmd_Argument[1])
				{
					case SDS_CONFIG:
						HS6200_Bank0_Activate(DevNum);	
						Config.config=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_CONFIG); 
						Config.setup_retr=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_SETUP_RETR); 
						Config.rf_ch=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RF_CH); 
						Config.rf_setup=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RF_SETUP); 
						Config.feature=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_FEATURE); 
						
						Config.other0=0x00;
						if(ce_low_befor_write==1) Config.other0 |= BIT7;
						else Config.other0 &=~BIT7;
						if(Cal_after_Ack==1) Config.other0 |= BIT6;
						else Config.other0 &=~ BIT6;
						if(flush_tx_when_max_rt==0x01)Config.other0 |= BIT5;
						else Config.other0 &= ~BIT5;
						if(nRF24L01_X_is_ce_high(DevNum))Config.other0 |=BIT4;
						else Config.other0 &=~BIT4;	
						if(Tx_By_CE_High)Config.other0 |=BIT3;
						else Config.other0 &=~BIT3;						
						if(Nop_After_W_Tx_Payload) Config.other0 |=BIT2;
						else Config.other0 &=~BIT2;
							
//						Config.other1=0x00;
//						if(nRF24L01_1_EI==0) Config.other1 |= BIT7;
//						else Config.other1 &= ~BIT7;
//						if(nRF24L01_0_EI==0) Config.other1 |= BIT6;
//						else Config.other1 &= ~BIT6;

						Config.setup_aw=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_SETUP_AW); 
						switch( (Config.setup_aw) & (BIT0+BIT1) )
						{
							case nRF24L01_AW_5_BYTES:   
								Addr_Width=0x05;
							break;
							case nRF24L01_AW_4_BYTES:
								Addr_Width=0x04;
							break;
							case nRF24L01_AW_3_BYTES:
								Addr_Width=0x03;		
							break;
							default:
							break;
						}
						for(i=0x00;i<0x05;i++)Temp[i]=0x00;  //clear Temp,当读取的字节数不足5字节时，后面补0，回应主机，这主要影响到TxAddr,Pipe0,Pipe1。
						nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_TX_ADDR,Temp,Addr_Width); 					
						for(i=0x00;i<Addr_Width;i++)Config.tx_addr[i]=Temp[Addr_Width-0x01-i];
						Config.en_rxaddr=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_EN_RXADDR);
						Config.en_aa=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_EN_AA);
						Config.dynpd=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_DYNPD);

						for(i=0x00;i<0x05;i++)Temp[i]=0x00;  //clear Temp
						nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P0,Temp,Addr_Width); 					
						for(i=0x00;i<Addr_Width;i++)Config.rx_addr_p0[i]=Temp[Addr_Width-0x01-i];

						for(i=0x00;i<0x05;i++)Temp[i]=0x00;  //clear Temp
						if(Dev_Flag[DevNum]==nRF24L01_DEV)
						{
							nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P1,Temp,Addr_Width); 					
							for(i=0x00;i<Addr_Width;i++)Config.rx_addr_p1[i]=Temp[Addr_Width-0x01-i];
						}
						else   //HS6200 dev
						{
							nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P1,Temp,1); 					
							Config.rx_addr_p1[0]=Temp[0];
						}
						nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P2,Temp,1);
						Config.rx_addr_p2=Temp[0];		
						nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P3,Temp,1); 	
						Config.rx_addr_p3=Temp[0];
						nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P4,Temp,1); 	
						Config.rx_addr_p4=Temp[0];
						nRF24L01_X_read_pipe_addr(DevNum,HS6200_BANK0_RX_ADDR_P5,Temp,1); 	
						Config.rx_addr_p5=Temp[0];
						Config.rx_pw_p0=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RX_PW_P0);
						Config.rx_pw_p1=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RX_PW_P1);
						Config.rx_pw_p2=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RX_PW_P2);
						Config.rx_pw_p3=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RX_PW_P3);
						Config.rx_pw_p4=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RX_PW_P4);
						Config.rx_pw_p5=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RX_PW_P5);
						
						pTemp[0]=SDS_CONFIG;
						for(i=0x00;i<sizeof(Config);i++)pTemp[1+i]=((U8 *)&Config)[i];

						USB_ACK_2_Host(DevNum,TYPE_SDS,pTemp, sizeof(Config)+1);
				break;
				case SDS_MCU:
					#ifdef HS6200_DEBUG
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_FUN_NOT_FINSH);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_FUN_NOT_FINSH);
						#endif	
					#endif  /*HS6200_DEBUG*/	
				break;
				case SDS_FIRMWARE:   //回应日期号
					 USB_ACK_2_Host(DevNum,TYPE_DBG, SoftWare_Date,0x06);
				break;
				default:
					#ifdef HS6200_DEBUG 
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_GET_CMD);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_GET_CMD);
						#endif	
					#endif  /*HS6200_DEBUG*/	
				break;
			}
		}
		else
		{
			#ifdef HS6200_DEBUG
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_GET_LENGTH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_GET_LENGTH);
				#endif	
			#endif  /*HS6200_DEBUG*/	
		}
		break;
		default:
			#ifdef HS6200_DEBUG 
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_GET_CMD);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_GET_CMD);
				#endif	
			#endif  /*HS6200_DEBUG*/	
		break;	
	}	
}

/*
 *    P[0]       P[1]                P[2]             P[3]    P[4]     P[5..n]
 *    PID       DevNum       Packet context length    Type    Cmd      Argument
 *  PID_GET    DEV_0/DEV_1            xx               xx      xx         xx
 *   1Byte       1Byte               1Byte            1Byte   1Byte     nBytes
 */

void Get_Protocol_Resolut(void)
{
	U8 DevNum;
	U8 Cmd_Argument[10];  //Cmd argument 长度暂定为10
	U8 Cmd_Argument_Length;
	U8 i;
	DevNum=USB_Host_Out_Packet[1];
	Cmd_Argument_Length=USB_Host_Out_Packet[2];
	
	if( (DevNum==RF_DEV_0) || (DevNum==RF_DEV_1) )
	{
			if( (Cmd_Argument_Length<=sizeof(Cmd_Argument) ) && (Cmd_Argument_Length>=0x01) )
			{
				for(i=0x00;i<Cmd_Argument_Length;i++)Cmd_Argument[i]=USB_Host_Out_Packet[3+i];
				Get_Mode(DevNum,Cmd_Argument,Cmd_Argument_Length);
			}
			else    //length err
			{
				#ifdef HS6200_DEBUG 
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_GET_LENGTH);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_GET_LENGTH);
					#endif	
				#endif  /*HS6200_DEBUG*/	
			}
	}
	else   //RF_DEV err
	{
		#ifdef HS6200_DEBUG
			#if (DEBUG_OUT_ERR==LED_OUT_ERR)
    			LED_Out_Err_Info(ERR_USB_PACKET_GET_RF_DEV);
			#else
				USB_Out_Err_Info(DevNum,ERR_USB_PACKET_GET_RF_DEV);
			#endif	
		#endif  /*HS6200_DEBUG*/	
	}
}


void Debug_Mode(U8 DevNum,U8 *Cmd_Argument,U8 Cmd_Argument_Length)
{
	U8 Temp[10];
	U8 *p_xAddr;
	U8 *p_iAddr;
	U8 data_read[32];   //最大到32Byte
	U8 i;
	
	Cmd_Argument_Length=Cmd_Argument_Length;
	switch(Cmd_Argument[0])
	{
		case DEBUG_SOFTWARE_DATE:					   //0x01
//			USB_ACK_2_Host(DevNum,TYPE_DBG,SoftWare_Date,0x06);
		break;
		case DEBUG_SOFTWARE_VER:						   //0x02
//		    USB_ACK_2_Host(DevNum,TYPE_DBG,&SoftWare_Ver,0x01);
		break;
		case DEBUG_DEVICE_FLG:						   //0x03
			if(Cmd_Argument_Length==0x01)
			{
				if(Dev_Flag[DEV_0]==HS6200_DEV) 	//DEV_0 Info     
				{
					Temp[0]=0x62;
					Temp[1]=0x00;
				}
				else if(Dev_Flag[DEV_0]==nRF24L01_DEV)
				{
					Temp[0]=0x24;
					Temp[1]=0x01;	
				}
				else
				{
					Temp[0]=0x00;
					Temp[1]=0x00;
				}
				if(Dev_Flag[DEV_1]==HS6200_DEV)      //DEV_1 Info
				{
					Temp[2]=0x62;
					Temp[3]=0x00;
				}
				else if(Dev_Flag[DEV_1]==nRF24L01_DEV)
				{
					Temp[2]=0x24;
					Temp[3]=0x01;	
				}
				else
				{
					Temp[2]=0x00;
					Temp[3]=0x00;
				}
				USB_ACK_2_Host(DevNum,TYPE_DBG,Temp,4);
			}
			else if(Cmd_Argument_Length==0x03)   //2 dev
			{
				if(Cmd_Argument[1]==0x00)Dev_Flag[DEV_0]=DEV_NONE;
				else if(Cmd_Argument[1]==0x01) Dev_Flag[DEV_0]=nRF24L01_DEV;
				else if(Cmd_Argument[1]==0x02) Dev_Flag[DEV_0]=HS6200_DEV;
				if(Cmd_Argument[2]==0x00)Dev_Flag[DEV_1]=DEV_NONE;
				else if(Cmd_Argument[2]==0x01) Dev_Flag[DEV_1]=nRF24L01_DEV;
				else if(Cmd_Argument[2]==0x02) Dev_Flag[DEV_1]=HS6200_DEV;
				USB_ACK_2_Host(DevNum,0,NULL,0);
			}
		break;	
		case DEBUG_MCU_INT:       //0x04  中断情况
//			Temp[0]=nRF24L01_0_EI;
//			Temp[1]=nRF24L01_1_EI;
//			Temp[2]=EA;
			USB_ACK_2_Host(DevNum,TYPE_DBG,Temp,3);
		break;	
        case DEBUG_DEV_CE:	       //0x05
			Temp[0]=nRF24L01_X_is_ce_high(DEV_0);
			Temp[1]=nRF24L01_X_is_ce_high(DEV_1); 
		    USB_ACK_2_Host(DevNum,TYPE_DBG,Temp,2);
		break;		
		case DEBUG_CAL_AFTER_ACK:	//0x06
			Temp[0]=Cal_after_Ack;
		    USB_ACK_2_Host(DevNum,TYPE_DBG,Temp,1);
		break;
		case DEBUG_CE_LOW_BEFORE_WRITE:	  //0x07
			Temp[0]=ce_low_befor_write;
		    USB_ACK_2_Host(DevNum,TYPE_DBG,Temp,1);			
		break;
		case DEBUG_DEV_BUSY:    //	0x08
			if(Cmd_Argument_Length==0x01)  //查看
			{
				Temp[0]=nRF24L01_X_Busy[DEV_0];
				Temp[1]=nRF24L01_X_Busy[DEV_1];
				USB_ACK_2_Host(DevNum,TYPE_DBG,Temp,2);	
			}
			else if(Cmd_Argument_Length==0x03) //set busy bit
			{
				if(Cmd_Argument[1]) nRF24L01_X_Busy[DEV_0]=1;
				else nRF24L01_X_Busy[DEV_0]=0;
				if(Cmd_Argument[2]) nRF24L01_X_Busy[DEV_1]=1;
				else nRF24L01_X_Busy[DEV_1]=0;
		        USB_ACK_2_Host(DevNum,TYPE_DBG,NULL,0);	
			}
		break; 
		case DEBUG_CE_STATE_BEFORE_FLUSH:
			if(Cmd_Argument_Length==0x01)   //查看
			{
				Temp[0]=ce_state_before_flush;
				USB_ACK_2_Host(DevNum,TYPE_DBG,Temp,1);	
			}
			else if(Cmd_Argument_Length==0x02)   //设置ce_state_before_flush
			{
				if(Cmd_Argument[1])ce_state_before_flush=0x01;
				else ce_state_before_flush=0x00;
				USB_ACK_2_Host(DevNum,TYPE_DBG,NULL,0);	
			}	
		break;
        case DEBUG_TEST_MODE_OTHER_SPI_WRITE:     //特殊命令，留给FPGA调试使用。
            if(Cmd_Argument_Length==0x04)
            {
//                TEST_CSN=0;   //CSN pull down
                if(Dev_Flag[DEV_0]!=DEV_NONE)
                {
                    nRF24L01_X_spi_rw(DEV_0,Cmd_Argument[1]);
                    nRF24L01_X_spi_rw(DEV_0,Cmd_Argument[2]);
                    nRF24L01_X_spi_rw(DEV_0,Cmd_Argument[3]);
                }
                else  
                {
                    nRF24L01_X_spi_rw(DEV_1,Cmd_Argument[1]);
                    nRF24L01_X_spi_rw(DEV_1,Cmd_Argument[2]);
                    nRF24L01_X_spi_rw(DEV_1,Cmd_Argument[3]);
                }
//                TEST_CSN=1;   //CSN pull up
                USB_ACK_2_Host(DevNum,TYPE_DBG,NULL,0);
            }
            else
            {
                #ifdef HS6200_DEBUG 
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_DBG_LENGTH);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DBG_LENGTH);
					#endif	
                 #endif  /*HS6200_DEBUG*/	
            }         
        break;
        case DEBUG_READ_RX_PAYLOAD:  //查看刚才收到的payload
        	if(Cmd_Argument_Length==0x01)  //查看32字节 
        	{
        		USB_ACK_2_Host(DevNum,TYPE_PLD,&nRF24L01_X_Rx_Buf[DevNum][nRF24L01_X_PipeNum[DevNum]][0],33); 
        	}
            else if(Cmd_Argument_Length==0x02)   //查看指定长度 
            {
                if((Cmd_Argument[1]>0x00) && (Cmd_Argument[1]<=32))
	                USB_ACK_2_Host(DevNum,TYPE_PLD,&nRF24L01_X_Rx_Buf[DevNum][nRF24L01_X_PipeNum[DevNum]][0],Cmd_Argument[1]+0x01);
	            else //参数错误
	            {
	                #ifdef HS6200_DEBUG 
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_DBG_DATA);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DBG_DATA);
						#endif	
	                 #endif  /*HS6200_DEBUG*/	
	            }  
	            
            }
            else   //length error
            {
    		    #ifdef HS6200_DEBUG 
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_DBG_LENGTH);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DBG_LENGTH);
					#endif	
             	#endif  /*HS6200_DEBUG*/		
            }            
        break; 
        case DEBUG_READ_ADDR:
        	if(Cmd_Argument_Length==3)    //访问内Idata区域
        	{
        		p_iAddr=(U8*)Cmd_Argument[1];  //初始化指针  
        		if((Cmd_Argument[2]>0x00) && (Cmd_Argument[2]<=32))  //读取的字节数在[1-32]字节
        		{
        			for(i=0;i<Cmd_Argument[2];i++)data_read[i]=*p_iAddr++;
        			USB_ACK_2_Host(DevNum,TYPE_PLD,data_read,Cmd_Argument[2]);        			
        		}
        		else  //参数错误
        		{
        	    	#ifdef HS6200_DEBUG 
						#if (DEBUG_OUT_ERR==LED_OUT_ERR)
							LED_Out_Err_Info(ERR_USB_PACKET_DBG_DATA);
						#else
							USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DBG_DATA);
						#endif	
	                 #endif  /*HS6200_DEBUG*/
        		}
        	 		  
        	}
        	else if(Cmd_Argument_Length==4) //访问Xdata区域
        	{
        		p_xAddr=(U8 *)((Cmd_Argument[1]<<8)+Cmd_Argument[2]);  //先高字节后低字节
        		if( (Cmd_Argument[3]>0x00) && (Cmd_Argument[3]<=32) )
        		{
        			//for(i=0x00;i<Cmd_Argument[3];i++)data_read[i]=*p_xAddr++;
        			//data_read=p_xAddr;
                    USB_ACK_2_Host(DevNum, TYPE_PLD,p_xAddr, Cmd_Argument[3]);
        		}
        		else  //参数错误
        		{
        			#ifdef HS6200_DEBUG
        				#if(DEBUG_OUT_ERR==LED_OUT_ERR)
        					LED_Out_Err_Info(ERR_USB_PACKET_DBG_DATA);
        				#else
        					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DBG_DATA);
        				#endif
        			#endif
        		}
        	}
        	else
        	{
        		#ifdef HS6200_DEBUG
        			#if(DEBUG_OUT_ERR==LED_OUT_ERR)
        				LED_Out_Err_Info(ERR_USB_PACKET_DBG_LENGTH);
        			#else
        				USB_Out_Err_Info(DevNum, ERR_USB_PACKET_DBG_LENGTH);
        			#endif
        		#endif
        	}
        break;
        case DEBUG_CE_HIGH_LOW_START:
            USB_Host_Out_Flag=USB_HOST_OUT_FLAG_C8051F_USED;

            HS6200_Bank1_Activate(DevNum);           
            nRF24L01_X_read_reg_multibytes(DevNum, HS6200_BANK1_RF_IVGEN, Temp, 4);
            if(Cmd_Argument_Length==0x03)
            {
                Debug_ce_High_Low_Stop_Flag=0x00;
                if((Cmd_Argument[1]==0x00)&& Cmd_Argument[2]==0x00)Cmd_Argument[1]=0x01;
                while(Debug_ce_High_Low_Stop_Flag==0x00)
                {
                    nRF24L01_X_ce_high(DevNum);
                    Temp[3] &=~(BIT2+BIT3);
                    nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_RF_IVGEN, Temp, 4); 
                    if(Cmd_Argument[2]==0x00)chThdSleepMicroseconds(Cmd_Argument[1]);   //delay Cmd_Argument*10us.                                            
                    else chThdSleepMilliseconds(Cmd_Argument[1]);
                    nRF24L01_X_ce_low(DevNum);
                    Temp[3] |=(BIT2+BIT3);
                    nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_RF_IVGEN, Temp, 4);
                    if(Cmd_Argument[2]==0x00)chThdSleepMicroseconds(Cmd_Argument[1]);   //delay Cmd_Argument*10us.                                            
                    else chThdSleepMilliseconds(Cmd_Argument[1]);
                }
                HS6200_Bank0_Activate(DevNum);
            }
           
            else
        	{
        		#ifdef HS6200_DEBUG
        			#if(DEBUG_OUT_ERR==LED_OUT_ERR)
        				LED_Out_Err_Info(ERR_USB_PACKET_DBG_LENGTH);
        			#else
        				USB_Out_Err_Info(DevNum, ERR_USB_PACKET_DBG_LENGTH);
        			#endif
        		#endif
        	}   
 
        break;
		default:
			#ifdef HS6200_DEBUG 
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_DBG_CMD);
					#else
						USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DBG_CMD);
					#endif	
			#endif  /*HS6200_DEBUG*/	
		break;
	}	
}

void Debug_Protocol_Resolut(void)
{
	U8 DevNum;
	U8 Cmd_Argument[10];  //Cmd argument 长度暂定为10
	U8 Cmd_Argument_Length;
	U8 i;
	DevNum=USB_Host_Out_Packet[1];
	Cmd_Argument_Length=USB_Host_Out_Packet[2];
	
	if( (DevNum==RF_DEV_0) || (DevNum==RF_DEV_1) )
	{
		if( (Cmd_Argument_Length<10) && (Cmd_Argument_Length>=0x01) )
		{
			for(i=0x00;i<Cmd_Argument_Length;i++)Cmd_Argument[i]=USB_Host_Out_Packet[3+i];
			Debug_Mode(DevNum,Cmd_Argument,Cmd_Argument_Length);
		}
		else    //length err
		{
			#ifdef HS6200_DEBUG 
				#if (DEBUG_OUT_ERR==LED_OUT_ERR)
					LED_Out_Err_Info(ERR_USB_PACKET_DBG_LENGTH);
				#else
					USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DBG_LENGTH);
				#endif	
			#endif  /*HS6200_DEBUG*/	
		}
	}
	else   //RF_DEV err
	{
		#ifdef HS6200_DEBUG
			#if (DEBUG_OUT_ERR==LED_OUT_ERR)
    			LED_Out_Err_Info(ERR_USB_PACKET_DBG_RF_DEV);
			#else
				USB_Out_Err_Info(DevNum,ERR_USB_PACKET_DBG_RF_DEV);
			#endif	
		#endif  /*HS6200_DEBUG*/	
	}
}

/*----------------------------------------USB Potocol Resolut----------------------------------------------*/ 
/*
 * Analog test 根据需要进行更改BANK
 * 其他模式处于BANK0
 */
extern U8 g_interrupt_flag;
extern void HS6200_interrupt_process(void);
extern void MAC6200_interrupt_process(void);
void USB_Protocol_Resolut(void)
{
	if(USB_Host_Out_Flag==USB_HOST_OUT_FLAG_C8051F_RECEIVED)   //MCU Rx complete
	{

		switch(USB_Host_Out_Packet[0])
		{
			case PID_COMM: 		    //0x01
				Comm_Protocol_Resolut();	 	
			break; 
			case PID_SETUP:		     //0x02
				Setup_Protocol_Resolut();
			break;
			
			/*
			 *    P[0]       P[1]            P[2] 	           P[3]       P[4]
             *  PID_SPI	     Dev	 Packet Context Length	 cmd code	 argument
			 *	 0x03	 DEV_0/DEV_1         xx					xx			xx
			 *	1Byate		1Byte		  1Byte				   1Byte	  n Byte
			 */			    
			case PID_SPI:			  //0x03
				SPI_Protocol_Resolut();
			break;	
				
			/*
			 *   P[0]       P[1]              P[2]              P[3]         P[4]
			 * PID_SPEC	    Dev		  Packet Context Length   cmd code	   Argument
			 *  0x04	DEV_0/DEV_1			  xx				 xx			  xx
			 *  1Byte	   1Byte		     1Byte			   1Byte		  xx
			 *
			 */
			case PID_SPEC:				 //0x04
				Spec_Protocol_Resolut();
			break;
			
			case PID_DIGITAL_TEST:		//0x09
				Ditital_Test_Protocol_Resolut();
			break;

			/*
			 *       P[0]         P[1]            P[2]			    p[3]         p[4]
			 *	PID_ANALOG_TEST	  Dev     packet context length     mode       Argument 
			 *		0x09		0x00/0x01		   xx			   0x01-0x08      xx
			 *      1Byte		 1Byte			  1Byte				1Byte	    n Byte
			 */
			case PID_ANALOG_TEST:		//0x0A
				Analog_Test_Protocol_Resolut();
			break;
			case PID_SYSTEM_TEST:       //0x0B
				System_Protocol_Resolut();
			break;
			case PID_SET:               //0x0C
				Set_Protocol_Resolut();
			break;
			case PID_GET:               //0x0D
				Get_Protocol_Resolut();
			break;
			case PID_DEBUG:             //0x0E
				Debug_Protocol_Resolut();	
			break;			
			default: 
				#ifdef HS6200_DEBUG
					#if (DEBUG_OUT_ERR==LED_OUT_ERR)
						LED_Out_Err_Info(ERR_USB_PACKET_PID);
					#else
						USB_Out_Err_Info(0x00,ERR_USB_PACKET_PID);
					#endif
				#endif
			break;
		}
		USB_Host_Out_Flag=USB_HOST_OUT_FLAG_C8051F_USED;

	}
	else if(USB_HOST_OUT_FLAG_C8051F_DISCARD==USB_Host_Out_Flag)
	{
		USB_NAK_2_Host(USB_Discard_Dev_Num);   //  
		USB_Host_Out_Flag=USB_HOST_OUT_FLAG_C8051F_USED;
	} 
    
    if(g_interrupt_flag == INTERRUPT_HS6200)
        HS6200_interrupt_process();
    
    if(g_interrupt_flag == INTERRUPT_MAC6200)
        MAC6200_interrupt_process();
}

 

/*----------------------------------End Of File---------------------------------*/

