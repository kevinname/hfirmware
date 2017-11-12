
/*
 * 说明：HS6200相关测试程序，请参考《HS6200_Test_Plan.docx》.
 *
 * Yao  15/04/2013
 * HunterSun Electronics Co.,Ltd.
 *
 */
#include "nRF24L01_X.h"
#include "HS6200_test.h"
#include "HS6200_test_sys.h"

#include "HS6200_Reg.h"	
#include "HS6200_Debug.h"
#include "HS6200_Analog_Test.h"
#include "usbsubdev_hs6200.h"
#include "stdlib.h"
#include "HS6200Test_Application_Protocol.h"
#include "C8051F_USB.h"

U8 Cal_after_Ack=0;
U8 flush_tx_when_max_rt=0x01;

/**************************************************/

/*
 * Communication 
 *  Packet[0]     Packet[1]            Packet[2]            Packet[3]       Packet[4..n] 
 *    PID            Dev                  Bytes               pipe        payload/ACK payload
 *  PID_COMM      Dev_0/Dev_1      packet context length    pipe index        
 *   1Byte           1Byte               1+1——1+32Bytes       1Byte           32Bytes   
 *   0x01			 
 */

//--------------------------------------------HS6200 Init------------------------------------------
void HS6200_Port_Init(void)		        //HS6200端口初始化，包括CE拉低，判断外接设备属性
{
	U8 Rx_Addr=0x00;
	U8 Temp1,Temp2;

	nRF24L01_X_ce_low(DEV_0);	   //上电后，默认输出高电平，所以先拉低 
	nRF24L01_X_ce_low(DEV_1);
		
    //写进去和读出来 一致说明外接device.
	HS6200_Bank0_Activate(DEV_0);
	Rx_Addr=nRF24L01_X_read_reg(DEV_0,nRF24L01_RX_ADDR_P2);    //Rx pipe2 address	
	Temp1=0xAA;
	nRF24L01_X_write_reg(DEV_0,nRF24L01_RX_ADDR_P2,Temp1);
	Temp2=nRF24L01_X_read_reg(DEV_0,nRF24L01_RX_ADDR_P2);
	if(Temp1==Temp2)   //外接dev
	{
		 nRF24L01_X_write_reg(DEV_0,nRF24L01_RX_ADDR_P2,Rx_Addr);
//		 nRF24L01_0_EI=1;		  //允许中断
	}
	else  //未接dev
	{
//		nRF24L01_0_EI=0;		  //禁止中断
		Dev_Flag[DEV_0]=DEV_NONE;
	}
	HS6200_Bank0_Activate(DEV_1);
	Rx_Addr=nRF24L01_X_read_reg(DEV_1,nRF24L01_RX_ADDR_P2);    //Rx pipe2 address	
	Temp1=0xAA;
	nRF24L01_X_write_reg(DEV_1,nRF24L01_RX_ADDR_P2,Temp1);
	Temp2=nRF24L01_X_read_reg(DEV_1,nRF24L01_RX_ADDR_P2);
	if(Temp1==Temp2)   //外接dev
	{
		 nRF24L01_X_write_reg(DEV_1,nRF24L01_RX_ADDR_P2,Rx_Addr);
 //                nRF24L01_1_EI=1;		  //允许中断
	}
	else 
	{
//                nRF24L01_1_EI=0;		  //禁止中断
		Dev_Flag[DEV_1]=DEV_NONE;
	}
}

void HS6200_Init(U8 DevNum)	   //HS6200 初始化
{
	U8 Reg_Val[4];

	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,Reg_Val,2);
	Reg_Val[0]|=HS6200_BANK1_PLL_RST_CNT;		          //PLL_RST_CNT=1
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,Reg_Val,2);

	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Reg_Val,4);
	Reg_Val[3]|=HS6200_BANK1_PLL_RSTn_PFD;				  //PLL_RSTn_PFD=1
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Reg_Val,4);
	HS6200_Bank0_Activate(DevNum);
}

void System_Init(void)	  //系统初始化
{	
	Dev_Scan();	     	 //读取外接芯片类型，初始化成HS6200/nRF24L01
	HS6200_Port_Init();	 //端口初始化，判断有无设备	
 	while( (Dev_Flag[DEV_0]==DEV_NONE) && (Dev_Flag[DEV_1]==DEV_NONE) ) //未外接设备，表示设备可能未连接好，等待
 	{
 		chThdSleepMilliseconds(150);    // nRF24L01 加电复位时间：100ms。
 		HS6200_Port_Init();	 //端口初始化，判断有无设备	
 	}
// 	if( Dev_Flag[DEV_0]==HS6200_DEV )	HS6200_Init(DEV_0);		
    //HS6200 Init
// 	if( Dev_Flag[DEV_1]==HS6200_DEV )	HS6200_Init(DEV_1);
}


/*------------------------------------------------HS6200的测试---------------------------------------------*/
void HS6200_Rst_Bank0_All_Register(U8 DevNum)     //reset Bank0 all the registers
{
    U8 pTemp[5];
	U8 i;

	if( Dev_Flag[DevNum]==HS6200_DEV )	   //DevNum 是HS6200 device
	{
		HS6200_Bank0_Activate(DevNum);
	}
	//Register Addr:0x00-0x09
	nRF24L01_X_write_reg(DevNum,nRF24L01_CONFIG,nRF24L01_CONFIG_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_EN_AA,nRF24L01_EN_AA_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_EN_RXADDR,nRF24L01_EN_RXADDR_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_SETUP_AW,nRF24L01_SETUP_AW_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_SETUP_RETR,nRF24L01_SETUP_RETR_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_RF_CH,nRF24L01_RF_CH_RESET_VALUE);
	if(Dev_Flag[DevNum]==nRF24L01_DEV)nRF24L01_X_write_reg(DevNum,nRF24L01_RF_SETUP,0x0F);  //你RF24L01实际读出来是0x0F,nRF24L01_RF_SETUP_RESET_VALUE
	else if(Dev_Flag[DevNum]==HS6200_DEV)nRF24L01_X_write_reg(DevNum,nRF24L01_RF_SETUP,0x0E); //HS6200复位值为0x0E
	nRF24L01_X_write_reg(DevNum,nRF24L01_STATUS,nRF24L01_STATUS_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_OBSERVE_TX,nRF24L01_OBSERVE_TX_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_RPD,nRF24L01_RPD_RESET_VALUE);

	//0x0A-0x0B
	for(i=0x00;i<=0x04;i++) pTemp[i]=0xE7;
	nRF24L01_X_write_pipe_addr(DevNum, nRF24L01_RX_ADDR_P0, pTemp, 5);

	for(i=0x00;i<=0x04;i++) pTemp[i]=0xC2;
	nRF24L01_X_write_pipe_addr(DevNum, nRF24L01_RX_ADDR_P1, pTemp, 5);

	//0x0C-0x0F
	nRF24L01_X_write_reg(DevNum,nRF24L01_RX_ADDR_P2,nRF24L01_RX_ADDR_P2_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_RX_ADDR_P3,nRF24L01_RX_ADDR_P3_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_RX_ADDR_P4,nRF24L01_RX_ADDR_P4_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_RX_ADDR_P5,nRF24L01_RX_ADDR_P5_RESET_VALUE);

	//0x10
	for(i=0x00;i<=0x04;i++) pTemp[i]=0xE7;
	nRF24L01_X_write_pipe_addr(DevNum, nRF24L01_TX_ADDR, pTemp, 5);

	//0x11-0x17
	nRF24L01_X_write_reg(DevNum,nRF24L01_RX_PW_P0,nRF24L01_RX_PW_P0_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_RX_PW_P1,nRF24L01_RX_PW_P1_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_RX_PW_P2,nRF24L01_RX_PW_P2_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_RX_PW_P3,nRF24L01_RX_PW_P3_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_RX_PW_P4,nRF24L01_RX_PW_P4_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_RX_PW_P5,nRF24L01_RX_PW_P5_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_FIFO_STATUS,nRF24L01_FIFO_STATUS_RESET_VALUE);
	
	//0x1C-0x1D
	nRF24L01_X_write_reg(DevNum,nRF24L01_DYNPD,nRF24L01_DYNPD_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,nRF24L01_FEATURE,nRF24L01_FEATURE_RESET_VALUE);

	if( Dev_Flag[DevNum]==HS6200_DEV )
	{	
		pTemp[0]=0x28;
		pTemp[1]=0x28;			  
		nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK0_SETUP_VALUE,pTemp,2);
		nRF24L01_X_write_reg(DevNum,HS6200_BANK0_PRE_GURD,HS6200_BANK0_PRE_GURD_RESET_VALUE);
	}
}

