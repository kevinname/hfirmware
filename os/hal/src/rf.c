/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012,2013 Giovanni Di Sirio.
		 Copyright (C) 2014 HunterSun Technologies
                 shangzhou.hu@huntersun.com.cn
    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes ChibiOS/RT, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/
#include <string.h>
#include "hal.h"
#include "chprintf.h"

#if HAL_USE_RF || defined(__DOXYGEN__)


RFAddress RFAddr_4Bit = {
  .Addr_len = 4,
  .Addr[0]  = 0x48,
  .Addr[1]  = 0x54, 
  .Addr[2]  = 0x78,
  .Addr[3]  = 0x79,
};


RFAddress RFAddr_5Bit = {
  .Addr_len = 5,
  .Addr[0]  = 0x46,
  .Addr[1]  = 0x0b, 
  .Addr[2]  = 0xaf,
  .Addr[3]  = 0x43,
  .Addr[4]  = 0x98,
};

RFDriver MAC6200_default_config = {
  .RX_Addr  = &RFAddr_5Bit,
  .TX_Addr  = &RFAddr_5Bit,
  .Power    = RF_POWER_0dBm,
  .DataRate = RF_DR_1Mbps,
  .Mode     = COMMODE_SPL_SA_NAK_ZZ, 
  .Role     = COMROLE_PTX,
  .CRC      = COMCRC_DISABLE,
  .Chanle   = 37,  //max2829 default chanle is 2437
  .Length   = 32,
  .id_rf    = HS_MAC6200,
};

void MAC6200_set_freq(unsigned int freq)
{
  uint8_t reg_value = freq-2400;
  MAC6200_CE_Low();
  MAC6200_Bank0_Activate();
  MAC6200_Write_Reg(MAC6200_BANK0_RF_CH, &reg_value, 1);
  MAC6200_CE_High();
}

