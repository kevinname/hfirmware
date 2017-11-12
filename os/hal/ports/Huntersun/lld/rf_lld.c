/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 shangzhou.hu@huntersun.com.cn

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
/*
   Concepts and parts of this file have been contributed by Uladzimir Pylinsky
   aka barthess.
 */

/**
 * @file    hs66xx/rf_lld.c
 * @brief   HS66xx RF subsystem low level driver header.
 *
 * @addtogroup RF
 * @{
 */

//#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#if HAL_USE_RF || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief RF driver identifier.
 */
 RFDriver MAC6200;
static volatile uint32_t RF_SPI_flags=0x03;
volatile uint8_t RF_Status=0x00;
/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/**
 * @brief   Select MAC6200 as rf PHY.
 *
 * @notapi
 */
void rf_lld_Slect_MAC6200(void)
{
  cpmEnableBTPHY();
  HS_MAC6200->SPIRCON |= RF_MAC_SELECT;
}

/**
 * @brief   Change RF's SPI CSN to high.
 *
 * @notapi
 */
void rf_lld_CSN_High(void)  
{
  HS_MAC6200->RFCON |= RF_CSN;
}

/**
 * @brief   Change RF's SPI CSN to low.
 *
 * @notapi
 */
void rf_lld_CSN_Low(void) 
{
  HS_MAC6200->RFCON &= ~RF_CSN;
}

/**
 * @brief   Change CE to high.
 *
 * @notapi
 */
void rf_lld_CE_High(void)
{
  HS_MAC6200->RFCON |= RF_CE;
}

/**
 * @brief   Change CE to low.
 *
 * @notapi
 */
void rf_lld_CE_Low()
{
  HS_MAC6200->RFCON &= ~RF_CE;
}

/**
 * @brief  generate a CE high pulse.
 *
 * @notapi
 */
void rf_lld_CE_high_pulse(void)  
{                          
  rf_lld_CE_High();            
  chThdSleepMicroseconds(20);  
  rf_lld_CE_Low();          
}

/**
 * @brief  RFSPI write/read 1 Byte to/from RF .
 *
 * @notapi
 */
static uint8_t RF_SPI_WR_Byte_Poll(uint8_t Byte)
{
    while(!(HS_MAC6200->SPIRSTAT & RF_TXFIFO_READY));      //Tx Fifo not ready.
    HS_MAC6200->SPIRDAT=Byte;
    while(!(HS_MAC6200->SPIRSTAT & RF_RXDATA_READY));      //Rx Fifo has data.
    return HS_MAC6200->SPIRDAT;
}

/**
 * @brief  RFSPI write/read 1 Byte to/from RF  using interrupt.
 *
 * @notapi
 */