U8 HS6200_Read_Bank0_All_Register(U8 DevNum,U8 *Reg_Val)	 //read bank0 all the registers
{
	U8 i;
	U8 Reg_Val_Length=0;
	U8 pTemp[5];
	U8 Reg_Addr=0x00;
	if( Dev_Flag[DevNum]==HS6200_DEV )
	{
		HS6200_Bank0_Activate(DevNum);
	}
	//Register Addr ：0x00-0x09
	Reg_Addr=0x00;
	for(i=0x00;i<=27;)  //Reg_Val:00-02 03-05 06-08 09-11 12-14 15-17 18-20 21-23 24-26 27-29
	{	
		Reg_Val[0+i]=Reg_Addr;					  //reg addr
		Reg_Val[1+i]=0x01;				  //length
		Reg_Val[2+i]=nRF24L01_X_read_reg(DevNum, Reg_Addr);						  //Reg_Val
		Reg_Addr++;
		i+=0x03;
	}

	//Register Addr: 0x0A 
	nRF24L01_X_read_pipe_addr(DevNum, nRF24L01_RX_ADDR_P0, pTemp, 5);	  //Reg_Val: 30	31 32-36
	Reg_Val[30]=0x0A;
	Reg_Val[31]=0x05;  //length=5Bytes   
	for(i=0x00;i<=0x04;i++)Reg_Val[32+i]=pTemp[0x04-i];	  //5Bytes

	//Register Addr: 0x0B
	if(Dev_Flag[DevNum]==nRF24L01_DEV)	     //nRF24L01 dev
	{
		nRF24L01_X_read_pipe_addr(DevNum, nRF24L01_RX_ADDR_P1, pTemp, 5);	   //Reg_Val: 37 38 39-43   
		Reg_Val[37]=0x0B;
		Reg_Val[38]=0x05;
		for(i=0x00;i<=0x04;i++)Reg_Val[39+i]=pTemp[0x04-i];

	   	//Register Addr: 0x0C-0x0F
		Reg_Val[44]=0x0C;				  //Reg_Val: 44 45 46   
		Reg_Val[45]=0x01;
	    Reg_Val[46]=nRF24L01_X_read_reg(DevNum, 0x0C);
		Reg_Val[47]=0x0D;				  //Reg_Val: 47 48 49
		Reg_Val[48]=0x01;
	    Reg_Val[49]=nRF24L01_X_read_reg(DevNum, 0x0D);
		Reg_Val[50]=0x0E;				  //Reg_Val: 50 51 52
		Reg_Val[51]=0x01;
	    Reg_Val[52]=nRF24L01_X_read_reg(DevNum, 0x0E);
		Reg_Val[53]=0x0F;				 //Reg_Val:53 54 55
		Reg_Val[54]=0x01;
	    Reg_Val[55]=nRF24L01_X_read_reg(DevNum, 0x0F);


		//Register Addr: 0x10   
		Reg_Val[56]=0x10;
		Reg_Val[57]=0x05; 
		nRF24L01_X_read_pipe_addr(DevNum, nRF24L01_TX_ADDR, pTemp, 5);		  //Reg_Val=56 57 58-62
		for(i=0x00;i<=0x04;i++)Reg_Val[58+i]=pTemp[0x04-i];
	
		//Register Addr: 0x11-0x17
		Reg_Addr=0x11;
		for(i=0x00;i<=18;)
		{ 
			Reg_Val[63+i]=Reg_Addr;		        //Reg_Val: 63-65 66-68 69-71 72-74 75-77 78-80 81-83
			Reg_Val[64+i]=0x01;	    
			Reg_Val[65+i]=nRF24L01_X_read_reg(DevNum, Reg_Addr);	          
			Reg_Addr++;
			i+=0x03;
		}
		//Register Addr:0x1C-0x1D
		Reg_Val[84]=0x1C;				  //Reg_Val:84 85 86
		Reg_Val[85]=0x01;
	    Reg_Val[86]=nRF24L01_X_read_reg(DevNum, 0x1C);
		Reg_Val[87]=0x1D;				 //Reg_Val:	87 88 89
		Reg_Val[88]=0x01;
	    Reg_Val[89]=nRF24L01_X_read_reg(DevNum, 0x1D);
        
		Reg_Val_Length=90;
	} 	
	else 									//HS6200 dev
	{ 
		Reg_Addr=0x0B;
		for(i=0x00;i<=12;)			//Reg_Val: 37-39,40-42,43-45, 46-48,49-51 
		{
			Reg_Val[37+i]=Reg_Addr;
			Reg_Addr++;
			Reg_Val[38+i]=0x01;
			Reg_Val[39+i]=nRF24L01_X_read_reg(DevNum, Reg_Addr);
			i+=3;	
		}
		//Register Addr: 0x10   
		Reg_Val[52]=0x10;
		Reg_Val[53]=0x05; 
		nRF24L01_X_read_pipe_addr(DevNum, nRF24L01_TX_ADDR, pTemp, 5);		  //Reg_Val=52 53 54-58
		for(i=0x00;i<=0x04;i++)Reg_Val[54+i]=pTemp[0x04-i];
	
		//Register Addr: 0x11-0x17
		Reg_Addr=0x11;
		for(i=0x00;i<=18;)
		{ 
			Reg_Val[59+i]=Reg_Addr;		        //Reg_Val: 59-61 62-64 65-67 68-70 71-73 74-76 77-79
			Reg_Val[60+i]=0x01;	    
			Reg_Val[61+i]=nRF24L01_X_read_reg(DevNum,Reg_Addr);	          
			Reg_Addr++;
			i+=0x03;
		}
		//Register Addr:0x1C-0x1D
		Reg_Val[80]=0x1C;				  //Reg_Val: 80 81 82
		Reg_Val[81]=0x01;
	    Reg_Val[82]=nRF24L01_X_read_reg(DevNum, 0x1C);
		Reg_Val[83]=0x1D;				  //Reg_Val: 83 84 85
		Reg_Val[84]=0x01;
	    Reg_Val[85]=nRF24L01_X_read_reg(DevNum, 0x1D);

		//Register Addr:0x1E
		Reg_Val[86]=0x1E;
		Reg_Val[87]=0x02;
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK0_SETUP_VALUE,pTemp,2);   //Reg_Val:86 87 88-89
		for(i=0x00;i<=0x01;i++)Reg_Val[88+i]=pTemp[0x01-i]; 	
		//Register Addr: 0x1F
		Reg_Val[89]=90;
		Reg_Val[90]=0x01;
		Reg_Val[91]=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_PRE_GURD);            //Reg_Val:90 91 92
		Reg_Val_Length=92;
	}
	return Reg_Val_Length;
}