void MAC6200_dump_RF_register(BaseSequentialStream *chp)
{
  uint8_t reg_value;
  uint8_t temp[5]; 
  int8_t i;
  
  MAC6200_Bank0_Activate();
    
  MAC6200_Read_Reg(MAC6200_BANK0_CONFIG, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_CONFIG: %02x\r\n", reg_value);  
  
  MAC6200_Read_Reg(MAC6200_BANK0_EN_AA, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_EN_AA: %02x\r\n", reg_value); 
  
  MAC6200_Read_Reg(MAC6200_BANK0_EN_RXADDR, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_EN_RXADDR: %02x\r\n", reg_value); 

  MAC6200_Read_Reg(MAC6200_BANK0_SETUP_AW, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_SETUP_AW: %02x\r\n", reg_value); 

  MAC6200_Read_Reg(MAC6200_BANK0_SETUP_RETR, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_SETUP_RETR: %02x\r\n", reg_value);  
  
  MAC6200_Read_Reg(MAC6200_BANK0_RF_CH, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RF_CH: %02x\r\n", reg_value); 
  
  MAC6200_Read_Reg(MAC6200_BANK0_RF_SETUP, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RF_SETUP: %02x\r\n", reg_value); 

  MAC6200_Read_Reg(MAC6200_BANK0_STATUS, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_STATUS: %02x\r\n", reg_value); 
  
  MAC6200_Read_Reg(MAC6200_BANK0_OBSERVE_TX, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_OBSERVE_TX: %02x\r\n", reg_value); 
  
  MAC6200_Read_Reg(MAC6200_BANK0_RPD, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RPD: %02x\r\n", reg_value); 

  MAC6200_Read_Reg(MAC6200_BANK0_RX_ADDR_P0, temp, 5);
  chprintf(chp, "MAC6200_BANK0_RX_ADDR_P0: "); 
  for(i=4;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");
  
  MAC6200_Read_Reg(MAC6200_BANK0_RX_ADDR_P1, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RX_ADDR_P1: %02x\r\n", reg_value);  
  
  MAC6200_Read_Reg(MAC6200_BANK0_RX_ADDR_P2, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RX_ADDR_P2: %02x\r\n", reg_value); 
  
  MAC6200_Read_Reg(MAC6200_BANK0_RX_ADDR_P3, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RX_ADDR_P3: %02x\r\n", reg_value); 

  MAC6200_Read_Reg(MAC6200_BANK0_RX_ADDR_P4, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RX_ADDR_P4: %02x\r\n", reg_value);
  
  MAC6200_Read_Reg(MAC6200_BANK0_RX_ADDR_P5, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RX_ADDR_P5: %02x\r\n", reg_value);  
  
  MAC6200_Read_Reg(MAC6200_BANK0_TX_ADDR, temp, 5);
  chprintf(chp, "MAC6200_BANK0_TX_ADDR: "); 
  for(i=4;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");
  
  MAC6200_Read_Reg(MAC6200_BANK0_RX_PW_P0, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RX_PW_P0: %02x\r\n", reg_value); 

  MAC6200_Read_Reg(MAC6200_BANK0_RX_PW_P1, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RX_PW_P1: %02x\r\n", reg_value); 

  MAC6200_Read_Reg(MAC6200_BANK0_RX_PW_P2, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RX_PW_P2: %02x\r\n", reg_value);  
  
  MAC6200_Read_Reg(MAC6200_BANK0_RX_PW_P3, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RX_PW_P3: %02x\r\n", reg_value); 
  
  MAC6200_Read_Reg(MAC6200_BANK0_RX_PW_P4, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RX_PW_P4: %02x\r\n", reg_value); 

  MAC6200_Read_Reg(MAC6200_BANK0_RX_PW_P5, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_RX_PW_P5: %02x\r\n", reg_value); 
  
  MAC6200_Read_Reg(MAC6200_BANK0_FIFO_STATUS, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_FIFO_STATUS: %02x\r\n", reg_value);  
  
  MAC6200_Read_Reg(MAC6200_BANK0_DYNPD, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_DYNPD: %02x\r\n", reg_value); 
  
  MAC6200_Read_Reg(MAC6200_BANK0_FEATURE, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_FEATURE: %02x\r\n", reg_value); 


  MAC6200_Read_Reg(MAC6200_BANK0_SETUP_VALUE, temp, 5);
  chprintf(chp, "MAC6200_BANK0_SETUP_VALUE: ");
  for(i=4;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");
  
  
  MAC6200_Read_Reg(MAC6200_BANK0_PRE_GURD, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK0_PRE_GURD: %02x\r\n\n\n", reg_value);  
  
  MAC6200_Bank1_Activate();

  MAC6200_Read_Reg(MAC6200_BANK1_LINE, temp, 2);
  chprintf(chp, "MAC6200_BANK1_LINE: ");
  for(i=1;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n"); 
  
  MAC6200_Read_Reg(MAC6200_BANK1_PLL_CTL0, temp, 4);
  chprintf(chp, "MAC6200_BANK1_PLL_CTL0: ");
  for(i=3;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");  
  
  MAC6200_Read_Reg(MAC6200_BANK1_PLL_CTL1, temp, 4);
  chprintf(chp, "MAC6200_BANK1_PLL_CTL1: ");
  for(i=3;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");

  MAC6200_Read_Reg(MAC6200_BANK1_CAL_CTL, temp, 5);
  chprintf(chp, "MAC6200_BANK1_CAL_CTL: ");
  for(i=4;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");

  MAC6200_Read_Reg(MAC6200_BANK1_A_CNT_REG, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_A_CNT_REG: %02x\r\n", reg_value);   

  MAC6200_Read_Reg(MAC6200_BANK1_B_CNT_REG, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_B_CNT_REG: %02x\r\n", reg_value);  

  MAC6200_Read_Reg(MAC6200_BANK1_RESERVED1, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_RESERVED1: %02x\r\n", reg_value);  

  MAC6200_Read_Reg(MAC6200_BANK1_STATUS, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_STATUS: %02x\r\n", reg_value);  

  MAC6200_Read_Reg(MAC6200_BANK1_STATE, temp, 2);
  chprintf(chp, "MAC6200_BANK1_STATE: ");
  for(i=1;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");  

  MAC6200_Read_Reg(MAC6200_BANK1_CHAN, temp, 4);
  chprintf(chp, "MAC6200_BANK1_CHAN: ");
  for(i=3;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");  
  
  MAC6200_Read_Reg(MAC6200_BANK1_IF_FREQ, temp, 3);
  chprintf(chp, "MAC6200_BANK1_IF_FREQ: ");
  for(i=2;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");

  MAC6200_Read_Reg(MAC6200_BANK1_AFC_COR, temp, 3);
  chprintf(chp, "MAC6200_BANK1_AFC_COR: ");
  for(i=2;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");

  MAC6200_Read_Reg(MAC6200_BANK1_FDEV, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_FDEV: %02x\r\n", reg_value);   

  MAC6200_Read_Reg(MAC6200_BANK1_DAC_RANGE, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_DAC_RANGE: %02x\r\n", reg_value);  

  MAC6200_Read_Reg(MAC6200_BANK1_DAC_IN, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_DAC_IN: %02x\r\n", reg_value);

  MAC6200_Read_Reg(MAC6200_BANK1_CTUNING, temp, 2);
  chprintf(chp, "HS6206_BANK1_CTUNING: ");
  for(i=1;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n"); 

  MAC6200_Read_Reg(MAC6200_BANK1_FTUNING, temp, 2);
  chprintf(chp, "MAC6200_BANK1_FTUNING: ");
  for(i=1;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n"); 
  
  MAC6200_Read_Reg(MAC6200_BANK1_RX_CTRL, temp, 4);
  chprintf(chp, "MAC6200_BANK1_RX_CTRL: ");
  for(i=3;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");  
  
  MAC6200_Read_Reg(MAC6200_BANK1_FAGC_CTRL, temp, 4);
  chprintf(chp, "MAC6200_BANK1_FAGC_CTRL: ");
  for(i=3;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");

  MAC6200_Read_Reg(MAC6200_BANK1_FAGC_CTRL_1, temp, 4);
  chprintf(chp, "MAC6200_BANK1_FAGC_CTRL_1: ");
  for(i=3;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");

  MAC6200_Read_Reg(MAC6200_BANK1_DAC_CAL_LOW, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_DAC_CAL_LOW: %02x\r\n", reg_value);   

  MAC6200_Read_Reg(MAC6200_BANK1_DAC_CAL_HI, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_DAC_CAL_HI: %02x\r\n", reg_value);  

  MAC6200_Read_Reg(MAC6200_BANK1_RESERVED3, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_RESERVED3: %02x\r\n", reg_value);  

  MAC6200_Read_Reg(MAC6200_BANK1_DOC_DACI, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_DOC_DACI: %02x\r\n", reg_value);  

  MAC6200_Read_Reg(MAC6200_BANK1_DOC_DACQ, &reg_value, 1);
  chprintf(chp, "MAC6200_BANK1_DOC_DACQ: %02x\r\n", reg_value);   

  MAC6200_Read_Reg(MAC6200_BANK1_AGC_CTRL, temp, 4);
  chprintf(chp, "MAC6200_BANK1_AGC_CTRL: ");
  for(i=3;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n"); 
  
  MAC6200_Read_Reg(MAC6200_BANK1_AGC_GAIN, temp, 4);
  chprintf(chp, "MAC6200_BANK1_AGC_GAIN: ");
  for(i=3;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");  
  
  MAC6200_Read_Reg(MAC6200_BANK1_RF_IVGEN, temp, 4);
  chprintf(chp, "MAC6200_BANK1_RF_IVGEN: ");
  for(i=3;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");

  MAC6200_Read_Reg(MAC6200_BANK1_TEST_PKDET, temp, 4);
  chprintf(chp, "MAC6200_BANK1_TEST_PKDET: ");
  for(i=3;i>=0;i--)
    chprintf(chp, "%02x " , temp[i]);
  chprintf(chp, "\r\n");  
  MAC6200_Bank0_Activate();
}

void MAC6206_set_sar_ldo() {
   *(volatile unsigned int*)(0x40020000|0x205<<2) = 0x1 ;
   *(volatile unsigned int*)(0x40020000|0x14c<<2) = 0x0 ;
   *(volatile unsigned int*)(0x40020000|0x144<<2) = 2500 ;
   *(volatile unsigned int*)(0x40020000|0x147<<2) = 0x10 ;
   *(volatile unsigned int*)(0x40020000|0x149<<2) = 0x08 ;
   *(volatile unsigned int*)(0x40020000|0x12a<<2) = 0xfffe ;
}


#endif /* HAL_USE_RF */
