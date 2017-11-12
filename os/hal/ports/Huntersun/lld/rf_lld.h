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
 * @file    hs66xx/rf_lld.h
 * @brief   HS66xx RF subsystem low level driver header.
 *
 * @addtogroup RF
 * @{
 */

#ifndef _RF_LLD_H_
#define _RF_LLD_H_

#if HAL_USE_RF || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/* configure register defines */
#define RF_MAC_SELECT			(1UL << 7)
#define RF_IRQ_RXFIFO_FULL 		(1UL << 3)
#define RF_IRQ_RXDATA_READY		(1UL << 2)
#define RF_IRQ_TxFIFO_EMPTY		(1UL << 1)
#define RF_IRQ_TXFIFO_READY		(1UL << 0)

/* status register defines */
#define RF_SPI_BUSY  			(1UL << 4)
#define RF_RX_FULL  			(1UL << 3)
#define RF_RXDATA_READY 		(1UL << 2)
#define RF_TXFIFO_EMPTY			(1UL << 1)
#define RF_TXFIFO_READY			(1UL << 0)

/* RF Configure register defines */
#define RF_CSN      			(1UL << 1)
#define RF_CE       			(1UL << 0)

/* RF Clear Interrupt register defines */
#define RF_ICR      			(1UL << 0)

#define HS_RF_INTR_PRIORITY     3

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*
 * @brief   Type of an RF Power.
 */
typedef enum {
	RF_POWER_n18dBm = 0x00,
	RF_POWER_n12dBm = 0x02,
	RF_POWER_n6dBm = 0x04,
	RF_POWER_0dBm = 0x06,
	RF_POWER_5dBm = 0x07,
} RFPower_t;

/*
 * @brief   Type of an RF Data rate.
 */
typedef enum {
	RF_DR_500Kbps = 0x28,
	RF_DR_1Mbps = 0x00,
	RF_DR_2Mbps = 0x08,
} RFDataRate_t;

/*
 * @brief   Type of an RF Communication Mode.
 */
typedef enum { 
	COMMODE_SPL_SA_NAK_ZZ = 0x00, // 静长静无答	
	COMMODE_SPL_SA_ACK_AE = 0x02, // 静长静空答	
	COMMODE_SPL_DA_ACK_AE = 0x06, // 静长动空答
	COMMODE_DPL_SA_NAK_ZZ = 0x0a, // 动长静无答	
	COMMODE_DPL_SA_ACK_AP = 0x0b, // 动长静载答
	COMMODE_DPL_DA_ACK_AE = 0x0e, // 动长动空答
	COMMODE_DPL_DA_ACK_AP = 0x0f, // 动长动载答
	COMMODE_MUL_SLAVE_NO_ACK = 0x10, // -主多从无应答
	COMMODE_MUL_SLAVE_ACK = 0x11, // -主多从带应答
        COMMODE_DPL_SA_NAK_ZZ_CRC_1Bit = 0x12, //动长静无答CRC 1bit	
} RFCommuicationMode_t;

/*
 * @brief   role of the RF when Communicate.
 */
typedef enum { 
	COMROLE_PTX = 0x00,
	COMROLE_PRX = 0x01, 
} RFRole_t;

/*
 * @brief   CRC Length of the RF.
 */
typedef enum { 
	COMCRC_1BIT = 0x00,
	COMCRC_2BIT = 0x01,
        COMCRC_DISABLE = 0x02,
} RFCRC_t;
/**
 * @brief   Structure representing a RF driver.
 */
typedef struct RFAddress {
  /**
   * @brief rf address length.
   */
  uint8_t Addr_len;
  /**
   * @brief RF address
   */
  uint8_t Addr[5];
  
} RFAddress;

/**
 * @brief   Structure representing a RF driver.
 */
typedef struct RFDriver {
  /**
   * @brief RF receive address .
   */
  RFAddress *RX_Addr;
  /**
   * @brief RF receive address .
   */
  RFAddress *TX_Addr;
  /**
   * @brief RF communication power.
   */
  RFPower_t Power;
  /**
   * @brief RF communication data rate.
   */
  RFDataRate_t DataRate;
  /**
   * @brief RF Communication Mode.
   */
  RFCommuicationMode_t Mode; 
  /**
   * @brief Pointer to the RF registers block.
   */
  HS_MAC6200_Type *id_rf;  
  /**
   * @brief rf as PTX or PRX .
   */
  RFRole_t Role;  
   /**
   * @brief Length of CRC.
   */ 
  RFCRC_t CRC;
  /**
   * @brief communication chanle .
   */
  uint8_t Chanle; 
  /**
   * @brief the max length of payload .
   */
  uint8_t Length; 
}RFDriver;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern RFDriver MAC6200;
extern volatile uint8_t RF_Status;
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*-------------------------------------------------------------
 * Select MAC6200 as rf PHY. 
 */
void rf_lld_Slect_MAC6200(void);

/*-------------------------------------------------------------
 *  Change RF's SPI CSN to high.
 */
void rf_lld_CSN_High(void);

/*-------------------------------------------------------------
 *  Change RF's SPI CSN to low.
 */
void rf_lld_CSN_Low(void);

/*-------------------------------------------------------------
 *  Change RF's CE to high.
 */
void rf_lld_CE_High(void);

/*-------------------------------------------------------------
 *  Change RF's CE to low.
 */
void rf_lld_CE_Low(void);

/*-------------------------------------------------------------
 * Write RF's register by SPI interface.
 * Reg:       RF register.
 * Reg_val:   ready to write RF register value to rf register.
 * Reg_Width: reg_value width.   
 */
void rf_lld_Wr_Reg(uint8_t Reg,uint8_t *Reg_Val,uint8_t len);

/*-------------------------------------------------------------
 * Read RF's register by SPI interface.
 * Reg:       RF register.
 * Reg_val:   ready to write RF register value to rf register.
 * Reg_Width: reg_value width.   
 */
void rf_lld_Rd_Reg(uint8_t Reg,uint8_t *Reg_Val,uint8_t len);

/*-------------------------------------------------------------
 * Write RF's register by SPI interface.
 * Reg:       RF register.
 * Reg_val:   ready to write RF register value to rf register.
 * Reg_Width: reg_value width.   
 */
void rf_lld_Wr_Reg_int(uint8_t Reg,uint8_t *Reg_Val,uint8_t len);

/*-------------------------------------------------------------
 * Read RF's register by SPI interface.
 * Reg:       RF register.
 * Reg_val:   ready to write RF register value to rf register.
 * Reg_Width: reg_value width.   
 */
void rf_lld_Rd_Reg_int(uint8_t Reg,uint8_t *Reg_Val,uint8_t len);

/*--------------------------------------------------------------
 * No Operation. Might be used to read RF's STATUS register.
 */
uint8_t rf_lld_Nop(void);

/*--------------------------------------------------------------
 * Flush RF's Tx Fifo
 */
void rf_lld_Flush_Tx_Fifo(void);

/*--------------------------------------------------------------
 * Flush RF's Rx Fifo
 */
void rf_lld_Flush_Rx_Fifo(void);

/*--------------------------------------------------------------
 * switch Rf's bank to Bank0
 */
void rf_lld_Bank0_Activate(void);

/*--------------------------------------------------------------
 * switch Rf's bank to Bank1
 */
void rf_lld_Bank1_Activate(void);

/*--------------------------------------------------------------
 * Read receive payload's length at the dynamic payload length mode
 */
uint8_t rf_lld_Read_Rx_Payload_Width(void);

/*--------------------------------------------------------------
 * Read receive payload's length at the static payload length mode
 * Pipe_num: pipe index. vaild value is 0-5 
 */
uint8_t rf_lld_Read_Rx_Pipe_Static_Payload_Width(uint8_t Pipe_Num);

/*---------------------------------------------------------------
 * Read received payload
 * pBuf:  received payload, and maximum payload length is 32 Bytes.
 * bytes: paylaod length which ready to read, valid length is 1-32 bytes.
 */
void rf_lld_Read_Rx_Payload(uint8_t *pBuf, uint8_t bytes);

/*---------------------------------------------------------------
 * Write Acknowledge payload
 * PipeNum : pipe index to write to ACk paylaod
 * pBuf:     ready to write payload, and valid length is 1-32bytes
 * byres:    acknowldge paylaod length, valid length is 1-32 bytes.   
 */ 
void rf_lld_Write_Ack_Payload(uint8_t PipeNum, uint8_t *pBuf, uint8_t bytes);

/*-----------------------------------------------------------------
 * Write Tx paylaod use the comand of W_TX_PAYLOAD_NOACK, it does not need ackpayload after send the payload. 
 * pBuf: ready to send paylaod value
 * bytes: ready to seng payload length, and valid value is 1-32 bytes.
 */
void rf_lld_Write_Tx_Payload_No_Ack(uint8_t *pBuf, uint8_t bytes);

/*-----------------------------------------------------------------
 * Write Tx paylaod use the comand of W_TX_PAYLOAD, it may be need ackpayload after send the payload. 
 * pBuf: ready to send paylaod value
 * bytes: ready to seng payload length, and valid value is 1-32 bytes.
 */
void rf_lld_Write_Tx_Payload(uint8_t *pBuf, uint8_t bytes);

/*-----------------------------------------------------------------
 * set MAC6200 device address 
 * Px_Addr_Reg:   pipe  address register
 * pPipeAddr:  point of pipe address 
 * Addr_Width: length of address
 */
void rf_lld_Write_Pipe_Addr(uint8_t Px_Addr_Reg, uint8_t *pPipeAddr, uint8_t Addr_Width);

/*-----------------------------------------------------------------
 * write ack payload
 * PipeNum:   pipe  which will be write
 * pBuf:  point to the ack buffer 
 * bytes: length of ack payload
 */
void rf_lld_Write_Ack_Payload(uint8_t PipeNum, uint8_t *pBuf, uint8_t bytes);

/*--------------------------------------------------------------
 * configure rf interrupt
 */
void rf_lld_Configure_RF_Interrupt(void);
/*-------------------------------------------------------------
 * MAC6200 init
 * RFConfig:  point to struct of rf driver 
 */
void rf_lld_Init(RFDriver *RFConfig);

/*-------------------------------------------------------------
 * MAX2829 init
 */
void MAX2829_init(void);

/*--------------------------------------------------------------
 * Configure MAX2829 as RX mode
 * channel:  recevice channel
 */
void rf_lld_MAX2829_RX(uint8_t channel);

/*--------------------------------------------------------------
 * Configure MAX2829 as TX mode
 * channel:  send channel
 */
void rf_lld_MAX2829_TX(uint8_t channel);

/*--------------------------------------------------------------
 * Configure MAX2829 as RX gain
 * gain:step-0.5dB
 */
void rf_lld_MAX2829_RX_gain(uint8_t gain);

/*--------------------------------------------------------------
 * Configure MAX2829 as TX gain
 * gain:step-0.5dB
 */
void rf_lld_MAX2829_TX_gain(uint8_t gain);

void MAX2829_RxMode_init();

void MAC6200_lld_reg_event(void (*event)(void));

#ifdef __cplusplus
}
#endif

#endif  /* HAL_USE_RF */
#endif /* _RF_LLD_H_ */

/** @} */