void HS6200_Write1_to_Bank0_All_Register(U8 DevNum)	   //write 1 to all register  
{
    U8 pTemp[5];
	U8 i;
	if( Dev_Flag[DevNum]==HS6200_DEV )
	{
		HS6200_Bank0_Activate(DevNum);
	}
	//Register Addr:0x00-0x09
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_CONFIG,HS6200_BANK0_CONFIG_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_AA,HS6200_BANK0_EN_AA_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_RXADDR,HS6200_BANK0_EN_RXADDR_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_SETUP_AW,HS6200_BANK0_SETUP_AW_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_SETUP_RETR,HS6200_BANK0_SETUP_RETR_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_CH,HS6200_BANK0_RF_CH_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,HS6200_BANK0_RF_SETUP_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_STATUS,HS6200_BANK0_STATUS_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_OBSERVE_TX,HS6200_BANK0_OBSERVE_TX_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RPD,HS6200_BANK0_RPD_1_VALUE);

	//0x0A-0x0B
	for(i=0x00;i<=0x04;i++) pTemp[i]=0xFF;
	nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_RX_ADDR_P0, pTemp, 5);

	for(i=0x00;i<=0x04;i++) pTemp[i]=0xFF;
	nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_RX_ADDR_P1, pTemp, 5);

	//0x0C-0x0F
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_ADDR_P2,HS6200_BANK0_RX_ADDR_P2_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_ADDR_P3,HS6200_BANK0_RX_ADDR_P3_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_ADDR_P4,HS6200_BANK0_RX_ADDR_P4_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_ADDR_P5,HS6200_BANK0_RX_ADDR_P5_1_VALUE);

	//0x10
	for(i=0x00;i<=0x04;i++) pTemp[i]=0xFF;
	nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_TX_ADDR, pTemp, 5);

	//0x11-0x17
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P0,HS6200_BANK0_RX_PW_P0_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P1,HS6200_BANK0_RX_PW_P1_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P2,HS6200_BANK0_RX_PW_P2_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P3,HS6200_BANK0_RX_PW_P3_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P4,HS6200_BANK0_RX_PW_P4_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P5,HS6200_BANK0_RX_PW_P5_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FIFO_STATUS,HS6200_BANK0_FIFO_STATUS_1_VALUE);
	
	//0x1C-0x1D
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_DYNPD,HS6200_BANK0_DYNPD_1_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FEATURE,HS6200_BANK0_FEATURE_1_VALUE);	
	
	//0x1E-0x1F
	if ( Dev_Flag[DevNum]==HS6200_DEV )
	{
		for(i=0x00;i<=0x01;i++) pTemp[i]=0xFF;
		nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK0_SETUP_VALUE,pTemp,2);
		nRF24L01_X_write_reg(DevNum,nRF24L01_FIFO_STATUS,HS6200_BANK0_PRE_GURD_1_VALUE);
	}
}

void HS6200_Write0_to_Bank0_All_Register(U8 DevNum)	   //write 0 to bank0 all the registers
{
    U8 pTemp[5];
	U8 i;
	if( Dev_Flag[DevNum]==HS6200_DEV )
	{
		HS6200_Bank0_Activate(DevNum);
	}

	//Register Addr:0x00-0x09
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_CONFIG,HS6200_BANK0_CONFIG_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_AA,HS6200_BANK0_EN_AA_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_RXADDR,HS6200_BANK0_EN_RXADDR_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_SETUP_AW,HS6200_BANK0_SETUP_AW_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_SETUP_RETR,HS6200_BANK0_SETUP_RETR_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_CH,HS6200_BANK0_RF_CH_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,HS6200_BANK0_RF_SETUP_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_STATUS,HS6200_BANK0_STATUS_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_OBSERVE_TX,HS6200_BANK0_OBSERVE_TX_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RPD,HS6200_BANK0_RPD_0_VALUE);

	//0x0A-0x0B
	for(i=0x00;i<=0x04;i++) pTemp[i]=0x00;
	nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_RX_ADDR_P0, pTemp, 5);

	for(i=0x00;i<=0x04;i++) pTemp[i]=0x00;
	nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_RX_ADDR_P1, pTemp, 5);

	//0x0C-0x0F
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_ADDR_P2,HS6200_BANK0_RX_ADDR_P2_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_ADDR_P3,HS6200_BANK0_RX_ADDR_P3_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_ADDR_P4,HS6200_BANK0_RX_ADDR_P4_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_ADDR_P5,HS6200_BANK0_RX_ADDR_P5_0_VALUE);

	//0x10
	for(i=0x00;i<=0x04;i++) pTemp[i]=0x00;
	nRF24L01_X_write_pipe_addr(DevNum, HS6200_BANK0_TX_ADDR, pTemp, 5);

	//0x11-0x17
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P0,HS6200_BANK0_RX_PW_P0_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P1,HS6200_BANK0_RX_PW_P1_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P2,HS6200_BANK0_RX_PW_P2_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P3,HS6200_BANK0_RX_PW_P3_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P4,HS6200_BANK0_RX_PW_P4_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RX_PW_P5,HS6200_BANK0_RX_PW_P5_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FIFO_STATUS,HS6200_BANK0_FIFO_STATUS_0_VALUE);
	
	//0x1C-0x1D
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_DYNPD,HS6200_BANK0_DYNPD_0_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FEATURE,HS6200_BANK0_FEATURE_0_VALUE);
	
	//0x1E-0x1F	
	if( Dev_Flag[DevNum]==HS6200_DEV )
	{
		for(i=0x00;i<=0x01;i++) pTemp[i]=0x00;
		nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK0_SETUP_VALUE,pTemp,2);
		nRF24L01_X_write_reg(DevNum,nRF24L01_FIFO_STATUS,HS6200_BANK0_PRE_GURD_0_VALUE);
	}
}

//----------------------------Bank1的测试操作-------------------------------
 //读BANK1  所有的寄存器