static uint8_t RF_SPI_WR_Byte_Int(uint8_t Byte)
{
    while(!(RF_SPI_flags & RF_TXFIFO_READY));      //Tx Fifo not ready.
    HS_MAC6200->SPIRDAT=Byte;
    while(!(RF_SPI_flags & RF_RXDATA_READY));      //Rx Fifo has data.
    return HS_MAC6200->SPIRDAT;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   RF Write register.
 *
 * @notapi
 */
void rf_lld_Wr_Reg(uint8_t Reg, uint8_t *Reg_val, uint8_t len)
{
  uint8_t i;
  
  rf_lld_CSN_Low(); 
  RF_SPI_WR_Byte_Poll(MAC6200_W_REGISTER+Reg);
  for(i=0; i<len; i++) {
    RF_SPI_WR_Byte_Poll(*Reg_val++);  
  }
  rf_lld_CSN_High();  
}

/**
 * @brief   RF Write register.
 *
 * @notapi
 */
void rf_lld_Rd_Reg(uint8_t Reg, uint8_t *Reg_val, uint8_t len)
{
  uint8_t i;
  
  rf_lld_CSN_Low(); 
  RF_SPI_WR_Byte_Poll(MAC6200_R_REGISTER+Reg);
  for(i=0; i<len; i++) {
    Reg_val[i] = RF_SPI_WR_Byte_Poll(0x00);  
  }
  rf_lld_CSN_High();  
}

/**
 * @brief   RF Write register using interrupt.
 *
 * @notapi
 */
void rf_lld_Wr_Reg_int(uint8_t Reg, uint8_t *Reg_val, uint8_t len)
{
  uint8_t i;
  
  rf_lld_CSN_Low(); 
  RF_SPI_WR_Byte_Int(MAC6200_W_REGISTER+Reg);
  for(i=0; i<len; i++) {
    RF_SPI_WR_Byte_Int(*Reg_val++);  
  }
  rf_lld_CSN_High();  
}

/**
 * @brief   RF Write register using interrupt.
 *
 * @notapi
 */
void rf_lld_Rd_Reg_int(uint8_t Reg, uint8_t *Reg_val, uint8_t len)
{
  uint8_t i;
  
  rf_lld_CSN_Low(); 
  RF_SPI_WR_Byte_Int(MAC6200_R_REGISTER+Reg);
  for(i=0; i<len; i++) {
    Reg_val[i] = RF_SPI_WR_Byte_Int(0x00);  
  }
  rf_lld_CSN_High();  
}

/**
 * @brief  switch bank0.
 *
 * @notapi
 */
void rf_lld_Bank0_Activate(void)
{
  uint8_t status;
  
  rf_lld_Rd_Reg(MAC6200_BANK0_STATUS, &status, 0x01);
  if((status & MAC6200_BANK_BIT) == MAC6200_BANK1){
    rf_lld_CSN_Low(); 
    RF_SPI_WR_Byte_Poll(MAC6200_ACTIVATE);
    RF_SPI_WR_Byte_Poll(0x53);
    rf_lld_CSN_High(); 
  }
}

/**
 * @brief  switch bank1.
 *
 * @notapi
 */
void rf_lld_Bank1_Activate(void)
{
  uint8_t status;
  
  rf_lld_Rd_Reg(MAC6200_BANK0_STATUS, &status, 0x01);
  if((status & MAC6200_BANK_BIT) == MAC6200_BANK0){
    rf_lld_CSN_Low(); 
    RF_SPI_WR_Byte_Poll(MAC6200_ACTIVATE);
    RF_SPI_WR_Byte_Poll(0x53);
    rf_lld_CSN_High(); 
  }
}


void rf_lld_Read_Rx_Payload(uint8_t *pBuf, uint8_t bytes)
{
  uint8_t i;
  rf_lld_CSN_Low();   //拉低 
  RF_SPI_WR_Byte_Poll(MAC6200_R_RX_PAYLOAD);
  for(i=0x00;i<bytes;i++)	pBuf[i] = RF_SPI_WR_Byte_Poll(0x00);
  rf_lld_CE_High();
}

void rf_lld_Write_Tx_Payload(uint8_t *pBuf, uint8_t bytes)
{
  uint8_t i;
  rf_lld_CSN_Low();   //拉低
  RF_SPI_WR_Byte_Poll(MAC6200_W_TX_PAYLOAD);
  for(i=0x00;i<bytes;i++)
  {
    RF_SPI_WR_Byte_Poll(pBuf[i]);
  }
  rf_lld_CSN_High();	
}

void rf_lld_Flush_Tx_Fifo(void)
{
  rf_lld_CSN_Low();   //拉低
  RF_SPI_WR_Byte_Poll(MAC6200_FLUSH_TX);    // send command
  rf_lld_CSN_High();                   // CSN high again
}

void rf_lld_Flush_Rx_Fifo(void)
{
  rf_lld_CSN_Low();   //拉低
  RF_SPI_WR_Byte_Poll(MAC6200_FLUSH_RX);    // send command
  rf_lld_CSN_High();                   // CSN high again
}

uint8_t rf_lld_Read_Rx_Payload_Width(void)
{
  uint8_t Width;
  rf_lld_CSN_Low();   //拉低
  RF_SPI_WR_Byte_Poll(MAC6200_R_RX_PL_WID); 	
  Width=RF_SPI_WR_Byte_Poll(0x00);
  rf_lld_CSN_High();
  return Width; 
}

void rf_lld_Write_Ack_Payload(uint8_t PipeNum, uint8_t *pBuf, uint8_t bytes)
{
    uint8_t byte_ctr;
    rf_lld_CSN_Low();   //拉低
    RF_SPI_WR_Byte_Poll(MAC6200_W_ACK_PAYLOAD | PipeNum); 
    for(byte_ctr=0; byte_ctr<bytes; byte_ctr++)	   // then write all byte in buffer(*pBuf)
    {                          
       RF_SPI_WR_Byte_Poll(*pBuf++);
    }
    rf_lld_CSN_High();			
}

void rf_lld_Write_Tx_Payload_No_Ack(uint8_t *pBuf, uint8_t bytes)
{
  uint8_t byte_ctr;
  rf_lld_CSN_Low();   //拉低
  RF_SPI_WR_Byte_Poll(MAC6200_W_TX_PAYLOAD_NOACK); 	 
  for(byte_ctr=0; byte_ctr<bytes; byte_ctr++) // then write all byte in buffer(*pBuf)
  {                          
    RF_SPI_WR_Byte_Poll(*pBuf++);
  }
  rf_lld_CSN_High();			
}

uint8_t rf_lld_Nop(void)
{
  uint8_t status;
  rf_lld_CSN_Low();
  status = RF_SPI_WR_Byte_Poll(MAC6200_NOP);
  rf_lld_CSN_High();
  return (status); 
}

void rf_lld_Set_Addr_Width(uint8_t Addr_Width)
{
  uint8_t Temp;
  switch (Addr_Width)
  {
  //    case 0x03:       //Not support 3 Bytes
  //        Temp=MAC6200_AW_3_BYTES;
  //	break;
    case 0x04:
        Temp=MAC6200_AW_4_BYTES;
    break;
    case 0x05:
        Temp=MAC6200_AW_5_BYTES;
    break;
    default:
        Temp=0x00;
    break;
  }	
  rf_lld_Wr_Reg(MAC6200_BANK0_SETUP_AW,&Temp,1);
}

void rf_lld_Write_Pipe_Addr(uint8_t Px_Addr_Reg, uint8_t *pPipeAddr, uint8_t Addr_Width)
{
  uint8_t byte_ctr; 
  
  rf_lld_CSN_Low();                                      // CSN low, init SPI transaction
  RF_SPI_WR_Byte_Poll(MAC6200_W_REGISTER + Px_Addr_Reg); // Select register to write to and read status byte
  for(byte_ctr=0; byte_ctr<Addr_Width; byte_ctr++)       // then write all byte in buffer(*pBuf)
      RF_SPI_WR_Byte_Poll(*pPipeAddr++);
  rf_lld_CSN_High();                                     // CSN high again                                                           
}

void rf_lld_Read_Pipe_Addr(uint8_t Px_Addr_Reg, uint8_t *pPipeAddr, uint8_t Addr_Width)
{
  uint8_t byte_ctr;
  
  rf_lld_CSN_Low();                                     // CSN low, init SPI transaction
  RF_SPI_WR_Byte_Poll(MAC6200_R_REGISTER + Px_Addr_Reg);// Select register to write to and read status byte
  for(byte_ctr=0; byte_ctr<Addr_Width; byte_ctr++)      // then write all byte in buffer(*pBuf)
      *pPipeAddr++=RF_SPI_WR_Byte_Poll(0x00);
  rf_lld_CSN_High();                                     // CSN high again                                                          
}

uint8_t rf_lld_Is_Auto_Ack_Px(uint8_t Pipe)
{
  uint8_t Temp;
  Temp=0x00;
  rf_lld_Rd_Reg(MAC6200_BANK0_EN_AA,&Temp,1); 
  Temp=Temp & (1<<Pipe);
  return Temp;
}


uint8_t rf_lld_Is_Dyn_Ack_Feature(void)
{
  uint8_t Temp;
  rf_lld_Rd_Reg(MAC6200_BANK0_FEATURE,&Temp,1);
  return  (Temp&0x01);
}

uint8_t rf_lld_Is_DPL_Feature(void)   //判断是否是动长
{
  uint8_t Temp;
  rf_lld_Rd_Reg(MAC6200_BANK0_FEATURE,&Temp,1);
  return  (Temp&0x04);	
}

uint8_t rf_lld_Read_Rx_Pipe_Static_Payload_Width(uint8_t Pipe_Num)
{
  uint8_t Temp;
  rf_lld_Rd_Reg(MAC6200_BANK0_RX_PW_P0+Pipe_Num,&Temp,1);
  return Temp;
}

bool rf_lld_Is_PRX(void)
{
  uint8_t temp;
  
  rf_lld_Rd_Reg(MAC6200_BANK0_CONFIG, &temp, 1);
  if(temp&MAC6200_PRX)
    return TRUE;
  else 
    return FALSE;
}

/*
 * 1. EN_RXADDR=0x3F, 使能接收通道.
 * 2. Rx_PW_Px均都设置完毕.
 * 3. DYNPD: 对Rx来说，EN_DPL,DPL_Px: pipex 启用动长机制. 对Tx来说，EN_DPL, DPL_P0启用总体动长机制
 * 4. EN_AA:  对Rx来说，EN_AA_Px: pipex启用自动应答机制. 对Tx来说,EN_AA_P0启用总体动长机制   
 */
static void rf_lld_Mode_Config(RFCommuicationMode_t Config)
{
  uint8_t Reg_Feature=0x00;
  uint8_t Reg_ENAA=0x00;
  
  rf_lld_Bank0_Activate();
  rf_lld_Rd_Reg(MAC6200_BANK0_FEATURE, &Reg_Feature, 1);
  rf_lld_Rd_Reg(MAC6200_BANK0_EN_AA, &Reg_ENAA, 1);
  switch(Config)
  {
    /*
    *   EN_DPL  DYNPD   EN_ACK_PAY       EN_DYN_ACK       EN_AA
    *     0   	 0     	   0                0              0
    */
    case COMMODE_SPL_SA_NAK_ZZ:				  // 静长静无答
      Reg_Feature&=~MAC6200_EN_DPL;
      Reg_Feature&=~MAC6200_EN_ACK_PAY;
      Reg_Feature&=~MAC6200_EN_DYN_ACK;
      rf_lld_Wr_Reg(MAC6200_BANK0_FEATURE, &Reg_Feature, 1);
      
      Reg_ENAA&=~MAC6200_EN_AA_P0;  		//Tx: EN_AA_P0=0.  
      Reg_ENAA=0x00; //Rx: EN_AA_Px=0			
      rf_lld_Wr_Reg(MAC6200_BANK0_EN_AA, &Reg_ENAA, 1);
      Reg_Feature = 0x00;
      rf_lld_Wr_Reg(MAC6200_BANK0_DYNPD, &Reg_Feature, 1); 		  
    break;
    
    /*
    *   EN_DPL  DYNPD   EN_ACK_PAY       EN_DYN_ACK       EN_AA
    *     0   	   0	   0                0              1
    */
    case COMMODE_SPL_SA_ACK_AE:			      //  静长静空答 
      Reg_Feature&=~MAC6200_EN_DPL;
      Reg_Feature&=~MAC6200_EN_ACK_PAY;
      Reg_Feature&=~MAC6200_EN_DYN_ACK;
      rf_lld_Wr_Reg(MAC6200_BANK0_FEATURE, &Reg_Feature, 1);
      
      Reg_ENAA|=MAC6200_EN_AA_P0;
      if( rf_lld_Is_PRX() )Reg_ENAA=0x3F;
      rf_lld_Wr_Reg(MAC6200_BANK0_EN_AA, &Reg_ENAA, 1);
      Reg_Feature = 0x00;
      rf_lld_Wr_Reg(MAC6200_BANK0_DYNPD, &Reg_Feature, 1); 
    break;
    
    /*
    *   EN_DPL   DYNPD  EN_ACK_PAY       EN_DYN_ACK       EN_AA
    *     0   	  0   	   0                1              1
    */						  
    case COMMODE_SPL_DA_ACK_AE:               // 静长动空答 	
      Reg_Feature&=~MAC6200_EN_DPL;
      Reg_Feature&=~MAC6200_EN_ACK_PAY;
      Reg_Feature|=MAC6200_EN_DYN_ACK;
      rf_lld_Wr_Reg(MAC6200_BANK0_FEATURE, &Reg_Feature, 1);
      
      Reg_ENAA|=MAC6200_EN_AA_P0;
      if( rf_lld_Is_PRX() )Reg_ENAA=0x3F;
      rf_lld_Wr_Reg(MAC6200_BANK0_EN_AA, &Reg_ENAA, 1); 
      Reg_Feature = 0x00;
      rf_lld_Wr_Reg(MAC6200_BANK0_DYNPD, &Reg_Feature, 1); 				
    break;
    
    /*
    *   EN_DPL   DYNPD   EN_ACK_PAY       EN_DYN_ACK       EN_AA
    *     1   	    1 	      0                0              1
    */
    case COMMODE_DPL_SA_NAK_ZZ:                // 动长静无答
    case COMMODE_MUL_SLAVE_NO_ACK:             // 一主多从用动长静无答模式测试
    case COMMODE_DPL_SA_NAK_ZZ_CRC_1Bit:       // 用动长静无答模式测试1bit C
      Reg_Feature|=MAC6200_EN_DPL;
      Reg_Feature&=~MAC6200_EN_ACK_PAY;
      Reg_Feature&=~MAC6200_EN_DYN_ACK;
      rf_lld_Wr_Reg(MAC6200_BANK0_FEATURE, &Reg_Feature, 1);
      
      Reg_ENAA|=MAC6200_EN_AA_P0;
      Reg_ENAA=0x00;
      rf_lld_Wr_Reg(MAC6200_BANK0_EN_AA, &Reg_ENAA, 1); 
      Reg_Feature = 0x3f;
      rf_lld_Wr_Reg(MAC6200_BANK0_DYNPD, &Reg_Feature, 1);	
    break;
    
    /*
    *   EN_DPL   DYNPD  EN_ACK_PAY       EN_DYN_ACK       EN_AA
    *     1   	    1	     1                0             1
    */
    case COMMODE_DPL_SA_ACK_AP:				   //动长静载答
      Reg_Feature|=MAC6200_EN_DPL;
      Reg_Feature|=MAC6200_EN_ACK_PAY;
      Reg_Feature&=~MAC6200_EN_DYN_ACK;
      rf_lld_Wr_Reg(MAC6200_BANK0_FEATURE, &Reg_Feature, 1);
      
      Reg_ENAA|=MAC6200_EN_AA_P0;
      if( rf_lld_Is_PRX() )Reg_ENAA=0x3F;
      rf_lld_Wr_Reg(MAC6200_BANK0_EN_AA, &Reg_ENAA, 1);
      Reg_Feature = 0x3f;
      rf_lld_Wr_Reg(MAC6200_BANK0_DYNPD, &Reg_Feature, 1);		 	
    break;
    
    /*
    *   EN_DPL   DYNPD  EN_ACK_PAY       EN_DYN_ACK       EN_AA
    *     1   	  1   	   0                1              1
    */
    case COMMODE_DPL_DA_ACK_AE:				   // 动长动空答
      Reg_Feature|=MAC6200_EN_DPL;
      Reg_Feature&=~MAC6200_EN_ACK_PAY;
      Reg_Feature|=MAC6200_EN_DYN_ACK;
      rf_lld_Wr_Reg(MAC6200_BANK0_FEATURE, &Reg_Feature, 1);
      
      Reg_ENAA|=MAC6200_EN_AA_P0;
      if( rf_lld_Is_PRX() )	Reg_ENAA=0x3F;
      rf_lld_Wr_Reg(MAC6200_BANK0_EN_AA, &Reg_ENAA, 1);  
      Reg_Feature = 0x3f;
      rf_lld_Wr_Reg(MAC6200_BANK0_DYNPD, &Reg_Feature, 1);				
    break;
    
    /*
    *   EN_DPL   DYNPD  EN_ACK_PAY   EN_DYN_ACK   EN_AA
    *     1   	  1   	  1             1          1
    */										  
    case COMMODE_DPL_DA_ACK_AP:		         // 动长动载答
    case COMMODE_MUL_SLAVE_ACK:              // 一主多从用动长动载答模式测试
      Reg_Feature|=MAC6200_EN_DPL;
      Reg_Feature|=MAC6200_EN_ACK_PAY;
      Reg_Feature|=MAC6200_EN_DYN_ACK;
      rf_lld_Wr_Reg(MAC6200_BANK0_FEATURE, &Reg_Feature, 1);
      
      Reg_ENAA|=MAC6200_EN_AA_P0;
      if( rf_lld_Is_PRX() )	Reg_ENAA=0x3F;
      rf_lld_Wr_Reg(MAC6200_BANK0_EN_AA, &Reg_ENAA, 1);
      Reg_Feature = 0x3f;
      rf_lld_Wr_Reg(MAC6200_BANK0_DYNPD, &Reg_Feature, 1);		
    break;
    default:
    break;
  }	
}

void MAC6200_Calibration(void)
{
	uint8_t temp[5];
	rf_lld_Bank0_Activate();
	temp[0]=0x03;
	rf_lld_Wr_Reg(MAC6200_BANK0_CONFIG,temp, 1);

	temp[0]=0x32;
	rf_lld_Wr_Reg(MAC6200_BANK0_RF_CH, temp,1);

	temp[0]=0x40;       //1MHz速率,发射功率-18dBm.
    rf_lld_Wr_Reg(MAC6200_BANK0_RF_SETUP,temp,1); 

  
	rf_lld_Bank1_Activate();
	temp[0]=0x40;
	temp[1]=0x00;
	temp[2]=0x10;
 	temp[3]=0xE4;
	rf_lld_Wr_Reg(MAC6200_BANK1_PLL_CTL0, temp, 4);



	temp[0]=0x00;
	temp[1]=0x00;
	temp[2]=0x1F;
	rf_lld_Wr_Reg(MAC6200_BANK1_IF_FREQ, temp, 3);
	
	temp[0]=0x14;
	rf_lld_Wr_Reg(MAC6200_BANK1_FDEV, temp, 1);

    rf_lld_Bank0_Activate();
	chThdSleepMilliseconds(2);
	rf_lld_CE_high_pulse();
	chThdSleepMilliseconds(40);   //delay 150ms waitting for calibaration. 

}


void MAC6200_Analog_Init(void)	   //MAC6200 初始化
{	
    uint8_t  temp[5];  
    
    rf_lld_CE_Low();

	rf_lld_Bank0_Activate();

	temp[0]=0x28;
	temp[1]=0x32;
	temp[2]=0x80;                      
	temp[3]=0x06;
	temp[4]=0x00;
	rf_lld_Wr_Reg(MAC6200_BANK0_SETUP_VALUE,temp, 5);


    rf_lld_Bank1_Activate();
    
	temp[0]=0x40;
	temp[1]=0x01;               
	temp[2]=0x30;                
	temp[3]=0xE0;                
	   
	 rf_lld_Wr_Reg(MAC6200_BANK1_PLL_CTL0, temp,4);

	temp[0]=0x00;
	temp[1]=0x42;
	temp[2]=0x10;                      
	temp[3]=0x00;
	rf_lld_Wr_Reg(MAC6200_BANK1_PLL_CTL1, temp,4);

	temp[0]=0x29;
	temp[1]=0x89;
	temp[2]=0x75;                      
	temp[3]=0x28;
	temp[4]=0x50;
	rf_lld_Wr_Reg(MAC6200_BANK1_CAL_CTL, temp,5);
    

    
    temp[0]=0xFF;
    temp[1]=0x1F;
    temp[2]=0x08;    
    temp[3]=0x29;
    rf_lld_Wr_Reg(MAC6200_BANK1_FAGC_CTRL_1, temp,4);

    temp[0]=0x02;
    temp[1]=0xC1;
    temp[2]=0xCB;  
    temp[3]=0x1C;
    rf_lld_Wr_Reg(MAC6200_BANK1_AGC_GAIN, temp,4);

    temp[0]=0x97; 
    temp[1]=0x64;
    temp[2]=0x00;
    temp[3]=0x01;
    rf_lld_Wr_Reg(MAC6200_BANK1_RF_IVGEN, temp,4);
	rf_lld_Bank0_Activate();
}

void rf_lld_Init(RFDriver *RFConfig)
{
  uint8_t reg_value = 0x00;
  uint8_t temp[5];
  
  MAC6200.id_rf = HS_MAC6200;
 
  RFConfig->id_rf->SPIRCON |= RF_MAC_SELECT;  //RF PHY Select MAC6200
 
  RFConfig->id_rf->SPIRCON |= (RF_IRQ_RXFIFO_FULL + RF_IRQ_RXDATA_READY   \
            +RF_IRQ_TxFIFO_EMPTY + RF_IRQ_TXFIFO_READY);  //disable all spi interrupt
#ifndef RUN_RTL_IN_SERVER 
  MAC6200_Calibration();
  MAC6200_Analog_Init();
#endif
  if(RFConfig->Role == COMROLE_PRX)   //PTX or PRX
    reg_value |= MAC6200_PRX;
  else
    reg_value &= ~MAC6200_PRX;
  
  if(RFConfig->CRC == COMCRC_DISABLE)   //disable CRC
     reg_value &= ~MAC6200_CRCO;
  else if(RFConfig->CRC == COMCRC_1BIT) {  //enable CRC, CRC 1bit
     reg_value |= MAC6200_EN_CRC;
     reg_value &= ~MAC6200_CRCO_2_BYTES;
  }
  else if(RFConfig->CRC == COMCRC_2BIT) {  //enable CRC, CRC 2bit
    reg_value |= MAC6200_EN_CRC;
    reg_value |= MAC6200_CRCO_2_BYTES;
  }
  reg_value |= MAC6200_PWR_UP;           //Power up
  reg_value |= 0x80;

  rf_lld_Wr_Reg(MAC6200_BANK0_CONFIG, &reg_value, 1);
  
  reg_value = 0x3f;           
  rf_lld_Wr_Reg(MAC6200_BANK0_EN_RXADDR, &reg_value, 1);  //enable all RX pipe
  
  reg_value = 0x28;           
  rf_lld_Wr_Reg(MAC6200_BANK0_SETUP_RETR, &reg_value, 1);  //disable translate auto retry
  
  if(RFConfig->RX_Addr->Addr_len == 5) {   //5bit address
     reg_value = MAC6200_AW_5_BYTES;
     rf_lld_Wr_Reg(MAC6200_BANK0_SETUP_AW, &reg_value, 1);
     rf_lld_Wr_Reg(MAC6200_BANK0_RX_ADDR_P0, RFConfig->RX_Addr->Addr, 5);
     rf_lld_Wr_Reg(MAC6200_BANK0_TX_ADDR, RFConfig->TX_Addr->Addr, 5);
  }
  else  {                                  //4bit address
     reg_value = MAC6200_AW_4_BYTES;
     rf_lld_Wr_Reg(MAC6200_BANK0_SETUP_AW, &reg_value, 1);
     rf_lld_Wr_Reg(MAC6200_BANK0_RX_ADDR_P0, RFConfig->RX_Addr->Addr, 4);
     rf_lld_Wr_Reg(MAC6200_BANK0_TX_ADDR, RFConfig->RX_Addr->Addr, 4);  
  }

  reg_value = RFConfig->Chanle;           
  rf_lld_Wr_Reg(MAC6200_BANK0_RF_CH, &reg_value, 1);  //set up chanle
  
  reg_value = RFConfig->Length;           
  rf_lld_Wr_Reg(MAC6200_BANK0_RX_PW_P0, &reg_value, 1);  //set up payload length

  reg_value = (uint8_t)RFConfig->Power | (uint8_t)RFConfig->DataRate;           
  rf_lld_Wr_Reg(MAC6200_BANK0_RF_SETUP, &reg_value, 1);  //set up payload length
    
  rf_lld_Mode_Config(RFConfig->Mode);
  
  rf_lld_Flush_Rx_Fifo();
  rf_lld_Flush_Tx_Fifo();
  
  reg_value = 0x70;           
  rf_lld_Wr_Reg(MAC6200_BANK0_STATUS, &reg_value, 1);
  
  temp[0] = 0x3c;
  temp[1] = 0x4b;
  temp[2] = 0xff;
  temp[3] = 0x10;
  if(RFConfig->Role == COMROLE_PRX)  
     temp[4] = 0x01; 
  else  
     temp[4] = 0x6f;
  rf_lld_Wr_Reg(MAC6200_BANK0_SETUP_VALUE, temp, 5);
  
  MAC6200_Bank1_Activate();

  temp[0] = 0x20;
  temp[1] = 0x08;
  temp[2] = 0x54;
  temp[3] = 0x2b;
  temp[4] = 0x78;
  rf_lld_Wr_Reg(MAC6200_BANK1_CAL_CTL, temp, 5);  

  temp[0] = 0x0f;
  temp[1] = 0x28;
  temp[2] = 0x6f;
  temp[3] = 0x00;
  rf_lld_Wr_Reg(MAC6200_BANK1_PLL_CTL1, temp, 5);  
  
  MAC6200_Bank0_Activate();
  
   /* IRQ vector permanently assigned to this driver.*/
  nvicEnableVector(IRQ_6200_RF, ANDES_PRIORITY_MASK(HS_RF_INTR_PRIORITY));
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
/**
 * @brief   RF interrupt handler.
 *
 * @isr
 */
//CH_IRQ_HANDLER(MAC6200_RF_IRQHandler)
//{
//  CH_IRQ_PROLOGUE();
//  MAC6200_Read_Reg(MAC6200_BANK0_STATUS, (uint8_t *)&RF_Status, 1); //read RF status register
//  if(RF_Status & MAC6200_MAX_RT) {
//    MAC6200_Flush_Tx_Fifo();
//    MAC6200_Flush_Rx_Fifo();    
//  }  
//  MAC6200_Write_Reg(MAC6200_BANK0_STATUS, (uint8_t *)&RF_Status, 1); //clear rf interrupt
//  CH_IRQ_EPILOGUE();
//}
static void (*MAC6200_event)(void);
static void MAC6200_serve_interrupt(void)
{
  MAC6200_Read_Reg(MAC6200_BANK0_STATUS, (uint8_t *)&RF_Status, 1); //read RF status register
  if(RF_Status & MAC6200_MAX_RT) {
    MAC6200_Flush_Tx_Fifo();
    MAC6200_Flush_Rx_Fifo();    
  }  
  MAC6200_Write_Reg(MAC6200_BANK0_STATUS, (uint8_t *)&RF_Status, 1); //clear rf interrupt  
  if(MAC6200_event != NULL)
    (*MAC6200_event)();
}

CH_IRQ_HANDLER(MAC6200_RF_IRQHandler) {

  CH_IRQ_PROLOGUE();
  MAC6200_serve_interrupt();
  CH_IRQ_EPILOGUE();
}

void MAC6200_lld_reg_event(void (*event)(void))
{
    MAC6200_event = event;
}

/**
 * @brief   RF interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(MAC6200_SPI_IRQHandler)
{
  CH_IRQ_PROLOGUE();

  /* Mask of all enabled and pending sources.*/
  RF_SPI_flags = HS_MAC6200->SPIRSTAT;
  HS_MAC6200->ICR |= RF_ICR;   //clear spi interrupt
  CH_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver for MAX2829.                                                */
/*===========================================================================*/
#define MAX2829_IF0_ENABLE 1

//(channel index+2402)*4/3/20
 
#define FREQ_INT_IF0 \
0x20A0, 0x30A0, 0xA0  , 0x10A0, 0x20A0, 0x30A0, 0x10A0, 0x20A0, 0x30A0, 0x40A0, \
0x10A0, 0x20A0, 0x30A0, 0xA1  , 0x10A1, 0x20A1, 0x30A1, 0xA1  , 0x10A1, 0x20A1, \
0x30A1, 0x10A1, 0x20A1, 0x30A1, 0x40A1, 0x10A1, 0x20A1, 0x30A1, 0xA2  , 0x10A2, \
0x20A2, 0x30A2, 0xA2  , 0x10A2, 0x20A2, 0x30A2, 0x10A2, 0x20A2, 0x30A2, 0x40A2, \
0x10A2, 0x20A2, 0x30A2, 0xA3  , 0x10A3, 0x20A3, 0x30A3, 0xA3  , 0x10A3, 0x20A3, \
0x30A3, 0x10A3, 0x20A3, 0x30A3, 0x40A3, 0x10A3, 0x20A3, 0x30A3, 0xA4  , 0x10A4, \
0x20A4, 0x30A4, 0xA4  , 0x10A4, 0x20A4, 0x30A4, 0x10A4, 0x20A4, 0x30A4, 0x40A4, \
0x10A4, 0x20A4, 0x30A4, 0xA5  , 0x10A5, 0x20A5, 0x30A5, 0xA5  , 0x10A5,
 
#define FREQ_INT_IF750k \
0x10A0, 0x20A0, 0x30A0, 0x10A0, 0x20A0, 0x30A0, 0x40A0, 0x10A0, 0x20A0, 0x30A0, \
0xA0  , 0x10A0, 0x20A0, 0x30A0, 0xA1  , 0x10A1, 0x20A1, 0x30A1, 0x10A1, 0x20A1, \
0x30A1, 0x40A1, 0x10A1, 0x20A1, 0x30A1, 0xA1  , 0x10A1, 0x20A1, 0x30A1, 0xA2  , \
0x10A2, 0x20A2, 0x30A2, 0x10A2, 0x20A2, 0x30A2, 0x40A2, 0x10A2, 0x20A2, 0x30A2, \
0xA2  , 0x10A2, 0x20A2, 0x30A2, 0xA3  , 0x10A3, 0x20A3, 0x30A3, 0x10A3, 0x20A3, \
0x30A3, 0x40A3, 0x10A3, 0x20A3, 0x30A3, 0xA3  , 0x10A3, 0x20A3, 0x30A3, 0xA4  , \
0x10A4, 0x20A4, 0x30A4, 0x10A4, 0x20A4, 0x30A4, 0x40A4, 0x10A4, 0x20A4, 0x30A4, \
0xA4  , 0x10A4, 0x20A4, 0x30A4, 0xA5  , 0x10A5, 0x20A5, 0x30A5, 0x10A5,
 
#define FREQ_FRA_IF0 \
0x888 , 0xCCC , 0x1111, 0x1555, 0x1999, 0x1DDD, 0x2222, 0x2666, 0x2AAA, 0x2EEE, \
0x3333, 0x3777, 0x3BBB, 0x0   , 0x444 , 0x888 , 0xCCC , 0x1111, 0x1555, 0x1999, \
0x1DDD, 0x2222, 0x2666, 0x2AAA, 0x2EEE, 0x3333, 0x3777, 0x3BBB, 0x0   , 0x444 , \
0x888 , 0xCCC , 0x1111, 0x1555, 0x1999, 0x1DDD, 0x2222, 0x2666, 0x2AAA, 0x2EEE, \
0x3333, 0x3777, 0x3BBB, 0x0   , 0x444 , 0x888 , 0xCCC , 0x1111, 0x1555, 0x1999, \
0x1DDD, 0x2222, 0x2666, 0x2AAA, 0x2EEE, 0x3333, 0x3777, 0x3BBB, 0x0   , 0x444 , \
0x888 , 0xCCC , 0x1111, 0x1555, 0x1999, 0x1DDD, 0x2222, 0x2666, 0x2AAA, 0x2EEE, \
0x3333, 0x3777, 0x3BBB, 0x0   , 0x444 , 0x888 , 0xCCC , 0x1111, 0x1555,
 
#define FREQ_FRA_IF750k \
0x555 , 0x999 , 0xDDD , 0x1222, 0x1666, 0x1AAA, 0x1EEE, 0x2333, 0x2777, 0x2BBB, \
0x3000, 0x3444, 0x3888, 0x3CCC, 0x111 , 0x555 , 0x999 , 0xDDD , 0x1222, 0x1666, \
0x1AAA, 0x1EEE, 0x2333, 0x2777, 0x2BBB, 0x3000, 0x3444, 0x3888, 0x3CCC, 0x111 , \
0x555 , 0x999 , 0xDDD , 0x1222, 0x1666, 0x1AAA, 0x1EEE, 0x2333, 0x2777, 0x2BBB, \
0x3000, 0x3444, 0x3888, 0x3CCC, 0x111 , 0x555 , 0x999 , 0xDDD , 0x1222, 0x1666, \
0x1AAA, 0x1EEE, 0x2333, 0x2777, 0x2BBB, 0x3000, 0x3444, 0x3888, 0x3CCC, 0x111 , \
0x555 , 0x999 , 0xDDD , 0x1222, 0x1666, 0x1AAA, 0x1EEE, 0x2333, 0x2777, 0x2BBB, \
0x3000, 0x3444, 0x3888, 0x3CCC, 0x111 , 0x555 , 0x999 , 0xDDD , 0x1222,
 
const uint16_t Freq_Int_Reg_Rx[]={               /*-750K frequency offset */
         FREQ_INT_IF750k
};
const uint16_t Freq_Fra_Reg_Rx[]={               /*-750K frequency offset */
         FREQ_FRA_IF750k
};
 
#if MAX2829_IF0_ENABLE     /*0-IF enable*/
const uint16_t Freq_Int_Reg_Tx[]= {              /*offset 0*/
         FREQ_INT_IF0
};
const uint16_t Freq_Fra_Reg_Tx[]={               /*offset 0*/
         FREQ_FRA_IF0
};
#else  /* MAX2829_IF0_ENABLE */
const uint16_t Freq_Int_Reg_Tx[]={               /*-750K frequency offset */
         FREQ_INT_IF750k
};
const uint16_t Freq_Fra_Reg_Tx[]={               /*-750K frequency offset */
         FREQ_FRA_IF750k
};
#endif /* MAX2829_IF0_ENABLE */
/*******************************************************************************
 * The following is a define the location of GIO lines in the MAX2829
 *
 * GIO 0   used to control TXENA of MAX2829.
 * GIO 1   used to control RXENA of MAX2829.
 * GIO 2   used to control #SHDN of MAX2829.
 * 
 * GIO 6   used to control TXENA in page/inquiry mode
 * GIO 7   used to control RXENA in page/inquiry mode
 *
 *******************************************************************************/
#define GIO_HIGH_TXENA                         0x40018030
#define GIO_LOW_TXENA                          0x40018048
#define GIO_OVERRIDE_MASK_TXENA                0x00008000


#define GIO_HIGH_RXENA                         0x40018030
#define GIO_LOW_RXENA                          0x40018048
#define GIO_OVERRIDE_MASK_RXENA                0x80000000 

#define GIO_HIGH_SHDN                          0x40018034
#define GIO_LOW_SHDN                           0x4001804C
#define GIO_OVERRIDE_MASK_SHDN                 0x00008000

#define FREQ_2497_INT   0x30a6
#define FREQ_2497_FRA   0x1ddd

#define FREQ_2497_25_INT   0x30a6
#define FREQ_2497_25_FRA   0x1eee


#define FREQ_2498_INT   0x10a6
#define FREQ_2498_FRA   0x2222

#define WRITE_REG3             WRITE_REG(0x03, FREQ_2497_25_INT) /*Intger-Divider Ratio*/
#define WRITE_REG4             WRITE_REG(0x04, FREQ_2497_25_FRA) /*Fractional-Divider Ratio*/

#define WRITE_REG(n,val)   ( 0x80000000 |((val)<<4)|(n) )
#define WRITE_REG0             WRITE_REG(0x00, 0x1140)   /*Register 0*/
#define WRITE_REG1             WRITE_REG(0x01, 0x00CA)   /*Register 1*/ 
#define WRITE_REG2             WRITE_REG(0x02, 0x1007)   /*standby */  
#define WRITE_REG3_TX(index)   WRITE_REG(0x03, Freq_Int_Reg_Tx[index])   /*Intger-Divider Ratio*/ 
#define WRITE_REG4_TX(index)   WRITE_REG(0x04, Freq_Fra_Reg_Tx[index])   /*Fractional-Divider Ratio*/
#define WRITE_REG3_RX(index)   WRITE_REG(0x03, Freq_Int_Reg_Rx[index])   /*Intger-Divider Ratio*/ 
#define WRITE_REG4_RX(index)   WRITE_REG(0x04, Freq_Fra_Reg_Rx[index])   /*Fractional-Divider Ratio*/
#define WRITE_REG5             WRITE_REG(0x05, 0x1824)   /*Band select and PLL*/      
#define WRITE_REG6             WRITE_REG(0x06, 0x1C00)   /*calibration*/
#define WRITE_REG7             WRITE_REG(0x07, 0x002A)   /*lowpass filter*/  
#define WRITE_REG8             WRITE_REG(0x08, 0x1C25)   /*Rx control/RSSI*/
#define WRITE_REG9             WRITE_REG(0x09, 0x0603)   /*Tx linearity/baseband gain*/
#define WRITE_REGA             WRITE_REG(0x0A, 0x03C0)   /*PA bias DAC*/
#define WRITE_REGB             WRITE_REG(0x0B, 0x0015)   /*Rx Gain*/
#define WRITE_REGC             WRITE_REG(0x0C, 0x003F)   /*Tx VGA Gain*/ 

#define HWradio_SetOverrideLow(GIO_NAME)  do { \
    *((volatile unsigned int*)GIO_LOW_##GIO_NAME) |= GIO_OVERRIDE_MASK_##GIO_NAME; \
    *((volatile unsigned int*)GIO_HIGH_##GIO_NAME) &= ~(GIO_OVERRIDE_MASK_##GIO_NAME); \
    }while(0)

#define HWradio_SetOverrideHigh(GIO_NAME)  do { \
    *((volatile unsigned int*)GIO_HIGH_##GIO_NAME) |= GIO_OVERRIDE_MASK_##GIO_NAME; \
    *((volatile unsigned int*)GIO_LOW_##GIO_NAME) &= ~(GIO_OVERRIDE_MASK_##GIO_NAME); \
    }while(0)

#define HWradio_SetGIOLowReg(GIO_NAME, value) \
     *((volatile unsigned int*)GIO_LOW_##GIO_NAME) = value;

#define HWradio_SetGIOHighReg(GIO_NAME, value) \
     *((volatile unsigned int*)GIO_HIGH_##GIO_NAME) = value;

static void _HWhab_Init_RF(void)    
{
  volatile uint32_t config_word = 0;

  //donot use autowake up    
  *(volatile unsigned int*)0x400180A0 = 0x00000000; 
  
  config_word = 0;  
  config_word |= (18 <<5); // 14bit data plus 4 bit address
  config_word |= (1<<10);  // clk pol - 1. data clocked out on rising edge.
  config_word |= (0<<11);  // data pol - 0. data not inverted 
  config_word |= (1<<12);  //serial enable -1
  config_word |= (1<<16);  // clk low - 1. number of refclk cycles for which SPI clk is low.
  config_word |= (1<<20);  // clk high -1. number of refclk cycles for which SPI clk is high.
  config_word |= (0<<23);  // clk byp.
  config_word |= (1<<24);  // sel pol - 1. active low select enable. 
  config_word |= (0<<27);  // set pol - 0. normal SPI mode.

  *(volatile unsigned int*)0x40016060 = config_word;

  config_word = (1 << 23);  //enable the ESER block
  config_word |= (0 << 0);  //mask -0
  config_word |= (1 << 20); //now -1
  config_word |= (0 << 21); // seq -0
 
  *(volatile unsigned int*)0x40016064  = config_word; 
  
  //GIO combines - Un combine everything 0x40018000
  *(volatile unsigned int*)0x40018060 = 0x00000000;  
  *(volatile unsigned int*)0x40018064 = 0x00000000;  
  *(volatile unsigned int*)0x40018068 = 0x00000000;  
  
  /*GIOs Low*/
  HWradio_SetOverrideLow(SHDN);            //SHDN low
  HWradio_SetGIOLowReg(SHDN,0x8000);
  HWradio_SetGIOLowReg(TXENA,0x8000);
  HWradio_SetGIOLowReg(RXENA,0x8000);

  /*GIOs High*/
  HWradio_SetGIOHighReg(SHDN,0x0000);
  HWradio_SetGIOHighReg(TXENA,0x0000);
  HWradio_SetGIOHighReg(RXENA,0x0000);
}

/*************************************************************************************
 *
 * FUNCTION NAME: _HWradio_ProgNow
 *
 * Use SPI NOW mode to prog a radio register. Due to MIPS consumptions on wait/polls
 * required in SPI NOW mode, this should be avoided where possible. Mainly suited to
 * radio initialisation.
 *
 *************************************************************************************/
static void _HWradio_ProgNow(uint32_t in_val)
{
    /* must wait if SPI bus is busy */
    while((*(volatile unsigned int*)0x40016060) & 0x80000000);

    /* specify data to write */
    *(volatile unsigned int*)0x40016600 = in_val;
 
    /* must wait if SPI bus is busy */
    while((*(volatile unsigned int*)0x40016060) & 0x80000000);
}
    
/*
 * MAX2829 go to standby state
 */
static void _HWradio_Go_To_Idle_State(void)   //MAX2829 go to standby state
{
  /*SPI reset mode*/
  HWradio_SetOverrideLow(SHDN);   
  HWradio_SetOverrideHigh(TXENA);
  HWradio_SetOverrideHigh(RXENA);
  
  /*standby mode*/
  HWradio_SetOverrideLow(TXENA);
  HWradio_SetOverrideLow(RXENA);
  HWradio_SetOverrideHigh(SHDN);
  
  //need to add test SER/ESER and RF
  /****************************
   * MAX2829 Initialise
   ****************************/
  _HWradio_ProgNow(WRITE_REG0);
  _HWradio_ProgNow(WRITE_REG1);
  _HWradio_ProgNow(WRITE_REG2);
  _HWradio_ProgNow(WRITE_REG3);
  _HWradio_ProgNow(WRITE_REG4);  //set freq 2497
  _HWradio_ProgNow(WRITE_REG5);
  _HWradio_ProgNow(WRITE_REG6);
  _HWradio_ProgNow(WRITE_REG7);
  #if MAX2829_FPGA_AGC_ENABLE
  _HWradio_ProgNow(WRITE_REG8_EXT_PIN_EN);
  #else
  _HWradio_ProgNow(WRITE_REG8);
  #endif
  _HWradio_ProgNow(WRITE_REG9);
  _HWradio_ProgNow(WRITE_REGA);
  _HWradio_ProgNow(WRITE_REGB);
  _HWradio_ProgNow(WRITE_REGC); 
  
//  _HWradio_ProgNow(WRITE_REG(0x03, 0xa5));       //configure the channel is 2484
//  _HWradio_ProgNow(WRITE_REG(0x04, 0x8ccc>>2)); 
}

void MAX2829_init(void)
{
  *(volatile unsigned int*)0x40018028 = 0x360062b8; 
  *(volatile unsigned int*)0x40018008 =  0x70000;   
  *(volatile unsigned int*)0x40018008 = 0x170000;   
  
  _HWhab_Init_RF();
  _HWradio_Go_To_Idle_State();  
}

void MAX2829_RxMode_init()
{
  *(volatile unsigned int*)0x40018028 = 0x360062b8; 
  *(volatile unsigned int*)0x40018008 = 0x70000;   
  *(volatile unsigned int*)0x40018008 = 0x170000;   
  
  HWradio_SetOverrideHigh(SHDN); 
  chThdSleepS(100); 
  HWradio_SetOverrideLow(TXENA); 
  chThdSleepS(100); 
  HWradio_SetOverrideHigh(RXENA); 
  chThdSleepS(100);
}

void rf_lld_MAX2829_RX(uint8_t channel)
{
  _HWradio_ProgNow(WRITE_REG3_RX(channel-2));
  _HWradio_ProgNow(WRITE_REG4_RX(channel-2));
  HWradio_SetOverrideHigh(SHDN);
  HWradio_SetOverrideLow(TXENA); 
  HWradio_SetOverrideHigh(RXENA);  
}
void rf_lld_MAX2829_TX(uint8_t channel)
{
  _HWradio_ProgNow(WRITE_REG3_TX(channel-2));
  _HWradio_ProgNow(WRITE_REG4_TX(channel-2));  
  HWradio_SetOverrideHigh(SHDN);
  HWradio_SetOverrideLow(RXENA); 
  HWradio_SetOverrideHigh(TXENA);    
}

void rf_lld_MAX2829_RX_gain(uint8_t gain)
{
  _HWradio_ProgNow(WRITE_REG(0x0B, gain));  //[0, 1f]
}

void rf_lld_MAX2829_TX_gain(uint8_t gain)
{
  _HWradio_ProgNow(WRITE_REG(0x0c, gain));  //[0, 3f]
}
#endif /* HAL_USE_RF */

/** @} */

