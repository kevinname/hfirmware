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
/*
   Concepts and parts of this file have been contributed by Uladzimir Pylinsky
   aka barthess.
 */

/**
 * @file    rf.h
 * @brief   RF Driver macros and structures.
 *
 * @addtogroup RF
 * @{
 */

#ifndef _RF_H_
#define _RF_H_

#if HAL_USE_RF || defined(__DOXYGEN__)

#include "rf_lld.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name   Max Payload length 
 * @{
 */ 
#define RF_MAX_PAYLOAD_LEN         32
/** @} */

//-------------RF command---------------------
#define MAC6200_R_REGISTER                 0x00 // Define read command to register
#define MAC6200_W_REGISTER                 0x20 // Define write command to register
#define MAC6200_R_RX_PAYLOAD               0x61 // Define RX payload register address
#define MAC6200_W_TX_PAYLOAD               0xA0 // Define TX payload register address
#define MAC6200_FLUSH_TX                   0xE1 // Define flush TX register command
#define MAC6200_FLUSH_RX                   0xE2 // Define flush RX register command
#define MAC6200_REUSE_TX_PL                0xE3 // Define reuse TX payload register command
#define MAC6200_ACTIVATE                   0x50 // Define ACTIVATE features register command
#define MAC6200_R_RX_PL_WID                0x60 // Define read RX payload width register command
#define MAC6200_W_ACK_PAYLOAD              0xA8 // Define write ACK payload register command
#define MAC6200_W_TX_PAYLOAD_NOACK         0xB0 // Define disable TX ACK for one time register command
#define MAC6200_NOP                        0xFF // Define No Operation, might be used to read status register

#define MAC6200_ACTIVATE_DATA              0x53

//MAC6200 Bank0 Register Addr
/**************************************************/               
#define MAC6200_BANK0_CONFIG                     0x00 // 'Config' register address
#define MAC6200_BANK0_EN_AA                      0x01 // 'Enable Auto Acknowledgment' register address
#define MAC6200_BANK0_EN_RXADDR                  0x02 // 'Enabled RX addresses' register address
#define MAC6200_BANK0_SETUP_AW                   0x03 // 'Setup address width' register address
#define MAC6200_BANK0_SETUP_RETR                 0x04 // 'Setup Auto. Retrans' register address
#define MAC6200_BANK0_RF_CH                      0x05 // 'RF channel' register address
#define MAC6200_BANK0_RF_SETUP                   0x06 // 'RF setup' register address
#define MAC6200_BANK0_STATUS                     0x07 // 'Status' register address
#define MAC6200_BANK0_OBSERVE_TX                 0x08 // 'Observe TX' register address
#define MAC6200_BANK0_RPD                        0x09 // 'Received Power Detector' register address
#define MAC6200_BANK0_RX_ADDR_P0                 0x0A // 'RX address pipe0' register address
#define MAC6200_BANK0_RX_ADDR_P1                 0x0B // 'RX address pipe1' register address
#define MAC6200_BANK0_RX_ADDR_P2                 0x0C // 'RX address pipe2' register address
#define MAC6200_BANK0_RX_ADDR_P3                 0x0D // 'RX address pipe3' register address
#define MAC6200_BANK0_RX_ADDR_P4                 0x0E // 'RX address pipe4' register address
#define MAC6200_BANK0_RX_ADDR_P5                 0x0F // 'RX address pipe5' register address
#define MAC6200_BANK0_TX_ADDR                    0x10 // 'TX address' register address
#define MAC6200_BANK0_RX_PW_P0                   0x11 // 'RX payload width, pipe0' register address
#define MAC6200_BANK0_RX_PW_P1                   0x12 // 'RX payload width, pipe1' register address
#define MAC6200_BANK0_RX_PW_P2                   0x13 // 'RX payload width, pipe2' register address
#define MAC6200_BANK0_RX_PW_P3                   0x14 // 'RX payload width, pipe3' register address
#define MAC6200_BANK0_RX_PW_P4                   0x15 // 'RX payload width, pipe4' register address
#define MAC6200_BANK0_RX_PW_P5                   0x16 // 'RX payload width, pipe5' register address
#define MAC6200_BANK0_FIFO_STATUS                0x17 // 'FIFO Status Register' register address
#define MAC6200_BANK0_DYNPD                      0x1C // 'Enable dynamic payload length' register address
#define MAC6200_BANK0_FEATURE                    0x1D // 'Feature' register address
#define MAC6200_BANK0_SETUP_VALUE                0x1E
#define MAC6200_BANK0_PRE_GURD                   0x1F


//data rate
#define MAC6200_BANK0_DR         (0x20+0x08) 
#define MAC6200_BANK0_DR_250K     0x20  
#define MAC6200_BANK0_DR_500K	 (0x20+0x08)
#define MAC6200_BANK0_DR_1M       0x00
#define MAC6200_BANK0_DR_2M	  0x08

//MAC6200_BANK0_SETUP_AW
//#define MAC6200_AW_BITS     (0x01+0x02)
//#define MAC6200_AW_4_BYTES   0x02
//#define MAC6200_AW_5_BYTES  (0x01+0x02)

/*------------------------------5.PA power-----------------------------------*/
#define MAC6200_BANK0_PA_POWER              (0x04+0x02+0x01) 
#define MAC6200_BANK0_PA_POWER_n18dBm        0x00
#define MAC6200_BANK0_PA_POWER_n12dBm        0x02
#define MAC6200_BANK0_PA_POWER_n6dBm         0x04
#define MAC6200_BANK0_PA_POWER_0dBm         (0x04+0x02)
#define MAC6200_BANK0_PA_POWER_5dBm          0x01  

//Cont wave
#define MAC6200_BANK0_CONT_WAVE              0x80  
#define MAC6200_BANK0_PRX                    0x01


//MAC6200 Bank1 register
#define MAC6200_BANK1_LINE                 0x00
#define MAC6200_BANK1_PLL_CTL0             0x01
#define MAC6200_BANK1_PLL_CTL1             0x02
#define MAC6200_BANK1_CAL_CTL              0x03
#define MAC6200_BANK1_A_CNT_REG            0x04
#define MAC6200_BANK1_B_CNT_REG            0x05
#define MAC6200_BANK1_RESERVED1            0x06
#define MAC6200_BANK1_STATUS               0x07
#define MAC6200_BANK1_STATE                0x08
#define MAC6200_BANK1_CHAN                 0x09
#define MAC6200_BANK1_IF_FREQ              0x0A
#define MAC6200_BANK1_AFC_COR              0x0B
#define MAC6200_BANK1_FDEV                 0x0C
#define MAC6200_BANK1_DAC_RANGE            0x0D
#define MAC6200_BANK1_DAC_IN               0x0E
#define MAC6200_BANK1_CTUNING              0x0F
#define MAC6200_BANK1_FTUNING              0x10
#define MAC6200_BANK1_RX_CTRL              0x11
#define MAC6200_BANK1_FAGC_CTRL            0x12
#define MAC6200_BANK1_FAGC_CTRL_1          0x13
#define MAC6200_BANK1_DAC_CAL_LOW          0x17
#define MAC6200_BANK1_DAC_CAL_HI           0x18
#define MAC6200_BANK1_RESERVED3            0x19
#define MAC6200_BANK1_DOC_DACI             0x1A
#define MAC6200_BANK1_DOC_DACQ             0x1B
#define MAC6200_BANK1_AGC_CTRL             0x1C
#define MAC6200_BANK1_AGC_GAIN             0x1D
#define MAC6200_BANK1_RF_IVGEN             0x1E
#define MAC6200_BANK1_TEST_PKDET           0x1F

#define MAC6200_BANK_BIT    0x80      /*bank 指示位*/
#define MAC6200_BANK1       0x80
#define MAC6200_BANK0       0x00 

/**************************************************
                 MAC6200 寄存器位
**************************************************/
// CONFIG
#define MAC6200_MASK_RX_DR                 0x40
#define MAC6200_MASK_TX_DS                 0x20
#define MAC6200_MASK_MAX_RT                0x10
#define MAC6200_EN_CRC                     0x08
#define MAC6200_CRCO                       0x04
#define MAC6200_CRCO_1_BYTES               0x00
#define MAC6200_CRCO_2_BYTES               0x04
#define MAC6200_PWR_UP_MASK                0x02
#define MAC6200_PWR_UP_CODE                0x02
#define MAC6200_PWR_DWN_CODE               0x00
#define MAC6200_PWR_UP                     0x02
#define MAC6200_PWR_DWN                    0x00
#define MAC6200_PRIM_RX                    0x01
#define MAC6200_PRX                        0x01
#define MAC6200_PTX                        0x00
// EN_AA                                    
#define MAC6200_EN_AA_P5                   0x20
#define MAC6200_EN_AA_P4                   0x10
#define MAC6200_EN_AA_P3                   0x08
#define MAC6200_EN_AA_P2                   0x04
#define MAC6200_EN_AA_P1                   0x02
#define MAC6200_EN_AA_P0                   0x01
// EN_RXADDR                                
#define MAC6200_ERX_P5                     0x20
#define MAC6200_ERX_P4                     0x10
#define MAC6200_ERX_P3                     0x08
#define MAC6200_ERX_P2                     0x04
#define MAC6200_ERX_P1                     0x02
#define MAC6200_ERX_P0                     0x01
// SETUP_AW                                 
#define MAC6200_AW_BITS                    0x03
#define MAC6200_AW_3_BYTES                 0x01
#define MAC6200_AW_4_BYTES                 0x02
#define MAC6200_AW_5_BYTES                 0x03
// SETUP_RETR: ARD | ARC                    
#define MAC6200_ARD_BITS                   0xF0
#define MAC6200_ARD_250_US                 0x00
#define MAC6200_ARD_500_US                 0x01
#define MAC6200_ARD_750_US                 0x02
#define MAC6200_ARD_1000_US                0x03
#define MAC6200_ARD_1250_US                0x04
#define MAC6200_ARD_1500_US                0x05
#define MAC6200_ARD_1750_US                0x06
#define MAC6200_ARD_2000_US                0x07
#define MAC6200_ARD_2250_US                0x08
#define MAC6200_ARD_2500_US                0x09
#define MAC6200_ARD_2750_US                0x0A
#define MAC6200_ARD_3000_US                0x0B
#define MAC6200_ARD_3250_US                0x0C
#define MAC6200_ARD_3500_US                0x0D
#define MAC6200_ARD_3750_US                0x0E
#define MAC6200_ARD_4000_US                0x0F
#define MAC6200_ARC_BITS                   0x0F
#define MAC6200_AUTORETR_DISABLED          0x00
#define MAC6200_ARC_00                     0x00
#define MAC6200_ARC_01                     0x01
#define MAC6200_ARC_02                     0x02
#define MAC6200_ARC_03                     0x03
#define MAC6200_ARC_04                     0x04
#define MAC6200_ARC_05                     0x05
#define MAC6200_ARC_06                     0x06
#define MAC6200_ARC_07                     0x07
#define MAC6200_ARC_08                     0x08
#define MAC6200_ARC_09                     0x09
#define MAC6200_ARC_10                     0x0A
#define MAC6200_ARC_11                     0x0B
#define MAC6200_ARC_12                     0x0C
#define MAC6200_ARC_13                     0x0D
#define MAC6200_ARC_14                     0x0E
#define MAC6200_ARC_15                     0x0F
// RF_CH: 0~125, F0=2400+RF_CH[MHz]=2.400GHz~2.525GHz.
#define MAC6200_RF_CH_BITS                 0x7F
                                            
// RF_SETUP                                 
#define MAC6200_CONT_WAVE                  0x80
#define MAC6200_RF_DR_LOW                  0x20
#define MAC6200_RF_DR_HIGH                 0x08
#define MAC6200_RF_DR_500_Kbps             0x20
#define MAC6200_RF_DR_1_Mbps               0x00
#define MAC6200_RF_DR_2_Mbps               0x08
#define MAC6200_PLL_LOCK                   0x10
#define MAC6200_RF_PWR_n18_dBm             0x00
#define MAC6200_RF_PWR_n12_dBm             0x02
#define MAC6200_RF_PWR_n6_dBm              0x04
#define MAC6200_RF_PWR_0_dBm               0x06
#define MAC6200_RF_PWR_5_dBm               0x07
#define MAC6200_LNA_HCURR                  0x01
                                            
// STATUS                                   
#define MAC6200_RX_DR                      0x40
#define MAC6200_TX_DS                      0x20
#define MAC6200_MAX_RT                     0x10
#define MAC6200_RX_P_NO                    0x0E
#define MAC6200_RX_FIFO_EMPTY              0x0E
#define MAC6200_TX_FIFO_FULL               0x01 // 与 FIFO_STATUS 中 TX_FULL 标志位重复
                                                 
// OBSERVE_TX                                    
#define MAC6200_PLOS_CNT                   0xF0 
#define MAC6200_ARC_CNT                    0x0F 
// PRD                                           
#define MAC6200_RPD_BITS                   0x01 
// RX_PW_PX                                      
#define MAC6200_RX_PW_BITS                 0x3F 
                                                 
// FIFO_STATUS                                   
#define MAC6200_FIFO_STATUS_TX_REUSE       0x40 
#define MAC6200_FIFO_STATUS_TX_FULL        0x20 // 与 STATUS 中 TX_FULL 标志位重复
#define MAC6200_FIFO_STATUS_TX_EMPTY       0x10 
#define MAC6200_FIFO_STATUS_RX_FULL        0x02 
#define MAC6200_FIFO_STATUS_RX_EMPTY       0x01 
                                                 
// DYNPD                                         
#define MAC6200_DPL_P5                     0x20 // 启用动长通道
#define MAC6200_DPL_P4                     0x10 // 启用动长通道
#define MAC6200_DPL_P3                     0x08 // 启用动长通道
#define MAC6200_DPL_P2                     0x04 // 启用动长通道
#define MAC6200_DPL_P1                     0x02 // 启用动长通道
#define MAC6200_DPL_P0                     0x01 // 启用动长通道
                                                 
// FEATURE                                       
#define MAC6200_EN_DPL                     0x04 // 启用动态载长
#define MAC6200_EN_DPL_MASK                0x04 
#define MAC6200_EN_DPL_CODE                0x04 
                                                 
#define MAC6200_EN_ACK_PAY                 0x02 // 启用有载应答
#define MAC6200_EN_DYN_ACK                 0x01 // 启用动态应答

/**
 * @brief   Select MAC6200 as rf PHY.
 *
 * @notapi
 */
#define MAC6200_PHY_Enable rf_lld_Slect_MAC6200

/**
 * @brief   change MAC6200 RF's SPI CSN to high.
 *
 * @param[in]  NONE
 *
 * @iclass
 */
#define MAC6200_CSN_High rf_lld_CSN_High

/**
 * @brief   change MAC6200 RF's CE to high.
 *
 * @param[in]  NONE
 *
 * @iclass
 */
#define MAC6200_CE_High rf_lld_CE_High

/**
 * @brief   change MAC6200 RF's SPI CSN to low.
 *
 * @param[in]  NONE
 *
 * @iclass
 */
#define MAC6200_CSN_Low rf_lld_CSN_Low

/**
 * @brief   change MAC6200 RF's CE to low.
 *
 * @param[in]  NONE
 *
 * @iclass
 */
#define MAC6200_CE_Low  rf_lld_CE_Low

/**
 * @brief  Configure MAX2829 to RX mode
 *
 * @param[in]  NONE
 *
 * @iclass
 */
#define Config_MAX2829_RX rf_lld_MAX2829_RX

/**
 * @brief  Configure MAX2829 to TX mode
 *
 * @param[in]  NONE
 *
 * @iclass
 */
#define Config_MAX2829_TX  rf_lld_MAX2829_TX

/**
 * @brief   MAC6200 Write register.
 *
 * @param[in] Reg   register which will write to
 * @param[in] Reg_Val   pointer to the value which will be write
 * @param[in] len    the length of value
 *
 * @iclass
 */
#define MAC6200_Write_Reg(Reg, Reg_Val, len) \
   rf_lld_Wr_Reg(Reg, Reg_Val, len)

/**
 * @brief   MAC6200 read register.
 *
 * @param[in] Reg   register which will be read
 * @param[in] Reg_Val   pointer to read buffer
 * @param[in] len    the length of value which will be read
 *
 * @iclass
 */
#define MAC6200_Read_Reg(Reg, Reg_Val, len) \
   rf_lld_Rd_Reg(Reg, Reg_Val, len)

/**
 * @brief   MAC6200 Write register using interrupt..
 *
 * @param[in] Reg   register which will write to
 * @param[in] Reg_Val   pointer to the value which will be write
 * @param[in] len    the length of value
 *
 * @iclass
 */
#define MAC6200_Write_Reg_Int(Reg, Reg_Val, len) \
   rf_lld_Wr_Reg_int(Reg, Reg_Val, len)

/**
 * @brief   MAC6200 read register using interrupt.
 *
 * @param[in] Reg   register which will be read
 * @param[in] Reg_Val   pointer to read buffer
 * @param[in] len    the length of value which will be read
 *
 * @iclass
 */
#define MAC6200_Read_Reg_Int(Reg, Reg_Val, len) \
   rf_lld_Rd_Reg_int(Reg, Reg_Val, len)


/**
 * @brief   MAC6200 Flush Tx FiFo.
 *
 * @param[in]  NONE
 *
 * @iclass
 */
#define MAC6200_Flush_Tx_Fifo() rf_lld_Flush_Tx_Fifo()


/**
 * @brief   MAC6200 Flush Rx FiFo.
 *
 * @param[in]  NONE
 *
 * @iclass
 */
#define MAC6200_Flush_Rx_Fifo() rf_lld_Flush_Rx_Fifo()


/**
 * @brief   switch MAC6200's bank to Bank0
 *
 * @param[in]  NONE
 *
 * @iclass
 */
#define MAC6200_Bank0_Activate() rf_lld_Bank0_Activate()

/**
 * @brief   switch MAC6200's bank to Bank1
 *
 * @param[in]  NONE
 *
 * @iclass
 */
#define MAC6200_Bank1_Activate() rf_lld_Bank1_Activate()

/**
 * @brief   Read receive payload's length at the dynamic payload length mode
 *
 * @param[in]  NONE
 *
 * @iclass
 */
#define MAC6200_Read_Rx_Payload_Width() rf_lld_Read_Rx_Payload_Width()

/**
 * @brief   Read receive payload's length at the static payload length mode
 *
 * @param[in]  Pipe_num   pipe index. vaild value is 0-5
 *
 * @iclass
 */
#define MAC6200_Read_Rx_Pipe_Static_Payload_Width(Pipe_Num)    \
      rf_lld_Read_Rx_Pipe_Static_Payload_Width(Pipe_Num)

/**
 * @brief   Read received payload.
 *
 * @param[in] pBuf:  received payload, and maximum payload length is 32 Bytes.
 * @param[in] bytes: paylaod length which ready to read, valid length is 1-32 bytes.
 *
 * @iclass
 */
#define MAC6200_Read_Rx_Payload(pBuf, bytes)  rf_lld_Read_Rx_Payload(pBuf, bytes)

/**
 * @brief  Write Tx paylaod use the comand of W_TX_PAYLOAD, 
           it may be need ackpayload after send the payload. 
 *
 * @param[in] pBuf:  ready to send paylaod value
 * @param[in] bytes: ready to seng payload length, and valid value is 1-32 bytes
 *
 * @iclass
 */
#define MAC6200_Write_Tx_Payload(pBuf, bytes)  rf_lld_Write_Tx_Payload(pBuf, bytes)

/**
 * @brief   Write Tx paylaod use the comand of W_TX_PAYLOAD_NOACK, 
            it does not need ackpayload after send the payload.
 *
 * @param[in] pBuf:  ready to send paylaod value
 * @param[in] bytes: ready to seng payload length, and valid value is 1-32 bytes
 *
 * @iclass
 */
#define MAC6200_Write_Tx_Payload_No_Ack(pBuf, bytes) \
     rf_lld_Write_Tx_Payload_No_Ack(pBuf, bytes)

/**
 * @brief   Write Acknowledge payload
 * @param[in] PipeNum : pipe index to write to ACk paylaod
 * @param[in] pBuf:  ready to send paylaod value
 * @param[in] bytes: ready to seng payload length, and valid value is 1-32 bytes
 *
 * @iclass
 */
#define MAC6200_Write_Ack_Payload(PipeNum, pBuf, bytes) \
     rf_lld_Write_Ack_Payload(PipeNum, pBuf, bytes)

/**
 * @brief  set MAC6200 Pipe address 
 * @param[in] Px_Addr_Reg:   pipe  address register
 * @param[in] pPipeAddr:  point of pipe address 
 * @param[in] Addr_Width: length of address
 *
 * @iclass
 */
#define MAC6200_Write_Pipe_Addr(Px_Addr_Reg, pPipeAddr, Addr_Width) \
     rf_lld_Write_Pipe_Addr(Px_Addr_Reg, pPipeAddr, Addr_Width)

/**
 * @brief  set MAC6200 Pipe address 
* @param[in]  RFConfig:  configure of  RF 
 *
 * @iclass
 */
#define MAC6200_Init(RFConfig) rf_lld_Init(RFConfig)

void MAC6206_set_sar_ldo();

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
void MAC6200_dump_RF_register(BaseSequentialStream *chp);

#ifdef __cplusplus
}
#endif

#endif  /* HAL_USE_RF */
#endif  /* _RF_H_ */