U8 HS6200_Read_Bank1_All_Register(U8 DevNum,U8 *Reg_Val)
{
	U8 i;
	U8 pTemp[5];
	if( Dev_Flag[DevNum]==HS6200_DEV )
	{
		HS6200_Bank1_Activate(DevNum);
	}

	//Register Addr ：0x00	 Reg_Val:0x00 0x01 02 
	Reg_Val[0]=0x80; 
	Reg_Val[1]=0x01;  
	Reg_Val[2]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_RESERVED0);						  

	//Register Addr:0x01	 Reg_Val:0x03 0x04 0x05-0x08
	Reg_Val[3]=0x81; 
	Reg_Val[4]=0x04;  
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,pTemp,4);  
	for(i=0x00;i<=0x03;i++) Reg_Val[5+i]=pTemp[0x03-i];
	 
    //Register Addr:0x02	 Reg_Val:0x09 0x0A 0x0B-0x0E
	Reg_Val[0x09]=0x82;
	Reg_Val[0x0A]=0x04;
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL1,pTemp,4);
	for(i=0x00;i<=0x03;i++) Reg_Val[0x0B+i]=pTemp[0x03-i];

    //Register Addr:0x03	 Reg_Val:0x0F 0x10 0x11-0x12
	Reg_Val[0x0F]=0x83;
	Reg_Val[0x10]=0x02;
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,pTemp,2);
	for(i=0x00;i<=0x01;i++) Reg_Val[0x11+i]=pTemp[0x01-i];

	//Register Addr:0x04     Reg_Val:0x13 0x14 0x15	
	Reg_Val[0x13]=0x84;
	Reg_Val[0x14]=0x01;
	Reg_Val[0x15]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_A_CNT_REG);	

	//Register Addr:0x05     Reg_Val:0x16 0x17 0x18 
	Reg_Val[0x16]=0x85;
	Reg_Val[0x17]=0x01;
	Reg_Val[0x18]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_B_CNT_REG);

	//Register Addr:0x06     Reg_Val:0x19 0x1A 0x1B 
	Reg_Val[0x19]=0x86;
	Reg_Val[0x1A]=0x01;
	Reg_Val[0x1B]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_RESERVED1);

	//Register Addr:0x07     Reg_Val:0x1C 0x1D 0x1E
	Reg_Val[0x1C]=0x87;
	Reg_Val[0x1D]=0x01;
	Reg_Val[0x1E]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_STATUS);

	//Register Addr:0x08     Reg_Val:0x1F 0x20 0x21 
	Reg_Val[0x1F]=0x88;
	Reg_Val[0x20]=0x01;
	Reg_Val[0x21]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_RESERVED2);


   	//Register Addr:0x09	 Reg_Val:0x22 0x23 0x24-0x27
	Reg_Val[0x22]=0x89;
	Reg_Val[0x23]=0x04;
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CHAN,pTemp,4);
	for(i=0x00;i<=0x03;i++) Reg_Val[0x24+i]=pTemp[0x03-i];

	//Register Addr:0x0A	 Reg_Val:0x28 0x29 0x2A-0x2C
	Reg_Val[0x28]=0x8A;
	Reg_Val[0x29]=0x03;
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_IF_FREQ,pTemp,3);
	for(i=0x00;i<=0x02;i++) Reg_Val[0x2A+i]=pTemp[0x02-i];

	//Register Addr:0x0B	 Reg_Val:0x2D 0x2E 0x2F-0x31
	Reg_Val[0x2D]=0x8B;
	Reg_Val[0x2E]=0x03;
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AFC_COR,pTemp,3);
	for(i=0x00;i<=0x02;i++) Reg_Val[0x2F+i]=pTemp[0x02-i];

	//Register Addr:0x0C	Reg_Val:0x32 0x33 0x34
	Reg_Val[0x32]=0x8C;
	Reg_Val[0x33]=0x01;
	Reg_Val[0x34]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_FDEV); 

   	//Register Addr:0x0D	Reg_Val:0x35 0x36 0x37
	Reg_Val[0x35]=0x8D;
	Reg_Val[0x36]=0x01;
	Reg_Val[0x37]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_DAC_RANGE); 

	//Register Addr:0x0E	Reg_Val:0x38 0x39 0x3A
	Reg_Val[0x38]=0x8E;
	Reg_Val[0x39]=0x01;
	Reg_Val[0x3A]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_DAC_IN); 

   	//Register Addr:0x0F	Reg_Val:0x3B 0x3C 0x3D
	Reg_Val[0x3B]=0x8F;
	Reg_Val[0x3C]=0x01;
	Reg_Val[0x3D]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_CTUNING); 

	//Register Addr:0x10	Reg_Val:0x3E 0x3F 0x40
	Reg_Val[0x3E]=0x90;
	Reg_Val[0x3F]=0x01;
	Reg_Val[0x40]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_FTUNING); 

	//Register Addr:0x11	 Reg_Val:0x41 0x42 0x43-0x46
	Reg_Val[0x41]=0x91;
	Reg_Val[0x42]=0x04;
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_RX_CTRL,pTemp,4);
	for(i=0x00;i<=0x03;i++) Reg_Val[0x43+i]=pTemp[0x03-i];

	//Register Addr:0x12	 Reg_Val:0x47 0x48 0x49-0x4C
	Reg_Val[0x47]=0x92;
	Reg_Val[0x48]=0x04;
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL,pTemp,4);
	for(i=0x00;i<=0x03;i++) Reg_Val[0x49+i]=pTemp[0x03-i];

	//Register Addr:0x17	Reg_Val:0x4D 0x4E 0x4F
	Reg_Val[0x4D]=0x97;
	Reg_Val[0x4E]=0x01;
	Reg_Val[0x4F]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_DAC_CAL_LOW); 
	
	//Register Addr:0x18	Reg_Val:0x50 0x51 0x52
	Reg_Val[0x50]=0x98;
	Reg_Val[0x51]=0x01;
	Reg_Val[0x52]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_DAC_CAL_HI); 

    //Register Addr:0x19	Reg_Val:0x53 0x54 0x55
	Reg_Val[0x53]=0x99;
	Reg_Val[0x54]=0x01;
	Reg_Val[0x55]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_RESERVED3);    
  
    //Register Addr:0x1A	Reg_Val:0x56 0x57 0x58
	Reg_Val[0x56]=0x9A;
	Reg_Val[0x57]=0x01;
	Reg_Val[0x58]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_DOC_DACI); 

   	//Register Addr:0x1B	Reg_Val:0x59 0x5A 0x5B
	Reg_Val[0x59]=0x9B;
	Reg_Val[0x5A]=0x01;
	Reg_Val[0x5B]=nRF24L01_X_read_reg(DevNum, HS6200_BANK1_DOC_DACQ);

	//Register Addr:0x1C	 Reg_Val:0x5C 0x5D 0x5E-0x61
	Reg_Val[0x5C]=0x9C;
	Reg_Val[0x5D]=0x04;
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_CTRL,pTemp,4);
//	for(i=0x00;i<=0x03;i++) Reg_Val[0x5E+i]=pTemp[0x03-i];
//
//	//Register Addr:0x1D	 Reg_Val:0x5F 0x60 0x61-0x64
//	Reg_Val[0x5F]=0x9D;
//	Reg_Val[0x60]=0x04;
//    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,pTemp,4);
//	for(i=0x00;i<=0x03;i++) Reg_Val[0x61+i]=pTemp[0x03-i];

	//Register Addr:0x1E	 Reg_Val:0x65 0x66 0x67-0x6A
	Reg_Val[0x65]=0x9E;
	Reg_Val[0x66]=0x04;
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_RF_IVGEN,pTemp,4);
	for(i=0x00;i<=0x03;i++) Reg_Val[0x67+i]=pTemp[i];

	//Register Addr:0x1F	 Reg_Val:0x6B 0x6C 0x6D-0x70
	Reg_Val[0x6B]=0x9F;
	Reg_Val[0x6C]=0x04;
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,pTemp,4);
	for(i=0x00;i<=0x03;i++) Reg_Val[0x6D+i]=pTemp[0x03-i];

	return 0x71;   
}

//reset HS6200 Bank1 all register
 void HS6200_Rst_Bank1_All_Register(U8 DevNum)
 {
 	U8 pTemp[4];
	U8 i;

	if( Dev_Flag[DevNum]==HS6200_DEV )
	{
		HS6200_Bank1_Activate(DevNum);
	}

	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_RESERVED0,HS6200_BANK1_RESERVED0_RESET_VALUE);	   //Bank1 Register Addr: 0x00

	for(i=0x00;i<=0x03;i++)	pTemp[i]=(HS6200_BANK1_PLL_CTL0_RESET_VALUE>>(8*i));		       //Bank1 Register Addr: 0x01
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, pTemp, 4);

	for(i=0x00;i<=0x03;i++)	pTemp[i]=(HS6200_BANK1_PLL_CTL1_RESET_VALUE>>(8*i));			   //Bank1 Register Addr: 0x02
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL1, pTemp, 4);

	for(i=0x00;i<=0x01;i++)	pTemp[i]=(HS6200_BANK1_CAL_CTL_RESET_VALUE>>(8*i));				   //Bank1 Register Addr: 0x03
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_CAL_CTL, pTemp, 2);

	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_A_CNT_REG,HS6200_BANK1_A_CNT_REG_RESET_VALUE);    //Bank1 Register Addr: 0x04-0x08
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_B_CNT_REG,HS6200_BANK1_B_CNT_REG_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_RESERVED1,HS6200_BANK1_RESERVED1_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_STATUS,HS6200_BANK1_STATUS_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_RESERVED2,HS6200_BANK1_RESERVED2_RESET_VALUE);

	for(i=0x00;i<=0x03;i++)	pTemp[i]=(HS6200_BANK1_CHAN_RESET_VALUE>>(8*i));                   //Bank1 Register Addr: 0x09
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_CHAN, pTemp, 4);

	for(i=0x00;i<=0x02;i++)	pTemp[i]=(HS6200_BANK1_IF_FREQ_RESET_VALUE>>(8*i));				  //Bank1 Register Addr: 0x0A
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_IF_FREQ, pTemp, 3);

	for(i=0x00;i<=0x02;i++)	pTemp[i]=(HS6200_BANK1_AFC_COR_RESET_VALUE>>(8*i));				  //Bank1 Register Addr: 0x0B
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AFC_COR, pTemp, 3);

   	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_FDEV,HS6200_BANK1_FDEV_RESET_VALUE);			  //Bank1 Register Addr: 0x0C-0x10
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_DAC_RANGE,HS6200_BANK1_DAC_RANGE_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_DAC_IN,HS6200_BANK1_DAC_IN_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_CTUNING,HS6200_BANK1_CTUNING_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_FTUNING,HS6200_BANK1_FTUNING_RESET_VALUE);

	for(i=0x00;i<=0x03;i++)	pTemp[i]=(HS6200_BANK1_RX_CTRL_RESET_VALUE>>(8*i));				 //Bank1 Register Addr: 0x11
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_RX_CTRL, pTemp, 4);

	for(i=0x00;i<=0x03;i++)	pTemp[i]=(HS6200_BANK1_FAGC_CTRL_RESET_VALUE>>(8*i));			//Bank1 Register Addr: 0x12
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_FAGC_CTRL, pTemp, 4);

	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_FDEV,HS6200_BANK1_DAC_CAL_LOW);			     //Bank1 Register Addr: 0x17-0x1B
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_DAC_CAL_HI,HS6200_BANK1_DAC_CAL_HI_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_RESERVED3,HS6200_BANK1_RESERVED3_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_DOC_DACI,HS6200_BANK1_DOC_DACI_RESET_VALUE);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_DOC_DACQ,HS6200_BANK1_DOC_DACQ_RESET_VALUE);

	for(i=0x00;i<=0x03;i++)	pTemp[i]=(HS6200_BANK1_AGC_CTRL_RESET_VALUE>>(8*i));		   //Bank1 Register Addr: 0x1C
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_CTRL, pTemp, 4);

	for(i=0x00;i<=0x03;i++)	pTemp[i]=(HS6200_BANK1_AGC_GAIN_RESET_VALUE>>(8*i));		   //Bank1 Register Addr: 0x1D
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, pTemp, 4);

	for(i=0x00;i<=0x03;i++)	pTemp[i]=(HS6200_BANK1_RF_IVGEN_RESET_VALUE>>(8*i));		   //Bank1 Register Addr: 0x1E
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_RF_IVGEN, pTemp, 4);

	for(i=0x00;i<=0x03;i++)	pTemp[i]=(HS6200_BANK1_TEST_PKDET_RESET_VALUE>>(8*i));	       //Bank1 Register Addr: 0x1F
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, pTemp, 4);
 }

//对BANK1 进行写操作，HS6200可能就会出乱
U8 HS6200_Write0_Bank1_All_Register(U8 DevNum)
{
	DevNum=DevNum;
	return 0;
}

U8 HS6200_Read_All_Register(U8 DevNum,U8* Reg_Val)
{
	U8 Temp_Length=0x00;
	U8 Temp[0x71];		  //0x71=113>93
	U8 Reg_Val_Length=0x00;
	U8 i;

	Temp_Length=HS6200_Read_Bank0_All_Register(DevNum,Temp);
	for(i=0x00;i<Temp_Length;i++) Reg_Val[i]=Temp[i];
	Reg_Val_Length=Temp_Length;
	if(Dev_Flag[DevNum]==HS6200_DEV)
	{
		Temp_Length=HS6200_Read_Bank1_All_Register(DevNum,Temp);
		for(i=0x00;i<Temp_Length;i++) Reg_Val[Reg_Val_Length+i]=Temp[i];
		Reg_Val_Length+=Temp_Length;
	}
	return Reg_Val_Length;
}


/*
 * HS6200/nRF24L01 DEV0的接收中断程序
 */
void HS6200_DEV0_Int(void)
{
	U8 pipe_num;
	U8 plw;
	U8 i=0x00;
	U8 Dev_Status;
	U8 USB_Ack_Flag=0x00;
	U8 Fifo_Status;
	U8 Payload[32];
        U8 g_Dev_Status;	   
	
	g_Dev_Status=nRF24L01_X_nop(DEV_0); //read register STATUS's value

	USB_ACK_Diagnose_Info(DEV_0);
	
	if(g_Dev_Status & BIT6)    //Receive interrupt
	{
		Fifo_Status=nRF24L01_X_read_reg(DEV_0,HS6200_BANK0_FIFO_STATUS);
		pipe_num=(g_Dev_Status & nRF24L01_RX_P_NO)>>1;   //read pipe num	
		if( (Fifo_Status & nRF24L01_FIFO_STATUS_RX_EMPTY) ) LED2_ON;
        
		while(!(Fifo_Status & nRF24L01_FIFO_STATUS_RX_EMPTY))  //FIFO status not empty
		{	
			if(pipe_num<6)  //7 RX FIFO is empty,6 is reserved
			{
				if ( nRF24L01_X_is_prx(DEV_0) )  //PRX
				{
//					if(nRF24L01_X_is_auto_ack_px(DEV_0, pipe_num) && nRF24L01_X_is_dpl_feature(DEV_0) && nRF24L01_X_is_dpl_px(DEV_0, pipe_num))  //dyanmic length 
                    if(nRF24L01_X_is_dpl_feature(DEV_0) && nRF24L01_X_is_dpl_px(DEV_0, pipe_num))  //dyanmic length
					{
						plw=nRF24L01_X_read_rx_payload_width(DEV_0);  //read rx payload width
					}
					else		   	//static length 
					{						
						plw=nRF24L01_X_read_rx_pipe_static_pay_load_width(DEV_0,pipe_num);
					}					
					if(plw>32)
					{
						plw=32;   //plw>32 err, HS6200 bug
// 					    nRF24L01_X_flush_rx(DEV_0);  //flush rx
					}
					
					nRF24L01_X_read_rx_payload(DEV_0,Payload,plw);
					if(plw!=nRF24L01_X_Rx_Payload_Width[DEV_0][pipe_num])   //plw 不等
					{
						nRF24L01_X_Rx_Payload_Width[DEV_0][pipe_num]=plw;  //update plw
						nRF24L01_X_Rx_Buf[DEV_0][pipe_num][0]=pipe_num;
						for(i=0x00;i<plw;i++)nRF24L01_X_Rx_Buf[DEV_0][pipe_num][i+1]=Payload[i];
					}
					else  //plw相等
					{
						nRF24L01_X_Rx_Buf[DEV_0][pipe_num][0]=pipe_num; //update pipenum
						for(i=0x00;i<plw;i++)
						{
							if(Payload[i]!=nRF24L01_X_Rx_Buf[DEV_0][pipe_num][i+1]) 
							{
								
								/////////**********////////////////////控制输出
//								LED2_ON;								
								break;
							}
						}
						for(i=0x00;i<plw;i++)nRF24L01_X_Rx_Buf[DEV_0][pipe_num][i+1]=Payload[i];   //updata payload
					}	
					if(plw>0x00)
					{
						nRF24L01_X_PipeNum[DEV_0]=pipe_num;
						USB_ACK_2_Host(DEV_0,TYPE_PLD,&nRF24L01_X_Rx_Buf[DEV_0][pipe_num][0],nRF24L01_X_Rx_Payload_Width[DEV_0][pipe_num]+0x01);				
						
					}
				}
				else   //PTX (ACK Rx interrupt) 	//PTX带载的才会有此中断
				{
					if( nRF24L01_X_is_dpl_feature(DEV_0) && nRF24L01_X_is_auto_ack_px(DEV_0,0) && nRF24L01_X_is_dpl_px(DEV_0, PIPE0))   //动长，自动应答，带载
					{
						plw = nRF24L01_X_read_rx_payload_width(DEV_0);  //ACK payload width						
						if (plw > 32)  //ACK payload err
						{
							plw=32;
	//						nRF24L01_X_flush_rx(DEV_0);  //flush rx
						}
						nRF24L01_X_read_rx_payload(DEV_0,Payload,plw);
						
						if(plw!=nRF24L01_X_Tx_ACK_Payload_Width[DEV_0]) //连续接收 plw不相等
						{
							nRF24L01_X_Tx_ACK_Payload_Width[DEV_0]=plw;
							nRF24L01_X_Tx_ACK_Payload_Buf[DEV_0][0]=Ack_Pipe_Num; //只有在通过通道发送时，才给上位机回通道号，其他情况回PIPE0
							Ack_Pipe_Num=PIPE0;
							for(i=0x00;i<plw;i++)nRF24L01_X_Tx_ACK_Payload_Buf[DEV_0][1+i]=Payload[i];   //update payload
						}
						else  //plw 与上次接收相等
						{
							nRF24L01_X_Tx_ACK_Payload_Buf[DEV_0][0]=Ack_Pipe_Num; //update pipenum  //只有在通过通道发送时，才给上位机回通道号，其他情况回PIPE0
							Ack_Pipe_Num=PIPE0;  
							for(i=0x00;i<plw;i++)
							{
								if(Payload[i]!=nRF24L01_X_Tx_ACK_Payload_Buf[DEV_0][i+1]) 
								{								
									/////////**********////////////////////控制输出
									//LED2_ON;
									break;
								}
							}
							for(i=0x00;i<plw;i++)nRF24L01_X_Tx_ACK_Payload_Buf[DEV_0][i+1]=Payload[i];   //updata payload							
						}	
						if(plw>0x00)
						{
							USB_ACK_2_Host(DEV_0,TYPE_PLD,&nRF24L01_X_Tx_ACK_Payload_Buf[DEV_0][0],plw+0x01);   //plw+0x01指的是 pipe num 
							USB_Ack_Flag=1;
						}
					}
				}			
			}
            else if(pipe_num==7)    // 如果STATUS寄存器和FIFO_STATUS寄存器反应的状态不一致:
            {                       // 由于在读status寄存器时，FIFO没有数据，而在读status和FIFO status寄存器之间的时间内，RF接收到数据
                LED2_ON;            // 此时FIFO status显示有数据，status寄存器显示没有数据。这种情况就会死在中断里。修改成当两者的状态反应的不一致时，
                                    // 再次读取status和FIFO status寄存器。      
            }
            Dev_Status=nRF24L01_X_nop(DEV_0); //read register STATUS's value
			nRF24L01_X_write_reg(DEV_0, nRF24L01_STATUS, Dev_Status);// 中断位为1则回写正好清除之.clear RX_DR interrupt	
			pipe_num=(Dev_Status & nRF24L01_RX_P_NO)>>1;   //read pipe num			
			Fifo_Status=nRF24L01_X_read_reg(DEV_0,HS6200_BANK0_FIFO_STATUS);	
		 } 			
	}
	if(g_Dev_Status & BIT5)	 //transmit interrupt
	{
	 	nRF24L01_X_Busy[DEV_0]=0;
		if( (nRF24L01_X_is_ptx(DEV_0)) && (USB_Ack_Flag==0x00) ) USB_ACK_2_Host(DEV_0,0,NULL,0);	   //PTX的情况下使用TX_PAYLOAD NO ACK的情况下
		else USB_Ack_Flag=0;
	}	
    if(g_Dev_Status & BIT4)    //maximum retransmit interrupt
	{
		if(flush_tx_when_max_rt)
		{
			 nRF24L01_X_flush_tx(DEV_0);	
		}	
	  nRF24L01_X_Busy[DEV_0]=0;  //transmit complete
	}

    //
    HS6200_Bank0_Activate(DEV_0);
	Dev_Status=nRF24L01_X_nop(DEV_0); //read register STATUS's value
	nRF24L01_X_write_reg(DEV_0, nRF24L01_STATUS, Dev_Status);// 中断位为1则回写正好清除之.clear MAX_RT interrupt
	
	if(Cal_after_Ack)	    //先清中断，然后再进行软复位
	{
		if(nRF24L01_X_is_ptx(DEV_0))   //PTX
		{
// 			nRF24L01_X_ce_low(DEV_0);
            if ( (Dev_Flag[DEV_0]==HS6200_DEV) && (nRF24L01_X_read_reg(DEV_0, nRF24L01_EN_AA) & nRF24L01_EN_AA_P0) )  //Not SB mode 静无答  
			{							
				HS6200_Soft_Rst(DEV_0);  //soft reset
				nRF24L01_X_flush_rx(DEV_0);
				nRF24L01_X_flush_tx(DEV_0);
//				HS6200_Calibration(DEV_0);   //触发一次校准  已完成切换到BANK0    PRX模式  //触发一次校准  已完成切换到BANK0    PRX模式----》不需要进行校准了，因为现在校准值已经直接写入了			
			}
		}			
	}
}

/*
 * HS6200/nRF24L01 DEV1 接收中断程序
 */
void HS6200_DEV1_Int(void)
{
	U8 pipe_num=0x00;
	U8 plw=0x00;
	U8 i=0x00;
	U8 Dev_Status;
	U8 USB_Ack_Flag=0x00;
	U8 Temp[3];
	U8 Fifo_Status;
	U8 Payload[32];
	U8 g_Dev_Status;

	g_Dev_Status=  RF_Status;  // nRF24L01_X_nop(DEV_1); //// read register STATUS's value

	USB_ACK_Diagnose_Info(DEV_1);  //从USB回应诊断信息

	if(g_Dev_Status & BIT6)    //Receive interrupt
	{
		Fifo_Status=nRF24L01_X_read_reg(DEV_1,HS6200_BANK0_FIFO_STATUS);		
        pipe_num=(g_Dev_Status & nRF24L01_RX_P_NO)>>1;   //read pipe num
		if( (Fifo_Status & nRF24L01_FIFO_STATUS_RX_EMPTY) ) LED1_ON;   //如果STATUS寄存器和FIFO_STATUS寄存器反应的状态不一致  
        while(!(Fifo_Status & nRF24L01_FIFO_STATUS_RX_EMPTY))
		{
            if(pipe_num<6)  //7 RX FIFO is empty
			{	
				if ( nRF24L01_X_is_prx(DEV_1) )  //PRX
				{
					//if (nRF24L01_X_is_auto_ack_px(DEV_1, pipe_num) && nRF24L01_X_is_dpl_feature(DEV_1) && nRF24L01_X_is_dpl_px(DEV_1, pipe_num))  //dyanmic length 
                    if (nRF24L01_X_is_dpl_feature(DEV_1) && nRF24L01_X_is_dpl_px(DEV_1, pipe_num))  //dyanmic length 
					{
						plw=nRF24L01_X_read_rx_payload_width(DEV_1);  //read rx payload width
					}
					else 		//static length
					{
						plw=nRF24L01_X_read_rx_pipe_static_pay_load_width(DEV_1,pipe_num);    //read payload width   static payload width			
					}					
					
					if(plw>32)
					{
						plw=32;
 						nRF24L01_X_flush_rx(DEV_1);  //flush rx
					}
					nRF24L01_X_read_rx_payload(DEV_1,Payload,plw);
					
					if(plw!=nRF24L01_X_Rx_Payload_Width[DEV_1][pipe_num])  //plw不相等
					{
						nRF24L01_X_Rx_Payload_Width[DEV_1][pipe_num]=plw;  //update plw
						nRF24L01_X_Rx_Buf[DEV_1][pipe_num][0]=pipe_num;
						for(i=0x00;i<plw;i++)nRF24L01_X_Rx_Buf[DEV_1][pipe_num][i+1]=Payload[i];
					}
					else  //plw相等
					{
						nRF24L01_X_Rx_Buf[DEV_1][pipe_num][0]=pipe_num; //update pipenum
						for(i=0x00;i<plw;i++)
						{
							if(Payload[i]!=nRF24L01_X_Rx_Buf[DEV_1][pipe_num][i+1])
							{
								//////////////***********///////////////////控制输出
								//LED1_ON;
								break;
							}	
						}
						for(i=0x00;i<plw;i++)nRF24L01_X_Rx_Buf[DEV_1][pipe_num][i+1]=Payload[i];   //updata payload
					}
					if(plw>0x00)
					{
						nRF24L01_X_PipeNum[DEV_1]=pipe_num;
						USB_ACK_2_Host(DEV_1,TYPE_PLD,&nRF24L01_X_Rx_Buf[DEV_1][pipe_num][0],nRF24L01_X_Rx_Payload_Width[DEV_1][pipe_num]+0x01);
					}
				}
				else   //PTX (ACK Rx interrupt) 
				{
					if( nRF24L01_X_is_dpl_feature(DEV_1) && nRF24L01_X_is_auto_ack_px(DEV_1,0) &&  nRF24L01_X_is_dpl_px(DEV_1, PIPE0))   //动长，自动应答，带载
					{
						plw = nRF24L01_X_read_rx_payload_width(DEV_1);  //ACK payload width									
						if (plw > 32)  //ACK payload err
						{
							plw=32;
 							nRF24L01_X_flush_rx(DEV_1);  //flush rx
						}
						nRF24L01_X_read_rx_payload(DEV_1,Payload,plw);
						
						if(plw!=nRF24L01_X_Tx_ACK_Payload_Width[DEV_0]) //连续接收 plw不相等
						{
							nRF24L01_X_Tx_ACK_Payload_Width[DEV_1]=plw;
							nRF24L01_X_Tx_ACK_Payload_Buf[DEV_1][0]=Ack_Pipe_Num;  //只有在通过通道发送时，才给上位机回通道号，其他情况回PIPE0
							Ack_Pipe_Num=PIPE0; 
							for(i=0x00;i<plw;i++)nRF24L01_X_Tx_ACK_Payload_Buf[DEV_1][1+i]=Payload[i];   //update payload
						}
						else  //plw 与上次接收相等
						{
							nRF24L01_X_Tx_ACK_Payload_Buf[DEV_1][0]=Ack_Pipe_Num; //update pipenum //只有在通过通道发送时，才给上位机回通道号，其他情况回PIPE0
							Ack_Pipe_Num=PIPE0;
							for(i=0x00;i<plw;i++)
							{
								if(Payload[i]!=nRF24L01_X_Tx_ACK_Payload_Buf[DEV_1][i+1]) 
								{								
									/////////**********////////////////////控制输出
									//LED1_ON;
									break;
								}
							}
							for(i=0x00;i<plw;i++)nRF24L01_X_Tx_ACK_Payload_Buf[DEV_1][i+1]=Payload[i];   //updata payload							
						}			
						if(plw>0x00)  //有载，回显payload
						{
							USB_ACK_2_Host(DEV_1,TYPE_PLD,&nRF24L01_X_Tx_ACK_Payload_Buf[DEV_1][0],plw+0x01);
							USB_Ack_Flag=1;
						}
					}
				}
			}
            else if(pipe_num==7)    // 如果STATUS寄存器和FIFO_STATUS寄存器反应的状态不一致:
            {                       // 由于在读status寄存器时，FIFO没有数据，而在读status和FIFO status寄存器之间的时间内，RF接收到数据
                LED1_ON;            // 此时FIFO status显示有数据，status寄存器显示没有数据。这种情况就会死在中断里。修改成当两者的状态反应的不一致时，
                                    // 再次读取status和FIFO status寄存器。      
            }
            Dev_Status=nRF24L01_X_nop(DEV_1); //read register STATUS's value
			nRF24L01_X_write_reg(DEV_1, nRF24L01_STATUS, Dev_Status);// 中断位为1则回写正好清除之.clear RX_DR interrupt	
			pipe_num=(Dev_Status & nRF24L01_RX_P_NO)>>1;   //read pipe num
			Fifo_Status=nRF24L01_X_read_reg(DEV_1,HS6200_BANK0_FIFO_STATUS);
		}	
    }	
	if(g_Dev_Status & BIT5)	 //transmit interrupt
	{
	 	nRF24L01_X_Busy[DEV_1]=0; 
		
		if( ( nRF24L01_X_is_ptx(DEV_1) ) && (USB_Ack_Flag==0x00)) USB_ACK_2_Host(DEV_1,0,NULL,0); 	  //PTX 的情况下使用TX_PAYLOAD_NOACK
		else USB_Ack_Flag=0x00;
	}	
    if(g_Dev_Status & BIT4)    //maximum retransmit interrupt
	{
		if(flush_tx_when_max_rt)
		{
			 nRF24L01_X_flush_tx(DEV_1);	
		}
	  	USB_ACK_2_Host(DEV_1,TYPE_REG,Temp,3);
		nRF24L01_X_Busy[DEV_1]=0;  //transmit complete
	}
    
	HS6200_Bank0_Activate(DEV_1);
	Dev_Status=nRF24L01_X_nop(DEV_1); //read register STATUS's value
	nRF24L01_X_write_reg(DEV_1, nRF24L01_STATUS, Dev_Status);	  // 中断位为1则回写正好清除之。
       
    if(Cal_after_Ack)		    //先清中断，然后再进行软复位
	{
		if(nRF24L01_X_is_ptx(DEV_1))   //PTX
		{
// 			nRF24L01_X_ce_low(DEV_1);
            if ( (Dev_Flag[DEV_1]==HS6200_DEV) && (nRF24L01_X_read_reg(DEV_1, nRF24L01_EN_AA) & nRF24L01_EN_AA_P0) ) //Not SB mode 静无答  
			{							
				HS6200_Soft_Rst(DEV_1);  //soft reset
				nRF24L01_X_flush_rx(DEV_1);
				nRF24L01_X_flush_tx(DEV_1);
//				HS6200_Calibration(DEV_1);   //触发一次校准  已完成切换到BANK0    PRX模式----》不需要进行校准了，因为现在校准值已经直接写入了		
			}
		}			
	}	
}

/*
 * 1. EN_RXADDR=0x3F, 使能接收通道.
 * 2. Rx_PW_Px均都设置完毕.
 * 3. DYNPD: 对Rx来说，EN_DPL,DPL_Px: pipex 启用动长机制. 对Tx来说，EN_DPL, DPL_P0启用总体动长机制
 * 4. EN_AA:  对Rx来说，EN_AA_Px: pipex启用自动应答机制. 对Tx来说,EN_AA_P0启用总体动长机制   
 */
void HS6200_Mode_Config(U8 DevNum,U8 Config)
{
	U8 Reg_Feature=0x00;
	U8 Reg_ENAA=0x00;

	HS6200_Bank0_Activate(DevNum);
	Reg_Feature=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_FEATURE);
	Reg_ENAA=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_EN_AA);
	switch(Config)
	{
		/*
		 *   EN_DPL  DYNPD   EN_ACK_PAY       EN_DYN_ACK       EN_AA
		 *     0   	   0     	0                0              0
		 */
		case COMMODE_SPL_SA_NAK_ZZ:				  // 静长静无答
			Reg_Feature&=~nRF24L01_EN_DPL;
			Reg_Feature&=~nRF24L01_EN_ACK_PAY;
			Reg_Feature&=~nRF24L01_EN_DYN_ACK;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FEATURE,Reg_Feature);
			
			Reg_ENAA&=~nRF24L01_EN_AA_P0;  		//Tx: EN_AA_P0=0.  
			if(nRF24L01_X_is_prx(DevNum)) Reg_ENAA=0x00; //Rx: EN_AA_Px=0			
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_AA,Reg_ENAA);

			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_DYNPD,0x00); 		  
		break;

		/*
		 *   EN_DPL  DYNPD   EN_ACK_PAY       EN_DYN_ACK       EN_AA
		 *     0   	   0	 	0                0              1
		 */
		case COMMODE_SPL_SA_ACK_AE:			      //  静长静空答 
			Reg_Feature&=~nRF24L01_EN_DPL;
			Reg_Feature&=~nRF24L01_EN_ACK_PAY;
			Reg_Feature&=~nRF24L01_EN_DYN_ACK;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FEATURE,Reg_Feature);
			
			Reg_ENAA|=nRF24L01_EN_AA_P0;
			if( nRF24L01_X_is_prx(DevNum) )Reg_ENAA=0x3F;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_AA,Reg_ENAA);
			
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_DYNPD,0x00); 	
		break;

		/*
		 *   EN_DPL   DYNPD  EN_ACK_PAY       EN_DYN_ACK       EN_AA
		 *     0   		0   	0                1              1
		 */						  
		case COMMODE_SPL_DA_ACK_AE:               // 静长动空答 	
			Reg_Feature&=~nRF24L01_EN_DPL;
			Reg_Feature&=~nRF24L01_EN_ACK_PAY;
			Reg_Feature|=nRF24L01_EN_DYN_ACK;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FEATURE,Reg_Feature);
			
			Reg_ENAA|=nRF24L01_EN_AA_P0;
			if( nRF24L01_X_is_prx(DevNum) )Reg_ENAA=0x3F;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_AA,Reg_ENAA); 
			
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_DYNPD,0x00); 				
		break;

   		/*
		 *   EN_DPL   DYNPD   EN_ACK_PAY       EN_DYN_ACK       EN_AA
		 *     1   	    1 	      0                0              1
		 */
		case COMMODE_DPL_SA_NAK_ZZ:                // 动长静空答
			Reg_Feature|=nRF24L01_EN_DPL;
			Reg_Feature&=~nRF24L01_EN_ACK_PAY;
			Reg_Feature&=~nRF24L01_EN_DYN_ACK;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FEATURE,Reg_Feature);
			
			Reg_ENAA|=nRF24L01_EN_AA_P0;
			if( nRF24L01_X_is_prx(DevNum) )Reg_ENAA=0x3F;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_AA,Reg_ENAA); 
			
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_DYNPD,0x3F); 	
		break;

		/*
		 *   EN_DPL   DYNPD  EN_ACK_PAY       EN_DYN_ACK       EN_AA
		 *     1   	    1	     1                0             1
		 */
		case COMMODE_DPL_SA_ACK_AP:				   //动长静载答
			Reg_Feature|=nRF24L01_EN_DPL;
			Reg_Feature|=nRF24L01_EN_ACK_PAY;
			Reg_Feature&=~nRF24L01_EN_DYN_ACK;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FEATURE,Reg_Feature);
			
			Reg_ENAA|=nRF24L01_EN_AA_P0;
			if( nRF24L01_X_is_prx(DevNum) )Reg_ENAA=0x3F;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_AA,Reg_ENAA);
			
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_DYNPD,0x3F); 			 	
		break;

		/*
		 *   EN_DPL   DYNPD  EN_ACK_PAY       EN_DYN_ACK       EN_AA
		 *     1   		1   	0                1              1
		 */
		case COMMODE_DPL_DA_ACK_AE:				   // 动长动空答
			Reg_Feature|=nRF24L01_EN_DPL;
			Reg_Feature&=~nRF24L01_EN_ACK_PAY;
			Reg_Feature|=nRF24L01_EN_DYN_ACK;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FEATURE,Reg_Feature);
			
			Reg_ENAA|=nRF24L01_EN_AA_P0;
			if( nRF24L01_X_is_prx(DevNum) )	Reg_ENAA=0x3F;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_AA,Reg_ENAA);  

			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_DYNPD,0x3F);				
		break;

		/*
		 *   EN_DPL   DYNPD  EN_ACK_PAY   EN_DYN_ACK   EN_AA
		 *     1   		1   	1             1          1
		 */										  
		case COMMODE_DPL_DA_ACK_AP:				   // 动长动载答
			Reg_Feature|=nRF24L01_EN_DPL;
			Reg_Feature|=nRF24L01_EN_ACK_PAY;
			Reg_Feature|=nRF24L01_EN_DYN_ACK;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_FEATURE,Reg_Feature);
			
			Reg_ENAA|=nRF24L01_EN_AA_P0;
			if( nRF24L01_X_is_prx(DevNum) )	Reg_ENAA=0x3F;
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_EN_AA,Reg_ENAA);
			
			nRF24L01_X_write_reg(DevNum,HS6200_BANK0_DYNPD,0x3F); 			
		break;
		default:
		break;
	}	
}








/*---------------------------------------------End Of File---------------------------------------------*/
