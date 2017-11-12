/*
 * hs66xx.h: CMSIS Cortex-M3 Device Peripheral Access Layer Header File.
 *
 * Copyright (C) 2014, HunterSun
 * wei.lu@huntersun.com.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file contains all the peripheral register's definitions, bits
 * definitions and memory mapping.
 * The file is the unique include file that the application programmer
 * is using in the C source code, usually in main.c.
 */

/**
 * @file    hs6601.h
 * @brief   CMSIS Andes-N1086 Device Peripheral Access Layer Header File.
 *
 * @addtogroup CMSIS
 * @{
 */

#ifndef __HS6601_H
#define __HS6601_H

#ifdef __cplusplus
 extern "C" {
#endif

#if !defined (HS6601)
  #define HS6601
#endif

#include "hs6601_regs.h"
#include <stdint.h>

#define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#define   __O     volatile             /*!< Defines 'write only' permissions                */
#define   __IO    volatile             /*!< Defines 'read / write' permissions              */

/**
 * @addtogroup Peripheral_registers_structures
 * @{
 */

/**
 * @brief SYS
 */

typedef struct
{
  __I  uint32_t CHIP_ID;
  uint8_t x04_x17[0x18-0x04];
  __IO uint32_t SYS_TICK;
  __IO uint32_t USB_CTRL;
  uint8_t x20_x37[0x38-0x20];
  __IO uint32_t DEBUG_MON_ID;
#define DEBUG_MBUS_UART0   0x010
#define DEBUG_MBUS_UART1   0x020
#define DEBUG_MBUS_TIMER1  0x030
#define DEBUG_MBUS_TIMER2  0x040
#define DEBUG_MBUS_TIMER3  0x050
#define DEBUG_MBUS_SFLASH  0x060
#define DEBUG_MBUS_CODEC   0x070
#define DEBUG_MBUS_SD      0x080
#define DEBUG_MBUS_SPI     0x090
#define DEBUG_MBUS_SPI1    0x0A0
#define DEBUG_MBUS_I2C     0x0B0
#define DEBUG_MBUS_WDT     0x0C0
#define DEBUG_MBUS_APB     0x0D0
#define DEBUG_MBUS_AHB     0x0E0
#define DEBUG_MBUS_RTC     0x0F0
#define DEBUG_MBUS_UART0_O 0x100
#define DEBUG_MBUS_UART1_O 0x110
#define DEBUG_MBUS_BTPHY   0x120

  __IO uint32_t CHIP_REG_UPD;
  __IO uint32_t CHARGER_STATE;
} HS_SYS_Type;

/**
 * @brief PSO_CPM ?
 */

typedef struct
{
  __I  uint32_t REV;
  __IO uint32_t CPU_CFG;
  __IO uint32_t REG_UPD;
  __IO uint32_t APB_CFG;
  __IO uint32_t SFLASH_CFG;
  __IO uint32_t USB_CFG;
  __IO uint32_t SD_CFG;
  __IO uint32_t SD_DRV_CFG;
  __IO uint32_t TIM0_CFG;
  __IO uint32_t TIM1_CFG;
  __IO uint32_t TIM2_CFG;
  __IO uint32_t x2c;
  __IO uint32_t UART0_CFG;
  __IO uint32_t UART1_CFG;
  __IO uint32_t I2C0_CFG;
  __IO uint32_t I2S_CFG;
  __IO uint32_t x40;
  __IO uint32_t CODEC_CFG;
  __IO uint32_t x48;
  __IO uint32_t BTPHY_CFG;
  __IO uint32_t SPI0_CFG;
  __IO uint32_t SPI1_CFG;
  __IO uint32_t WDT_CFG;
  __IO uint32_t x5c;
  __IO uint32_t x60;
  __IO uint32_t x64;
  __IO uint32_t x68;
  __IO uint32_t x6c;
  __IO uint32_t AHB_GATE_CFG;
  __IO uint32_t CPM_GATE_CFG;
  __IO uint32_t CPM_ANA_CFG;
 } HS_PSO_CPM_Type;

/**
 * @brief PMU_CPM ?
 */

typedef struct
{
  __I  uint32_t REV;
  __IO uint32_t UPD;
  __IO uint32_t APB_CFG;
  __IO uint32_t RTC_CFG;
  __IO uint32_t CRY_CFG;
  __IO uint32_t PLL_SRC_CFG;
  __IO uint32_t CPM_CEVA_CFG;
} HS_PMU_CPM_Type;

/**
 * @brief RTL_TEST
 */
typedef struct {
	__IO uint32_t prt;
	__IO uint32_t stop;
}HS_RTL_TEST_Type;

/**
 * @brief MAC6200
 */

typedef struct
{
  __IO uint32_t SPIRCON;
  __IO uint32_t SPIRSTAT;
  __IO uint32_t SPIRDAT;
  __IO uint32_t RFCON;
  __IO uint32_t ICR;
} HS_MAC6200_Type;

/**

 * @brief PMU
 */

typedef struct
{
  __IO uint32_t BASIC;
  __IO uint32_t PSO_PM_CON;
  __IO uint32_t RETEN_PM_CON;
  __IO uint32_t RAM_PM_CON;
  __IO uint32_t CEVA0_PM_CON;
  __IO uint32_t CEVA1_PM_CON;
  __IO uint32_t POWER_PIN; //0X18
  __IO uint32_t SCRATCH;
  __IO uint32_t SCRATCH1;
  __IO uint32_t EXT; //0X24
       uint8_t x28_x39[0x40-0x28];
  __IO uint32_t PADC_CON[24]; //0x40 ~ 0x9C
  __IO uint32_t PADC_CSF;
  __IO uint32_t CHIP_PM_PIN;
       uint8_t xa8_xbf[0xc0-0xa8];
  __IO uint32_t GPIO_STATUS[2]; //0xc0 0xc4
  __IO uint32_t GPIO_PL_UP_30K; //pull up 30ohm for gpio[23:16], i.e. SD
       uint32_t RESET_EN;       //0xcc
  __IO uint32_t OPT_RESET_CON;
       uint32_t xd4_xdb[2];
  __IO uint32_t PMU_CLOCK_MUX;  //0xDC
       uint8_t xE0_xfb[0xfc-0xe0];
  __IO uint32_t ANA_CON; //system LDO & PLL
} HS_PMU_Type;

/**
 * @brief General Purpose I/O
 */

typedef struct
{
  __IO uint32_t DATA;
  __IO uint32_t DATAOUT;
       uint32_t Reserved0[2];
  __IO uint32_t OUTENSET;
  __IO uint32_t OUTENCLR;
  __IO uint32_t ALTFUNCSET;
  __IO uint32_t ALTFUNCCLR;
  __IO uint32_t INTENSET;
  __IO uint32_t INTENCLR;
  __IO uint32_t INTTYPESET;
  __IO uint32_t INTTYPECLR;
  __IO uint32_t INTPOLSET;
  __IO uint32_t INTPOLCLR;
  __IO uint32_t INTSTATUS;
       uint8_t  Reserved03c_39c[0x0400-0x003c];
  __IO uint32_t MASKLOWBYTE[0x100];
  __IO uint32_t MASKHIGHBYTE[0x100];
} HS_GPIO_Type;

/**
 * @brief Timer
 */

typedef struct
{
  __IO uint16_t CR1;         /*!< TIM control register 1,              Address offset: 0x00 */
  uint16_t      RESERVED0;   /*!< Reserved, 0x02                                            */
  __IO uint16_t CR2;         /*!< TIM control register 2,              Address offset: 0x04 */
  uint16_t      RESERVED1;   /*!< Reserved, 0x06                                            */
  __IO uint16_t SMCR;        /*!< TIM slave mode control register,     Address offset: 0x08 */
  uint16_t      RESERVED2;   /*!< Reserved, 0x0A                                            */
  __IO uint16_t DIER;        /*!< TIM DMA/interrupt enable register,   Address offset: 0x0C */
  uint16_t      RESERVED3;   /*!< Reserved, 0x0E                                            */
  __IO uint16_t SR;          /*!< TIM status register,                 Address offset: 0x10 */
  uint16_t      RESERVED4;   /*!< Reserved, 0x12                                            */
  __IO uint16_t EGR;         /*!< TIM event generation register,       Address offset: 0x14 */
  uint16_t      RESERVED5;   /*!< Reserved, 0x16                                            */
  __IO uint16_t CCMR1;       /*!< TIM capture/compare mode register 1, Address offset: 0x18 */
  uint16_t      RESERVED6;   /*!< Reserved, 0x1A                                            */
  __IO uint16_t CCMR2;       /*!< TIM capture/compare mode register 2, Address offset: 0x1C */
  uint16_t      RESERVED7;   /*!< Reserved, 0x1E                                            */
  __IO uint16_t CCER;        /*!< TIM capture/compare enable register, Address offset: 0x20 */
  uint16_t      RESERVED8;   /*!< Reserved, 0x22                                            */
  __IO uint32_t CNT;         /*!< TIM counter register,                Address offset: 0x24 */
  __IO uint16_t PSC;         /*!< TIM prescaler,                       Address offset: 0x28 */
  uint16_t      RESERVED9;   /*!< Reserved, 0x2A                                            */
  __IO uint32_t ARR;         /*!< TIM auto-reload register,            Address offset: 0x2C */
  __IO uint16_t RCR;         /*!< TIM repetition counter register,     Address offset: 0x30 */
  uint16_t      RESERVED10;  /*!< Reserved, 0x32                                            */
  __IO uint32_t CCR[4];      /*!< TIM capture/compare register 1,2,3,4 Address offset: 0x34,0x38,0x3C,0x40 */
  __IO uint16_t BDTR;        /*!< TIM break and dead-time register,    Address offset: 0x44 */
  uint16_t      RESERVED11;  /*!< Reserved, 0x46                                            */
  __IO uint16_t DCR;         /*!< TIM DMA control register,            Address offset: 0x48 */
  uint16_t      RESERVED12;  /*!< Reserved, 0x4A                                            */
  __IO uint16_t DMAR;        /*!< TIM DMA address for full transfer,   Address offset: 0x4C */
  uint16_t      RESERVED13;  /*!< Reserved, 0x4E                                            */
  __IO uint16_t OR;          /*!< TIM option register,                 Address offset: 0x50 */
  uint16_t      RESERVED14;  /*!< Reserved, 0x52                                            */
} HS_TIM_Type;

/**
 * @brief WATCHDOG
 */

typedef struct
{
  __IO uint32_t CR;
  __IO uint32_t TORR;
  __I  uint32_t CCVR;
  __O  uint32_t CRR;
  __I  uint32_t STAT;
  __I  uint32_t EOI;
       uint32_t   Reserved0;
  __IO uint32_t CLOCK_EN;
} HS_WDT_Type;

/**
 * @brief Real-Time Clock
 */

typedef struct
{
  __IO uint32_t TR;
  __IO uint32_t DR;
  __IO uint32_t CR;
  __IO uint32_t ISR;
  __IO uint32_t PRER;
  __IO uint32_t WUTR;
  __IO uint32_t CALIBR;
  __IO uint32_t ALRMAR;
  __IO uint32_t ALRMBR;
  __IO uint32_t WPR;
  __IO uint32_t SSR;
  __IO uint32_t SHIFTR;
  __IO uint32_t TSTR;
  __IO uint32_t TSDR;
  __IO uint32_t TSSSR;
  __IO uint32_t CALR;
  __IO uint32_t TAFCR;
  __IO uint32_t ALRMASSR;
  __IO uint32_t ALRMBSSR;
       uint32_t RESERVED;
  __IO uint32_t BKR[20];
} HS_RTC_Type;

/**
 * @brief Inter Integrated Circuit Interface
 */

typedef struct
{
  __IO uint32_t CON;
  __IO uint32_t TAR;
  __IO uint32_t SAR;
  __IO uint32_t HS_MADDR;
  __IO uint32_t DATA_CMD;
  __IO uint32_t SS_SCL_HCNT;
  __IO uint32_t SS_SCL_LCNT;
  __IO uint32_t FS_SCL_HCNT;
  __IO uint32_t FS_SCL_LCNT;
  __IO uint32_t HS_SCL_HCNT;
  __IO uint32_t HS_SCL_LCNT;
  __I  uint32_t INTR_STAT;
  __IO uint32_t INTR_MASK;
  __I  uint32_t RAW_INTR_STAT;
  __IO uint32_t RX_TL;
  __IO uint32_t TX_TL;
  __I  uint32_t CLR_INTR;
  __I  uint32_t CLR_RX_UNDER;
  __I  uint32_t CLR_RX_OVER;
  __I  uint32_t CLR_TX_OVER;
  __I  uint32_t CLR_RD_REQ;
  __I  uint32_t CLR_TX_ABRT;
  __I  uint32_t CLR_RX_DONE;
  __I  uint32_t CLR_ACTIVITY;
  __I  uint32_t CLR_STOP_DET;
  __I  uint32_t CLR_START_DET;
  __I  uint32_t CLR_GEN_CALL;
  __IO uint32_t ENABLE;
  __I  uint32_t STATUS;
  __I  uint32_t TXFLR;
  __I  uint32_t RXFLR;
  __IO uint32_t SDA_HOLD;
  __IO uint32_t TX_ABRT_SOURCE;
  __IO uint32_t SLV_DATA_NACK_ONLY;
  __IO uint32_t DMA_CR;
  __IO uint32_t DMA_TDLR;
  __IO uint32_t DMA_RDLR;
  __IO uint32_t SDA_SETUP;
  __IO uint32_t ACK_GENERAL_CALL;
  __I  uint32_t ENABLE_STATUS;
  __IO uint32_t CON1;  // modify to support I2C use Only ONE dma 0xa0
       uint8_t Reserved0a0_0f3[0x0f4-0x0a4];
  __I  uint32_t COMP_PARAM_1;
  __I  uint32_t COMP_VERSION;
  __I  uint32_t COMP_TYPE;
} HS_I2C_Type;

typedef struct
{
  union
  {
    __I uint32_t LRBR;
    __O uint32_t LTHR;
  };

  union
  {
    __I uint32_t RRBR;
    __O uint32_t RTHR;
  };

  __IO uint32_t RER;
  __IO uint32_t TER;
  __IO uint32_t RCR;
  __IO uint32_t TCR;
  __I  uint32_t ISR;
  __IO uint32_t IMR;
  __I  uint32_t ROR;
  __I  uint32_t TOR;
  __IO uint32_t RFCR;
  __IO uint32_t TFCR;
  __O  uint32_t RFF;
  __O  uint32_t TFF;

  __IO uint32_t RESERVE[2];

}HS_I2SCHN_Type;

typedef struct
{
  __IO uint32_t IER;
  __IO uint32_t IRER;
  __IO uint32_t ITER;
  __IO uint32_t CER;
  __IO uint32_t CCR;
  __IO uint32_t RXFFR;
  __IO uint32_t TXFFR;

  __IO uint32_t RESERVE0;

  HS_I2SCHN_Type CHN_R0;
  HS_I2SCHN_Type CHN_R1;
  HS_I2SCHN_Type CHN_R2;
  HS_I2SCHN_Type CHN_R3;

  __IO uint32_t RESERVE1[40];

  __I  uint32_t RXDMA;
  __O  uint32_t RRXDMA;
  __O  uint32_t TXDMA;
  __O  uint32_t RTXDMA;

  __IO uint32_t RESERVE2[8];

  __I  uint32_t I2S_COMP_PARAM_2;
  __I  uint32_t I2S_COMP_PARAM_1;
  __I  uint32_t I2S_COMP_VERSION;
  __I  uint32_t I2S_COMP_TYPE;
} HS_I2S_Type;

/**
 * @brief SD/MMC host controller
 */

typedef struct
{
  __IO uint32_t CTRL;
  __IO uint32_t PWREN;
  __IO uint32_t CLKDIV;
  __IO uint32_t CLKSRC;
  __IO uint32_t CLKENA;
  __IO uint32_t TMOUT;
  __IO uint32_t CTYPE;
  __IO uint32_t BLKSIZ;
  __IO uint32_t BYTCNT;
  __IO uint32_t INTMASK;
  __IO uint32_t CMDARG;
  __IO uint32_t CMD;
  __I  uint32_t RESP0;
  __I  uint32_t RESP1;
  __I  uint32_t RESP2;
  __I  uint32_t RESP3;
  __I  uint32_t MINTSTS;
  __IO uint32_t RINTSTS;
  __I  uint32_t STATUS;
  __IO uint32_t FIFOTH;
  __I  uint32_t CDETECT;
  __I  uint32_t WRTPRT;
  __IO uint32_t GPIO;
  __I  uint32_t TCBCNT;
  __I  uint32_t TBBCNT;
  __IO uint32_t DEBNCE;
  __IO uint32_t USRID;
  __I  uint32_t VERID;	/* H'5342240a */
  __I  uint32_t HCON;	/* IP configuration */
  __IO uint32_t UHS_REG;/* UHS-1: DDR, High Voltage */
  __IO uint32_t RST_n;	/* reset card: 1-active mode, 0-reset */
       uint32_t Reserved7c;
  __IO uint32_t BMOD;	/* IDMAC */
  __O  uint32_t PLDMND;	/* poll demand */
  __IO uint32_t DBADDR;	/* descriptor list base address */
  __IO uint32_t IDSTS;	/* IDMAC status */
  __IO uint32_t IDINTEN;/* IDMAC interrupt enable */
  __I  uint32_t DSCADDR;/* current host descriptor address */
  __I  uint32_t BUFADDR;/* current buffer descriptor address */
       uint8_t Reserved09c_0ff[0x100-0x09c];
  __IO uint32_t CardThrCtl;	/* card threshold control */
  __IO uint32_t Back_end_power; /* 0-off, 1-supply */
       uint8_t Reserved108_1ff[0x200-0x108];
  __IO uint32_t DATA;	/* data FIFO */
} HS_SDHC_Type;

/**
 * @brief DMA controller
 */

typedef struct
{
  __IO uint32_t SAR;
       uint32_t __pad_SAR;
  __IO uint32_t DAR;
       uint32_t __pad_DAR;
  __IO uint32_t LLP;
       uint32_t __pad_LLP;
  __IO uint32_t CTL_LO;
  __IO uint32_t CTL_HI;
  __IO uint32_t SSTAT;
       uint32_t __pad_SSTAT;
  __IO uint32_t DSTAT;
       uint32_t __pad_DSTAT;
  __IO uint32_t SSTATAR;
       uint32_t __pad_SSTATAR;
  __IO uint32_t DSTATAR;
       uint32_t __pad_DSTATAR;
  __IO uint32_t CFG_LO;
  __IO uint32_t CFG_HI;
  __IO uint32_t SGR;
       uint32_t __pad_SGR;
  __IO uint32_t DSR;
       uint32_t __pad_DSR;
} HS_DMA_CH_Type;

typedef struct
{
  uint32_t XFER;
  uint32_t __pad_Tfr;
  uint32_t BLOCK;
  uint32_t __pad_Block;
  uint32_t SRC_TRAN;
  uint32_t __pad_SrcTran;
  uint32_t DST_TRAN;
  uint32_t __pad_DstTran;
  uint32_t ERROR;
  uint32_t __pad_Err;
} HS_DMA_IRQ_Type;

typedef struct
{
  __I  HS_DMA_IRQ_Type RAW;
  __I  HS_DMA_IRQ_Type STATUS;
  __IO HS_DMA_IRQ_Type MASK;
  __O  HS_DMA_IRQ_Type CLEAR;

  __I  uint32_t STATUS_INT;
       uint32_t __pad_StatusInt;

  __IO uint32_t REQ_SRC;
       uint32_t __pad_ReqSrc;
  __IO uint32_t REQ_DST;
       uint32_t __pad_ReqDst;
  __IO uint32_t SGL_REQ_SRC;
       uint32_t __pad_SglReqSrc;
  __IO uint32_t SGL_REQ_DST;
       uint32_t __pad_SglReqDst;
  __IO uint32_t LAST_SRC;
       uint32_t __pad_LstSrc;
  __IO uint32_t LAST_DST;
       uint32_t __pad_LstDst;

  __IO uint32_t CFG;
       uint32_t __pad_DmaCfg;
  __IO uint32_t CH_EN;
       uint32_t __pad_ChEn;
  __IO uint32_t ID;
       uint32_t __pad_DmaId;
  __IO uint32_t TEST;
       uint32_t __pad_DmaTest;

       uint64_t Reserved0;
       uint64_t Reserved1;

       uint32_t Reserved;
  __I  uint32_t DWC_PARAMS[8];
  __I  uint32_t MULTI_BLK_TYPE;
  __I  uint32_t MAX_BLK_SIZE;
  __I  uint32_t PARAMS;
  __I  uint32_t VERSION;
  __I  uint32_t TYPE;
} HS_DMAC_Type;

/**
 * @brief Serial Peripheral Interface
 */

typedef struct
{
  __IO uint32_t CTR;
  __IO uint32_t WDATA;
  __I  uint32_t RDATA;
  __IO uint32_t STAT;
  __IO uint32_t DMACR;
  __IO uint32_t DMATDLR;
  __IO uint32_t DMARDLR;
} HS_SPI_Type;

/**
 * @brief Universal Asynchronous Receiver Transmitter
 */

typedef struct
{
  union
  {
    __I  uint32_t RBR;
    __O  uint32_t THR;
    __IO uint32_t DLL;	/* LCR[7] = 1 */
  };
  union
  {
    __IO uint32_t DLH;	/* LCR[7] = 1 */
    __IO uint32_t IER;
  };
  union
  {
    __I  uint32_t IIR;
    __O  uint32_t FCR;
  };
  __IO uint32_t LCR;
  __IO uint32_t MCR;
  __I  uint32_t LSR;
  __I  uint32_t MSR;
  __IO uint32_t SCR;
  __IO uint32_t LPDLL;
  __IO uint32_t LPDLH;
       uint32_t Reserved28_2c[2];
  union
  {
    __I  uint32_t SRBR[(0x70-0x30)/4];
    __O  uint32_t STHR[(0x70-0x30)/4];
  };
  __IO uint32_t FAR;
  __I  uint32_t TFR;
  __O  uint32_t RFW;
  __I  uint32_t USR;
  __I  uint32_t TFL;
  __I  uint32_t RFL;
  __O  uint32_t SRR;
  __IO uint32_t SRTS;
  __IO uint32_t SBCR;
  __IO uint32_t SDMAM;
  __IO uint32_t SFE;
  __IO uint32_t SRT;
  __IO uint32_t STET;
  __IO uint32_t HTX;
  __O  uint32_t DMASA;
       uint8_t Reservedac_f0[0xf4-0xac];
  __I uint32_t CPR;
  __I uint32_t UCV;
  __I uint32_t CTR;
} HS_UART_Type;


/**
 * @brief spi flash interface
 */

typedef struct
{
  __I  uint32_t INTR_STATUS;
  __IO uint32_t RAW_INTR_STATUS;
  __IO uint32_t INTR_MASK;
  __IO uint32_t COMMAND;
  __IO uint32_t COMMAND_DATA0_REG;
  __IO uint32_t COMMAND_DATA1_REG;
  __IO uint32_t READ0_REG;
  __IO uint32_t READ1_REG;
  __IO uint32_t ADDRESS_REG;
  __IO uint32_t READ_OPCODE_REG;
  __IO uint32_t CONFIGURATION_0;
  __IO uint32_t CS_CONFIGURATION_0;
  __IO uint32_t CONFIGURATION_1;
  __IO uint32_t CS_CONFIGURATION_1;
  __IO uint32_t CONFIGURATION_2;
  __IO uint32_t CS_CONFIGURATION_2;
  __IO uint32_t CONFIGURATION_3;
  __IO uint32_t CS_CONFIGURATION_3;
} HS_SF_Type;

/**
 * @brief Audio CODEC Interface
 */

typedef struct
{
  __IO uint32_t ADC_CTRL;
  __IO uint32_t ADC_DECI_FILT_11;
  __IO uint32_t ADC_DECI_FILT_12;
  __IO uint32_t ADC_DECI_FILT_13;
  __IO uint32_t ADC_DECI_FILT_21;
  __IO uint32_t ADC_DECI_FILT_22;
  __IO uint32_t ADC_DECI_FILT_23;
  __IO uint32_t ADC_DECI_FILT_31;
  __IO uint32_t ADC_DECI_FILT_32;
  __IO uint32_t ADC_DECI_FILT_33;
  __IO uint32_t ADC_DECI_FILT_41;
  __IO uint32_t ADC_DECI_FILT_42;
  __IO uint32_t ADC_DECI_FILT_43;
  __IO uint32_t ADC_DECI_FILT_51;
  __IO uint32_t ADC_DECI_FILT_52;
  __IO uint32_t ADC_DECI_FILT_53;
  __IO uint32_t ADC_IIR_FILT_11;      /* 16 */
  __IO uint32_t ADC_IIR_FILT_12;
  __IO uint32_t ADC_IIR_FILT_13;
  __IO uint32_t ADC_IIR_FILT_21;
  __IO uint32_t ADC_IIR_FILT_22;
  __IO uint32_t ADC_IIR_FILT_23;
  __IO uint32_t ADC_IIR_FILT_31;
  __IO uint32_t ADC_IIR_FILT_32;
  __IO uint32_t ADC_IIR_FILT_33;
  __IO uint32_t ADC_DRC_CTRL_1;
  __IO uint32_t ADC_DRC_CTRL_2;
  __IO uint32_t ADC_DRC_CTRL_3;
  __IO uint32_t ADC_DRC_CTRL_4;
  __IO uint32_t ADC_DRC_CTRL_5;
  __IO uint32_t ADC_VOL_CTRL;
  __IO uint32_t ADC_SIDE_CTRL;
  __IO uint32_t DAC_EQ_CTRL_1;      /* 32 */
  __IO uint32_t DAC_EQ_CTRL_2;
  __IO uint32_t DAC_EQ_CTRL_3;
  __IO uint32_t DAC_EQ_CTRL_4;
  __IO uint32_t DAC_EQ_CTRL_5;
  __IO uint32_t DAC_EQ_CTRL_6;
  __IO uint32_t DAC_EQ_CTRL_7;
  __IO uint32_t DAC_EQ_CTRL_8;
  __IO uint32_t DAC_DRC_CTRL_1;
  __IO uint32_t DAC_DRC_CTRL_2;
  __IO uint32_t DAC_DRC_CTRL_3;
  __IO uint32_t DAC_DRC_CTRL_4;
  __IO uint32_t DAC_DRC_CTRL_5;
  __IO uint32_t DAC_VOL_CTRL;
  __IO uint32_t DAC_MOD_CTRL;
  __IO uint32_t DAC_CTRL;
  __IO uint32_t TEST_MODE;    /* 48 */
  __I  uint32_t GAIN_READ;
  __IO uint32_t CLK_CTRL_1;
  __IO uint32_t CLK_CTRL_2;
  __IO uint32_t ADC_DITHER_CTRL_1;
  __IO uint32_t ADC_DITHER_CTRL_2;
  __IO uint32_t DAC_DITHER_CTRL;
  __IO uint32_t INT_STATUS;
  __IO uint32_t INT_CTRL;
  __IO uint32_t xANA_CTRL_1;
  __IO uint32_t xANA_CTRL_2;
  __IO uint32_t xRSVD_1;
  __IO uint32_t ADC_PEAK_READ;
  __IO uint32_t DAC_PEAK_READ;
  __IO uint32_t AEC_DELAY_CFG;
  __IO uint32_t IF_CTRL;
} HS_CODEC_Type;

/**
 * @brief Bluetooth Baseband Link Controller
 */

typedef struct
{
  /*
   * BR
   */
  __IO uint32_t BD_ADDR;
  __IO uint32_t BD_ADDR4 : 8;
  __IO uint32_t BD_ADDR5 : 8;
  __IO uint32_t x06 : 16;

  __IO uint64_t SYNC;

  __IO uint32_t INTRASLOT_OFFSET : 16;
  __IO uint32_t x12 : 16;

  union {
    __O uint32_t CLK_UPDATE_INTRASLOT_VALUE;
    __I uint32_t CURRENT_BT_CLK_OFFSET;
  };

  union {
    __O uint32_t BT_CLK_OFFSET;
    __I uint32_t BT_CLK;
  };

  __IO uint32_t NATIVE_BT_CLK;

  __IO uint32_t AM_ADDR : 3;
  __IO uint32_t ENCRYPT : 2;
  __IO uint32_t USE_LF  : 1;
  __IO uint32_t SLAVE   : 1;
  __IO uint32_t PAGE    : 1;
  __IO uint32_t SCO_CFG0 : 4;
  __IO uint32_t SCO_CFG1 : 4;
  __IO uint32_t SCO_CFG2    : 4;
  __IO uint32_t SCO_FIFO    : 2;
  __IO uint32_t VCI_CLK_SEL : 2;
  __IO uint32_t CRC_INIT : 8;

  __IO uint32_t LOOP     : 1;
  __IO uint32_t TEST_ECO : 1;
  __IO uint32_t ECO_ERR  : 4;
  __IO uint32_t TEST_CRC : 1;
  __IO uint32_t TEST_HEC : 1;
  __IO uint32_t TEST_RADIO        : 1;
  __IO uint32_t VCI_CLK_SEL_MAP   : 1;
  __IO uint32_t TEST_FORCE_NO_DDW : 1;
  __IO uint32_t TEST_MSG          : 1;
  __IO uint32_t TEST_TX_SHIFT     : 3;
  __IO uint32_t JAL_USE_HAB_CTRL  : 1;
  __IO uint32_t DWH_INIT      : 6;
  __IO uint32_t x26_rsv       : 1;
  __IO uint32_t DWH_BY_BT_CLK : 1;
  __IO uint32_t DWH2_INIT     : 6;
  __IO uint32_t DOUBLE_BUF    : 1;
  __IO uint32_t WHITEN        : 1;

  /* Interrupt Control Register */
  __IO uint32_t TIM_INTR_MSK        : 4;
  __IO uint32_t PKD_INTR_MSK        : 1;
  __IO uint32_t AUX_TIM_INTR_MSK    : 1;
  __IO uint32_t PKA_INTR_MSK        : 1;
  __IO uint32_t PKD_RX_HDR_INTR_MSK : 1;
  __IO uint32_t x29_rsv              : 4;
  __IO uint32_t AES_INTR_MSK         : 1;
  __IO uint32_t NO_PKT_RCVD_INTR_MSK : 1;
  __IO uint32_t SYNC_DET_INTR_MSK    : 1;
  __IO uint32_t TX_RX_AUX1_PKT       : 1;
  __IO uint32_t TIM_INTR_CLR        : 4;
  __IO uint32_t PKD_INTR_CLR        : 1;
  __IO uint32_t AUX_TIM_INTR_CLR    : 1;
  __IO uint32_t PKA_INTR_CLR        : 1;
  __IO uint32_t PKD_RX_HDR_INTR_CLR : 1;
  __IO uint32_t x2b_rsv0             : 1;
  __IO uint32_t SPI_NOW_CONFLICT_CLR : 1;
  __IO uint32_t x2b_rsv2             : 1;
  __IO uint32_t SER0_WR_CLR          : 1;
  __IO uint32_t AES_INTR_CLR         : 1;
  __IO uint32_t NO_PKT_RCVD_INTR_CLR : 1;
  __IO uint32_t SYNC_DET_INTR_CLR    : 1;
  __IO uint32_t VCI_SYNC_DIR         : 1;

  /* Common Status Register */
  __IO uint32_t TIM_INTR        : 4;
  __IO uint32_t PKD_INTR        : 1;
  __IO uint32_t AUX_TIM_INTR    : 1;
  __IO uint32_t PKA_INTR        : 1;
  __IO uint32_t PKD_RX_HDR_INTR : 1;
  __IO uint32_t x2d_rsv0         : 1;
  __IO uint32_t SPI_NOW_CONFLICT : 1;
  __IO uint32_t x2d_rsv12        : 2;
  __IO uint32_t AES_INTR         : 1;
  __IO uint32_t NO_PKT_RCVD_INTR : 1;
  __IO uint32_t SYNC_DET_INTR    : 1;
  __IO uint32_t SLEEP_STATUS     : 1;
  __IO uint32_t x2e : 16;

  /* Transmit Control Register */
  __IO uint32_t TX_LEN  : 9;
  __IO uint32_t TX_TYPE : 4;
  __IO uint32_t TX_FLOW : 1;
  __IO uint32_t TX_ARQN : 1;
  __IO uint32_t TX_SEQN : 1;
  __IO uint32_t x32_rsv0  : 1;
  __IO uint32_t TX_MODE   : 2;
  __IO uint32_t TX_L_CH   : 2;
  __IO uint32_t TX_P_FLOW : 1;
  __IO uint32_t x32_rsv   : 2;
  __IO uint32_t x33 : 8;

  /* eSCO Control Register */
  __IO uint32_t ESCO_TX_LEN : 8;
  __IO uint32_t ESCO_RX_LEN : 8;
  __IO uint32_t ESCO_LT_ADDR : 3;
  __IO uint32_t SCO_ROUTE : 1;
  __IO uint32_t x36 : 12;

  __I  uint32_t NATIVE_BITCNT : 11;
  __IO uint32_t x39_rsv       : 5;
  __IO uint32_t x3a : 16;

  __IO uint32_t CODEC_TYPE : 3;
  __IO uint32_t x3c_rsv    : 5;
  __IO uint32_t x3d        : 24;

  uint8_t x40_x47[0x48-0x40];

  /* Receive Control Register */
  __IO uint32_t RX_MODE           : 2;
  __IO uint32_t SYNC_ERR          : 5;
  __IO uint32_t ABORT_ON_FEC_FAIL : 1;
  __IO uint32_t RX_BUF                 : 1;
  __IO uint32_t ERR_SEL                : 3;
  __IO uint32_t WIN_EXT                : 10;
  __IO uint32_t FREEZE_BIT_CNT         : 1;
  __IO uint32_t ABORT_ON_WRONG_AM_ADDR : 1;
  __IO uint32_t x4b : 8;

  uint8_t x4c_x4f[0x50-0x4c];

  /* Receive Status Register */
  __I  uint32_t RX_LEN  : 9;
  __I  uint32_t RX_TYPE : 4;
  __I  uint32_t RX_FLOW : 1;
  __I  uint32_t RX_ARQN : 1;
  __I  uint32_t RX_SEQN : 1;
  __I  uint32_t RX_AM_ADDR : 3;
  __I  uint32_t RX_L_CH    : 2;
  __I  uint32_t RX_P_FLOW  : 1;
  __I  uint32_t x52_rsv    : 2;
  __I  uint32_t RX_PKT        : 1;
  __I  uint32_t HEC_ERR       : 1;
  __I  uint32_t CRC_ERR       : 1;
  __I  uint32_t RX_HDR        : 1;
  __I  uint32_t FEC_ABORT     : 1;
  __I  uint32_t AM_ADDR_ABORT : 1;
  __I  uint32_t x53_rsv       : 2;

  __I  uint32_t RX0_OVER  : 1;
  __I  uint32_t RX0_UNDER : 1;
  __I  uint32_t RX1_OVER  : 1;
  __I  uint32_t RX1_UNDER : 1;
  __I  uint32_t RX2_OVER  : 1;
  __I  uint32_t RX2_UNDER : 1;
  __I  uint32_t x54_rsv   : 2;
  __I  uint32_t x55       : 24;

  __I  uint32_t PKD_POS_ERR : 8;
  __I  uint32_t x59         : 24;

  __I  uint32_t ERR_CNT : 16;
  __I  uint32_t x5e     : 16;

  /* Serial Interface */
  __IO uint32_t SER_CFG  : 31;
  __I  uint32_t SER_BUSY : 1;

  /* JAL_SER_ESER_CTRL_REG */
  __IO union {
    struct {
      uint32_t SER_TIME    : 8;
      uint32_t SER_COMBINE : 3;
      uint32_t x65_rsv     : 5;
      uint32_t SER_MASK    : 4;
      uint32_t sel0_rsv    : 12;
    } SER_MAP_SEL0;
    struct {
      uint32_t SER_MASK    : 16;
      uint32_t SER_COMBINE  : 3;
      uint32_t SER_ONE_SHOT : 1;
      uint32_t sel1_rsv     : 12;
    } SER_MAP_SEL1;
    struct {
      uint32_t x64_rsv     : 20;
      uint32_t SER_NOW     : 1;
      uint32_t SER_SEQ     : 1;
      uint32_t SER_DOUBLE  : 1;
      uint32_t SER_MAP_SEL : 1;
      uint32_t x67         : 8;
    } SER_MAP_COMM;
  } SER_MAP;

  /* JAL_SER_ESER_TIME_REG */
  __IO uint32_t SER_TIME_0 : 4;
  __IO uint32_t SER_TIME_1 : 4;
  __IO uint32_t SER_TIME_2 : 4;
  __IO uint32_t SER_TIME_3 : 4;
  __IO uint32_t SER_TIME_EARLY : 4;
  __IO uint32_t SER_TIME_LATE  : 4;
  __IO uint32_t x6b : 8;

  __I  uint32_t SER_READ_DATA;

  /* JAL_SER_DATAn_REG */
  __IO uint32_t SER_DATA[4];

  /* Serial Transceiver Interface */
  __IO uint32_t STB_CFG  : 26;
  __IO uint32_t x83_rsv  : 5;
  __IO uint32_t STB_BUSY : 1;

  __IO uint32_t STB_DATA;
  uint8_t x88_x8f[0x90-0x88];

  __IO uint32_t HIGH_SER0          : 11;
  __IO uint32_t x91_rsv            : 2;
  __IO uint32_t HIGH_SER0_EXT      : 2;
  __IO uint32_t HIGH_SER0_OVERRIDE : 1;
  __IO uint32_t HIGH_SER1          : 11;
  __IO uint32_t x93_rsv            : 2;
  __IO uint32_t HIGH_SER1_EXT      : 2;
  __IO uint32_t HIGH_SER1_OVERRIDE : 1;

  __IO uint32_t LOW_SER0          : 11;
  __IO uint32_t x95_rsv           : 2;
  __IO uint32_t LOW_SER0_EXT      : 2;
  __IO uint32_t LOW_SER0_OVERRIDE : 1;
  __IO uint32_t LOW_SER1          : 11;
  __IO uint32_t x97_rsv           : 2;
  __IO uint32_t LOW_SER1_EXT      : 2;
  __IO uint32_t LOW_SER1_OVERRIDE : 1;

  uint8_t x98_x9b[0x9c-0x98];

  /* Receive Volume control for Motorola Codec */
  __IO uint32_t sco_vol_att : 3;
  __IO uint32_t x9c_rsv     : 4;
  __IO uint32_t PF3_BYPASS  : 1;
  __IO uint32_t x9d         : 24;

  /* Auxiliary Timer */
  __IO uint32_t AUX_TIMER : 14;
  __IO uint32_t xa1       : 18;

  /* PIA */
  __IO uint32_t P_DATA_0 : 8;
  __IO uint32_t P_DATA_1 : 8;
  __IO uint32_t P_DATA_2 : 8;
  __IO uint32_t P_DATA_3 : 8;

  __IO uint32_t P_SETUP : 8;
  __IO uint32_t P_MASK     : 4;
  __IO uint32_t P_NOW      : 1;
  __IO uint32_t P_SEQ      : 1;
  __IO uint32_t P_DATA_DIR : 1;
  __IO uint32_t xa9_rsv    : 1;
  __IO uint32_t P_RDATA : 8;
  __IO uint32_t xab : 8;

  uint8_t xac_xcf[0xb0-0xac];

  /* Path Delays */
  __IO uint32_t RX_DELAY : 7;
  __IO uint32_t xb0_rsv  : 1;
  __IO uint32_t TX_DELAY : 4;
  __IO uint32_t xb1_rsv  : 4;
  __IO uint32_t xb2 : 16;
  uint8_t xb4_xb7[0xb8-0xb4];

  /* Encryption Key */
  __IO uint32_t ENC_KEY_LEN : 4;
  __IO uint32_t xb8         : 28;
  uint8_t xbc_xbf[0xc0-0xbc];
  __IO uint32_t ENC_KEY[4];

  /* HSE Registers */
  __IO uint32_t HSE_BT_CLK : 28;
  __IO uint32_t xd3_rsv    : 4;

  __IO uint32_t HSE_UAP_LAP : 28;
  __IO uint32_t xd7_rsv     : 4;

  __IO uint32_t HSE_SUM : 26;
  __IO uint32_t xdb_rsv : 6;

  __IO uint32_t HSE_RF_CHAN_INDEX : 7;
  __IO uint32_t xdc_rsv : 25;

  uint8_t xe0_xef[0xf0-0xe0];

  /* BT_CLK Controls */
  __IO uint32_t ADD_BT_CLK_RELATIVE   : 1;
  __IO uint32_t WRITE_ABSOLUTE_BT_CLK : 1;
  __IO uint32_t DELAYED_BT_CLK_UPDATE : 1;
  __IO uint32_t FREEZE_BT_CLK         : 1;
  __IO uint32_t SCO_REPEATER_BIT      : 1;
  __IO uint32_t xf0_rsv               : 27;
  uint8_t xf4_xf7[0xf8-0xf4];

  /* Revision Code */
  __I  uint32_t MINOR_REV : 8;
  __I  uint32_t MAJOR_REV : 8;
  __IO uint32_t xfa       : 16;

  /* Reset Controls */
  __IO uint32_t xfc      : 24;
  __IO uint32_t RST_CODE : 8;

  uint8_t x100_x10f[0x110-0x100];

  /* CVSD Filter Register */
  __IO uint32_t WR_RX_HPF_FILT : 16;
  __IO uint32_t x112           : 16;
  __IO uint32_t WR_RX_PF1_FILT_A;
  __IO uint32_t WR_RX_PF1_FILT_B;
  __IO uint32_t WR_RX_PF2_FILT;
  __IO uint32_t WR_RX_PF3_FILT;
  __IO uint32_t WR_RX_CVSD_FILT_A;
  __IO uint32_t WR_RX_CVSD_FILT_B;
  uint32_t x12c;

  __IO uint32_t WR_TX_HPF_FILT : 16;
  __IO uint32_t x132           : 16;
  __IO uint32_t WR_TX_PF1_FILT_A;
  __IO uint32_t WR_TX_PF1_FILT_B;
  __IO uint32_t WR_TX_PF2_FILT;
  __IO uint32_t WR_TX_PF3_FILT;
  __IO uint32_t WR_TX_CVSD_FILT_A;
  __IO uint32_t WR_TX_CVSD_FILT_B;
  uint32_t x14c;

  __IO uint32_t TX_HPF_FILT : 16;
  __IO uint32_t x152        : 16;
  __IO uint32_t TX_PF1_FILT_A;
  __IO uint32_t TX_PF1_FILT_B;
  __IO uint32_t TX_PF2_FILT;
  __IO uint32_t TX_PF3_FILT;
  __IO uint32_t TX_CVSD_FILT_A;
  __IO uint32_t TX_CVSD_FILT_B;
  uint32_t x16c;

  __IO uint32_t RX_HPF_FILT : 16;
  __IO uint32_t x172        : 16;
  __IO uint32_t RX_PF1_FILT_A;
  __IO uint32_t RX_PF1_FILT_B;
  __IO uint32_t RX_PF2_FILT;
  __IO uint32_t RX_PF3_FILT;
  __IO uint32_t RX_CVSD_FILT_A;
  __IO uint32_t RX_CVSD_FILT_B;

  uint8_t x18c_x19f[0x1a0-0x18c];

  /* VCI Channel FIFO Registers */
  __IO uint32_t SCO_CHAN_FIFO_MODE0 : 1;
  __IO uint32_t SCO_CHAN_16_8BIT0   : 1;
  __IO uint32_t SCO_CHAN_FIFO_RST0  : 1;
  __I  uint32_t SCO_RX_FIFO_LOW0    : 1;
  __I  uint32_t SCO_TX_FIFO_HIGH0   : 1;
  __IO uint32_t x1a0_rsv            : 3;
  __IO uint32_t SCO_RX_FIFO_THRESHOLD0  : 6;
  __IO uint32_t SCO_TX_FIFO_THRESHOLD0  : 6;
  __I  uint32_t SCO_RX_FIFO_FILL_LEVEL0 : 6;
  __I  uint32_t SCO_TX_FIFO_FILL_LEVEL0 : 6;

  __I  uint32_t SCO_FIFO_TX_DATA0;
  __O  uint32_t SCO_FIFO_RX_DATA0;

  uint8_t x1ac_x1ff[0x200-0x1ac];

  /* Transmit Memory */
  __IO uint32_t TX_ACL[(0x368-0x200)/4]; //BR: 340B
  uint8_t x368_x3ff[0x400-0x368];

  /* Receive Memory */
  __IO uint32_t RX_ACL[(0x568-0x400)/4];
  uint8_t x568_x5ff[0x600-0x568];

  /* Serial Interface Data Registers */
  __IO uint32_t ESER_DATA[16];

  uint8_t x640_x6ff[0x700-0x640];

  /*
   * EDR
   */
  /* EDR Control Registers */
  __IO uint32_t EDR_ENABLE     : 1;
  __IO uint32_t EDR_PTT_ACL    : 1;
  __IO uint32_t EDR_PTT_ESCO   : 1;
  __IO uint32_t EDR_SYNC_ERROR : 5;
  __IO uint32_t EDR_RX_EDR_DELAY : 4;
  __IO uint32_t EDR_TX_EDR_DELA  : 4;
  __IO uint32_t x702 : 16;

  uint8_t x704_x70a[0x710-0x704];

  /* Transmit Control Registers */
  __IO uint32_t EDR_TX_LEN  : 10;
  __IO uint32_t EDR_TX_TYPE : 4;
  __IO uint32_t EDR_TX_FLOW : 1;
  __IO uint32_t EDR_TX_ARQN : 1;
  __IO uint32_t EDR_TX_SEQN   : 1;
  __IO uint32_t x712_rsv      : 1;
  __IO uint32_t EDR_TX_MODE   : 2;
  __IO uint32_t EDR_TX_L_CH   : 2;
  __IO uint32_t EDR_TX_P_FLOW : 1;
  __IO uint32_t x712_x713     : 9;

  uint8_t x714_x71f[0x720-0x714];

  /* eSCO Control Registers */
  __IO uint32_t EDR_ESCO_TX_LEN  : 10;
  __IO uint32_t EDR_ESCO_LT_ADDR : 3;
  __IO uint32_t EDR_SCO_ROUTE    : 1;
  __IO uint32_t x721_rsv         : 2;
  __IO uint32_t EDR_ESCO_RX_LEN : 10;
  __IO uint32_t x723_rsv        : 6;

  uint8_t x724_x72f[0x730-0x724];

  /* Receive Control Register */
  __I  uint32_t EDR_RX_LEN  : 10;
  __I  uint32_t EDR_RX_TYPE : 4;
  __I  uint32_t EDR_RX_FLOW : 1;
  __I  uint32_t EDR_RX_ARQN : 1;
  __I  uint32_t EDR_RX_SEQN    : 1;
  __I  uint32_t EDR_RX_AM_ADDR : 3;
  __I  uint32_t EDR_RX_L_CH    : 2;
  __I  uint32_t EDR_RX_P_FLOW  : 1;
  __I  uint32_t x732_rsv       : 1;
  __I  uint32_t EDR_RX_PKT        : 1;
  __I  uint32_t EDR_HEC_ERR       : 1;
  __I  uint32_t EDR_CRC_ERR       : 1;
  __I  uint32_t EDR_RX_HDR        : 1;
  __I  uint32_t EDR_FEC_ABORT     : 1;
  __I  uint32_t EDR_AM_ADDR_ABORT : 1;
  __I  uint32_t EDR_SYNC_ERR      : 1;
  __I  uint32_t x733_rsv          : 1;

  uint8_t x734_x73f[0x740-0x734];

  /* Transmit Memory */
  __IO uint32_t EDR_TX_ACL[(0xb78-0x740)/4]; //EDR: 1021B

  uint8_t xb78_xb7f[0xb80-0xb78];

  /* Receive Memory */
  __IO uint32_t EDR_RX_ACL[(0xfb8-0xb80)/4];
} HS_BTBB_LC_Type;

/**
 * @brief Bluetooth Baseband Radio
 */

typedef struct
{
  /* Receive Control Registers */
  __IO uint64_t SYNC_WORD;
  __IO uint32_t SYNC_ERR : 6;
  __IO uint32_t RX_MODE  : 1;
  __IO uint32_t SYNC_1X  : 1; //0-4X symbol rate trigger;  1-1X trigger
  __IO uint32_t DADC_BYP : 1;
  __IO uint32_t RADC_BYP : 1;
  __IO uint32_t RADC_NUM : 1;
  __IO uint32_t FDB_DIS  : 1;
  __IO uint32_t x09_rsv  : 4;
  __IO uint32_t SYM_GN   : 3;
  __IO uint32_t SYM_ENB  : 1;
  __IO uint32_t SYM_MASK : 1;
  __IO uint32_t CMB_C1   : 1;
  __IO uint32_t CMB_C2   : 2;
  __IO uint32_t OCL_DC      : 6;
  __IO uint32_t OCL_FB_MODE : 1;
  __IO uint32_t x0b_rsv     : 1;

  uint8_t x0c_x1f[0x20-0x0c];

  /* Interrupt Control Registers */
  __IO uint32_t RIF_INTR_MSK0 : 2;
  __IO uint32_t RIF_INTR_MSK1 : 2;
  __IO uint32_t RIF_INTR_MSK2 : 2;
  __IO uint32_t RIF_INTR_MSK3 : 2;
  __IO uint32_t LTR_INTR_MSK : 1;
  __IO uint32_t x21_rsv      : 7;
  __IO uint32_t x22 : 16;

  __O  uint32_t RIF_INTR_CLR : 4;
  __IO uint32_t LTR_INTR_CLR : 1; //ASA_INTR_CLR
  __IO uint32_t x24_rsv      : 3;
  __IO uint32_t x25 : 24;

  /* PHY Interface Configuration Registers */
  __IO union {
    struct {
      __IO uint32_t REFCLK_DIV  : 5;
      __IO uint32_t REFCLK_DIR  : 1;
      __IO uint32_t PHYCLK_GATE : 1;
      __IO uint32_t LPOCLK_DIV  : 1;
      __IO uint32_t OPEN_DRAIN : 1;
      __IO uint32_t RST_DIR    : 1;
      __IO uint32_t TXDATA_DIR : 1;
      __IO uint32_t TXDATA_FRC : 2;
      __IO uint32_t RXBDW_DIR  : 2;
      __IO uint32_t REFCLK_INV : 1;
      __IO uint32_t SYNC_INV  : 1;
      __IO uint32_t SYNC_RX   : 1;
      __IO uint32_t SYNC_TX   : 2;
      __IO uint32_t TX_INV    : 1;
      __IO uint32_t TXDIR_INV : 1;
      __IO uint32_t TX_FMT    : 1;
      __IO uint32_t RX_FMT    : 1;
      __IO uint32_t x2b_rsv0 : 1;
      __IO uint32_t DAT_BYP  : 1;
      __IO uint32_t OCL_BYP  : 1;
      __IO uint32_t x2b_rsv3 : 1;
      __IO uint32_t GAU_BYP  : 1;
      __IO uint32_t LTR_BYP  : 1;
      __IO uint32_t RX_INV   : 1;
      __IO uint32_t x2b_rsv7 : 1;
    } bits;
    uint32_t val;
  } PHY_CFG;

  /* Status Registers */
  __I  uint32_t RADC_RSSI      : 5;
  __I  uint32_t LTR_LONG_PERIO : 1;
  __I  uint32_t LTR_INTR       : 1; /* FIXME */
  __I  uint32_t x2c_rsv        : 1;
  __I  uint32_t RIF_INTR   : 4;
  __I  uint32_t GIO_STATUS : 4;
  __I  uint32_t MINOR_REV  : 8;
  __I  uint32_t MAJOR_REV  : 8;

  /* GIO Control (HIGH) Registers */
  __IO uint32_t GIO_HIGH_0_VAL      : 11;
  __IO uint32_t x31_rsv             : 2;
  __IO uint32_t GIO_HIGH_0_EXT      : 2;
  __IO uint32_t GIO_HIGH_0_OVERRIDE : 1;
  __IO uint32_t GIO_HIGH_1_VAL      : 11;
  __IO uint32_t x33_rsv             : 2;
  __IO uint32_t GIO_HIGH_1_EXT      : 2;
  __IO uint32_t GIO_HIGH_1_OVERRIDE : 1;
  __IO uint32_t GIO_HIGH_2_VAL      : 11;
  __IO uint32_t x35_rsv             : 2;
  __IO uint32_t GIO_HIGH_2_EXT      : 2;
  __IO uint32_t GIO_HIGH_2_OVERRIDE : 1;
  __IO uint32_t GIO_HIGH_3_VAL      : 11;
  __IO uint32_t x37_rsv             : 2;
  __IO uint32_t GIO_HIGH_3_EXT      : 2;
  __IO uint32_t GIO_HIGH_3_OVERRIDE : 1;
  __IO uint32_t GIO_HIGH_4_VAL      : 11;
  __IO uint32_t x39_rsv             : 2;
  __IO uint32_t GIO_HIGH_4_EXT      : 2;
  __IO uint32_t GIO_HIGH_4_OVERRIDE : 1;
  __IO uint32_t GIO_HIGH_5_VAL      : 11;
  __IO uint32_t x3b_rsv             : 2;
  __IO uint32_t GIO_HIGH_5_EXT      : 2;
  __IO uint32_t GIO_HIGH_5_OVERRIDE : 1;
  __IO uint32_t GIO_HIGH_6_VAL      : 11;
  __IO uint32_t x3d_rsv             : 2;
  __IO uint32_t GIO_HIGH_6_EXT      : 2;
  __IO uint32_t GIO_HIGH_6_OVERRIDE : 1;
  __IO uint32_t GIO_HIGH_7_VAL      : 11;
  __IO uint32_t x3f_rsv             : 2;
  __IO uint32_t GIO_HIGH_7_EXT      : 2;
  __IO uint32_t GIO_HIGH_7_OVERRIDE : 1;
  __IO uint32_t GIO_HIGH_8_VAL      : 11;
  __IO uint32_t x41_rsv             : 2;
  __IO uint32_t GIO_HIGH_8_EXT      : 2;
  __IO uint32_t GIO_HIGH_8_OVERRIDE : 1;
  __IO uint32_t GIO_HIGH_9_VAL      : 11;
  __IO uint32_t x43_rsv             : 2;
  __IO uint32_t GIO_HIGH_9_EXT      : 2;
  __IO uint32_t GIO_HIGH_9_OVERRIDE : 1;
  __IO uint32_t GIO_HIGH_10_VAL      : 11;
  __IO uint32_t x45_rsv              : 2;
  __IO uint32_t GIO_HIGH_10_EXT      : 2;
  __IO uint32_t GIO_HIGH_10_OVERRIDE : 1;
  __IO uint32_t GIO_HIGH_11_VAL      : 11;
  __IO uint32_t x47_rsv              : 2;
  __IO uint32_t GIO_HIGH_11_EXT      : 2;
  __IO uint32_t GIO_HIGH_11_OVERRIDE : 1;

  /* GIO Control (LOW) Registers */
  __IO uint32_t GIO_LOW_0_VAL      : 11;
  __IO uint32_t x49_rsv            : 2;
  __IO uint32_t GIO_LOW_0_EXT      : 2;
  __IO uint32_t GIO_LOW_0_OVERRIDE : 1;
  __IO uint32_t GIO_LOW_1_VAL      : 11;
  __IO uint32_t x4b_rsv            : 2;
  __IO uint32_t GIO_LOW_1_EXT      : 2;
  __IO uint32_t GIO_LOW_1_OVERRIDE : 1;
  __IO uint32_t GIO_LOW_2_VAL      : 11;
  __IO uint32_t x4d_rsv            : 2;
  __IO uint32_t GIO_LOW_2_EXT      : 2;
  __IO uint32_t GIO_LOW_2_OVERRIDE : 1;
  __IO uint32_t GIO_LOW_3_VAL      : 11;
  __IO uint32_t x4f_rsv            : 2;
  __IO uint32_t GIO_LOW_3_EXT      : 2;
  __IO uint32_t GIO_LOW_3_OVERRIDE : 1;
  __IO uint32_t GIO_LOW_4_VAL      : 11;
  __IO uint32_t x51_rsv            : 2;
  __IO uint32_t GIO_LOW_4_EXT      : 2;
  __IO uint32_t GIO_LOW_4_OVERRIDE : 1;
  __IO uint32_t GIO_LOW_5_VAL      : 11;
  __IO uint32_t x53_rsv            : 2;
  __IO uint32_t GIO_LOW_5_EXT      : 2;
  __IO uint32_t GIO_LOW_5_OVERRIDE : 1;
  __IO uint32_t GIO_LOW_6_VAL      : 11;
  __IO uint32_t x55_rsv            : 2;
  __IO uint32_t GIO_LOW_6_EXT      : 2;
  __IO uint32_t GIO_LOW_6_OVERRIDE : 1;
  __IO uint32_t GIO_LOW_7_VAL      : 11;
  __IO uint32_t x57_rsv            : 2;
  __IO uint32_t GIO_LOW_7_EXT      : 2;
  __IO uint32_t GIO_LOW_7_OVERRIDE : 1;
  __IO uint32_t GIO_LOW_8_VAL      : 11;
  __IO uint32_t x59_rsv            : 2;
  __IO uint32_t GIO_LOW_8_EXT      : 2;
  __IO uint32_t GIO_LOW_8_OVERRIDE : 1;
  __IO uint32_t GIO_LOW_9_VAL      : 11;
  __IO uint32_t x5b_rsv            : 2;
  __IO uint32_t GIO_LOW_9_EXT      : 2;
  __IO uint32_t GIO_LOW_9_OVERRIDE : 1;
  __IO uint32_t GIO_LOW_10_VAL      : 11;
  __IO uint32_t x5d_rsv             : 2;
  __IO uint32_t GIO_LOW_10_EXT      : 2;
  __IO uint32_t GIO_LOW_10_OVERRIDE : 1;
  __IO uint32_t GIO_LOW_11_VAL      : 11;
  __IO uint32_t x5f_rsv             : 2;
  __IO uint32_t GIO_LOW_11_EXT      : 2;
  __IO uint32_t GIO_LOW_11_OVERRIDE : 1;

  /* GIO Combine Registers */
  __IO uint32_t GIO_COMBINE_0  : 6;
  __IO uint32_t x60_rsv        : 1;
  __IO uint32_t GIO_AUTO_EXT_0 : 1;
  __IO uint32_t GIO_COMBINE_1  : 6;
  __IO uint32_t x61_rsv        : 1;
  __IO uint32_t GIO_AUTO_EXT_1 : 1;
  __IO uint32_t GIO_COMBINE_2  : 6;
  __IO uint32_t x62_rsv        : 1;
  __IO uint32_t GIO_AUTO_EXT_2 : 1;
  __IO uint32_t GIO_COMBINE_3  : 6;
  __IO uint32_t x63_rsv        : 1;
  __IO uint32_t GIO_AUTO_EXT_3 : 1;
  __IO uint32_t GIO_COMBINE_4  : 6;
  __IO uint32_t x64_rsv        : 1;
  __IO uint32_t GIO_AUTO_EXT_4 : 1;
  __IO uint32_t GIO_COMBINE_5  : 6;
  __IO uint32_t x65_rsv        : 1;
  __IO uint32_t GIO_AUTO_EXT_5 : 1;
  __IO uint32_t GIO_COMBINE_6  : 6;
  __IO uint32_t x66_rsv        : 1;
  __IO uint32_t GIO_AUTO_EXT_6 : 1;
  __IO uint32_t GIO_COMBINE_7  : 6;
  __IO uint32_t x67_rsv        : 1;
  __IO uint32_t GIO_AUTO_EXT_7 : 1;
  __IO uint32_t GIO_COMBINE_8  : 6;
  __IO uint32_t x68_rsv        : 1;
  __IO uint32_t GIO_AUTO_EXT_8 : 1;
  __IO uint32_t GIO_COMBINE_9  : 6;
  __IO uint32_t x69_rsv        : 1;
  __IO uint32_t GIO_AUTO_EXT_9 : 1;
  __IO uint32_t GIO_COMBINE_10  : 6;
  __IO uint32_t x6a_rsv         : 1;
  __IO uint32_t GIO_AUTO_EXT_10 : 1;
  __IO uint32_t GIO_COMBINE_11  : 6;
  __IO uint32_t x6b_rsv         : 1;
  __IO uint32_t GIO_AUTO_EXT_11 : 1;

  uint8_t x6c_x6f[0x70-0x6c];

  /* Miscellaneous RF Control Registers */
  __IO uint32_t RF_SUPPORT : 3;
  __IO uint32_t x70_rsv    : 5;
  __IO uint32_t x71 : 24;

  uint8_t x74_x7f[0x80-0x74];

  /* GIO Window Widening Registers */
  __IO uint32_t GIO_HIGH_EARLY_EN : 12;
  __IO uint32_t GIO_HIGH_LATE_EN  : 12;
  __IO uint32_t x83               : 8;
  __IO uint32_t GIO_LOW_EARLY_EN : 12;
  __IO uint32_t GIO_LOW_LATE_EN  : 12;
  __IO uint32_t x87              : 8;

  uint8_t x88_x9f[0xa0-0x88];

  /* RF Interfacing Selection & Multiplexing */
  union {
    struct {
      __IO uint32_t SYNC_RXBDW_TRCLK      : 1;
      __IO uint32_t SYNC_RXBDW_TRCLK_EDGE : 1;
      __IO uint32_t xa0_rsv               : 6;
      __IO uint32_t RFMUX       : 3;
      __IO uint32_t xa1_rsv     : 2;
      __IO uint32_t AUTO_WAKEUP : 3;
      __IO uint32_t CLK_DIVIDER : 4;
      __IO uint32_t xa2 : 12;
    } bits;
    uint32_t val;
  } INF_MUX;
} HS_BTBB_RF_Type;

/**
 * @brief Bluetooth Baseband Low Energy
 */

typedef struct
{
  //struct {
    __IO uint32_t LE_MODE     : 1;
    __IO uint32_t ADDR_TYPE   : 1;
    __IO uint32_t MASTER_MODE : 1;
    __IO uint32_t ADV_STATE   : 1;
    __IO uint32_t TX_EN       : 1;
    __IO uint32_t AES_MODE    : 2;
    __IO uint32_t AES_ENABLE  : 1;

    __IO uint32_t T_IFS_DEFAULT : 1;
    __IO uint32_t T_IFS_ABORT   : 1;
    __IO uint32_t T_IFS_DELAY   : 6;

    __IO uint32_t WHITEN_INIT : 6;
    __IO uint32_t WHITEN_EN	: 1;
    __IO uint32_t TEST_CRC	: 1;

    __IO uint32_t DELAY_SRCH_WIN : 6;
    __IO uint32_t AES_START      : 1;
    __IO uint32_t AES_DATA_READY : 1;
  //} COM_CTRL_GP1;

  uint8_t x04_x13[0x14-0x04];

  __IO uint32_t ACC_ADDR;

  __I  uint32_t PREAMBLE : 8;
  __IO uint32_t CRC_ERROR : 1;

  __IO uint32_t CRC_INIT;

  __IO uint32_t AES_KEY[(0x30-0x20)/4]; /* big endian */
  __IO uint32_t AES_IV[(0x38-0x30)/4];  /* big endian */

  __IO uint32_t AES_PKT_CNT0 : 8;
  __IO uint32_t AES_PKT_CNT1 : 8;
  __IO uint32_t AES_PKT_CNT2 : 8;
  __IO uint32_t AES_PKT_CNT3 : 8;
  __IO uint32_t AES_PKT_CNT4 : 8;
  __IO uint32_t AES_PKT_LENGTH : 5;
  __IO uint32_t AES_PKT_LLID   : 2;
  uint32_t x3d_rsv : 1;

  uint8_t x3e_x3f[0x40-0x3e];

  /* Transmitter Header Registers - Advertising/Data channel PDU */
  __IO struct {
    uint32_t PDU_TYPE : 4;
    uint32_t RFU_ADVlo : 2;
    uint32_t TX_ADD   : 1;
    uint32_t RX_ADD   : 1;

    uint32_t TX_LENGTH_ADV : 6;
    uint32_t RFU_ADVhi     : 2;

    uint32_t LLID : 2;
    uint32_t NESN : 1;
    uint32_t SN   : 1;
    uint32_t MD   : 1;
    uint32_t RFU_DATAlo : 3;

    uint32_t TX_LENGTH_DATA : 5;
    uint32_t RFU_DATAhi : 3;
  } TX_HEADER_PDU;

  /* Receiver Header Registers - Advertising/Data channel PDU */
  union {
    __IO struct {
      uint32_t PDU_TYPE : 4;
      uint32_t RFU_ADVlo : 2;
      uint32_t TX_ADD   : 1;
      uint32_t RX_ADD   : 1;

      uint32_t TX_LENGTH_ADV : 6;
      uint32_t RFU_ADVhi     : 2;

      uint32_t x46_adv : 16;
    } RX_HEADER_PDU_ADV;

    __IO struct {
      uint32_t LLID : 2;
      uint32_t NESN : 1;
      uint32_t SN   : 1;
      uint32_t MD   : 1;
      uint32_t RFU_DATAlo : 3;

      uint32_t TX_LENGTH_DATA : 5;
      uint32_t RFU_DATAhi : 3;

      uint32_t x46_data : 16;
    } RX_HEADER_PDU_DATA;
  } RX_HEADER_PDU;

  uint8_t x48_x53[0x54-0x48];

  __IO struct {
    uint32_t MIC_STATUS : 1;
    uint32_t AES_ACTIVE : 1;
    uint32_t AES_FINISHED : 1;
    uint32_t rsv          : 3;
    uint32_t TIFS_ABORTED : 1;
    uint32_t CRC_ERROR    : 1;

    uint32_t LE_RX_FILTERED : 3;
    uint32_t TARGET_ADDR_MATCH : 1;

    uint32_t x56_rsv : 20;
  } LE_STATUS;

  __IO struct {
    uint32_t SPI_TIMED_EN : 1;
    uint32_t x58_rsv      : 1;
    uint32_t MASTER_MODE  : 1;
    uint32_t ADV_STATE    : 1;
    uint32_t TX_EN        : 1;
    uint32_t LE_FILTER_MODE : 3;

    uint32_t T_IFS_DEFAULT : 1;
    uint32_t T_IFS_ABORT   : 1;
    uint32_t x59_rsv       : 3;
    uint32_t USE_ADDR_FLTRS : 1;
    uint32_t USE_ADDR_FLTRS_CON_REQ : 1;
    uint32_t USE_ADDR_FLTRS_SCAN_REQ : 1;

    uint32_t FLTR_0_LOADED : 1;
    uint32_t FLTR_1_LOADED : 1;
    uint32_t FLTR_2_LOADED : 1;
    uint32_t FLTR_3_LOADED : 1;
    uint32_t FLTR_4_LOADED : 1;
    uint32_t FLTR_5_LOADED : 1;
    uint32_t FLTR_IMD_LOADED : 1;

    uint32_t x5b_rsv : 9;
  } LE_CONTROL;

  uint8_t x5c_x5f[0x60-0x5c];

  /* LE Address Filter Registers */
  __IO uint32_t FLTR_ADDR_0[8/4]; /* msb: FLTR_LOADED, FLTR_ADDR_TYPE, FLTR_MATCHED */
  __IO uint32_t FLTR_ADDR_1[8/4];
  __IO uint32_t FLTR_ADDR_2[8/4];
  __IO uint32_t FLTR_ADDR_3[8/4];
  __IO uint32_t FLTR_ADDR_4[8/4];
  __IO uint32_t FLTR_ADDR_5[8/4];
  __IO uint32_t FLTR_ADDR_DIRECT[8/4];
  __IO uint32_t LE_LOCAL_ADDR[8/4];

  uint8_t x0a0_x1ff[0x200-0x0a0];

  __IO uint32_t AES_MIC;
  __IO uint32_t AES_DATA[(0x220-0x204)/4];
} HS_BTBB_LE_Type;

/**
 * @brief Bluetooth MODEM/PHY, modified according to the HS6600_BTPHY_REG_v1.6_20150109.xlsx document.
 */

typedef struct
{
  __IO uint32_t RX_STR_EN;                     //offset address 0x0000
  __IO uint32_t XCORR_MOD_SEL;
  __IO uint32_t BLE_XCORR_TH;
  __IO uint32_t FSYNC_DET_SEL_IDX;
  __IO uint32_t CFO_FB_CNT_TH_SEL;             //offset address 0x0010
  __IO uint32_t CFO_CAP_TH;
  __IO uint32_t EN_CFO_EST;
  __IO uint32_t CFO_FOR_DPSK_SEL;
  __IO uint32_t CFO_CAP_FSYNC;                 //offset address 0x0020
  __IO uint32_t RX_TOP_CFO_MODE;
  __IO uint32_t H_IDX;
  __IO uint32_t TX_FILT_GAIN_CMP;
  __IO uint32_t EN_GFSK_24X_SAT;               //offset address 0x0030
  __IO uint32_t TX_EDR2_DLY_SEL;
  __IO uint32_t EN_DC_REMOVAL;
  __IO uint32_t DCNOTCH_K_SEL;
  __IO uint32_t RES_32X_EN;                    //offset address 0x0040
  __IO uint32_t ERR_SCALE_SEL;
  __IO uint32_t K1_GFSK_SEL;
  __IO uint32_t K2_GFSK_SEL;
  __IO uint32_t K1_DPSK_SEL;                   //offset address 0x0050
  __IO uint32_t K2_DPSK_SEL;
  __IO uint32_t K2_BYPASS;
  __IO uint32_t SW_VLD_DLY_SEL;
  __IO uint32_t SW_WIN_LTH_OFFSET;             //offset address 0x0060
  __IO uint32_t SW_WIN_HTH_OFFSET;
  __IO uint32_t DEM_DLY_BKOFF_GFSK;
  __IO uint32_t DEM_DLY_BKOFF_DPSK;
  __IO uint32_t N_EXTRA_OFFSET_SEL;            //offset address 0x0070
  __IO uint32_t N_FTUNE_GFSK_SEL;
  __IO uint32_t N_FTUNE_DPSK_SEL;
  __IO uint32_t XCORR_TH_GFSK_SEL;
  __IO uint32_t XCORR_TH_DPSK_SEL;             //offset address 0x0080
  __IO uint32_t XCORR_TH_BLE_SEL;
  __IO uint32_t CNT2TH;
  __IO uint32_t K_FIX0_2_TIMEOUT_SL;
  __IO uint32_t x90;                           //offset address 0x0090
  __IO uint32_t x94;
  __IO uint32_t FBFB_ACC_FIX_FSYNC;
  __IO uint32_t RX_FE_CFO_EST_K;
  __IO uint32_t CHF_INI_SEL;                   //offset address 0x00A0
  __IO uint32_t EN_CFO_CMP;
  __IO uint32_t CFO_CMP_DIS_LTH;
  __IO uint32_t CFO_CMP_DIS_HTH;
  __IO uint32_t EN_FAGC;                       //offset address 0x00B0
  __IO uint32_t LWIN;
  __IO uint32_t RX_FAGC_REF;
  __IO uint32_t GFAGC_REG;
  __IO uint32_t GFAGC_WEN;                     //offset address 0x00C0
  __IO uint32_t SNR_EST;
  __IO uint32_t RSSI_TIMEOUT_CNST;
  __IO uint32_t RSSI_SAVE_MODE;
  __IO uint32_t RSSI_K_SEL;                    //offset address 0x00D0
  __IO uint32_t RSSI_EST_SEL;
  __IO uint32_t RSSI_EST_REAL_TIME;
  __IO uint32_t SIG_DBM_EST;
  __IO uint32_t K_CHG_INI0;                    //offset address 0x00E0
  __IO uint32_t K_CHG_INI1;
  __IO uint32_t K_DCY_INI0;
  __IO uint32_t K_DCY_INI1;
  __IO uint32_t MAXMIN0_LIM;                   //offset address 0x00F0
  __IO uint32_t MAXMIN1_LIM;
  __IO uint32_t DC_LIM;
  __IO uint32_t DCY_COND_TH_SEL;
  __IO uint32_t VAR_DCY_FSYNC_EN;              //offset address 0x0100
  __IO uint32_t K_CHG_LIM;
  __IO uint32_t K_DCY_LIM;
  __IO uint32_t CNT_CHG_TH_SEL;
  __IO uint32_t CNT_DCY_TH_SEL;                //offset address 0x0110
  __IO uint32_t FINE_DCY_SEL;
  __IO uint32_t DC_LPF_SEL;
  __IO uint32_t REF_TH_SEL;
  __IO uint32_t K_DCY_CNT_EN;                  //offset address 0x0120
  __IO uint32_t K_DCY_REF_EN;
  __IO uint32_t K_DCY_COND_EN;
  __IO uint32_t K_CHG_CNT_EN;
  __IO uint32_t K_CHG_REF_EN;                  //offset address 0x0130
  __IO uint32_t K_CHG_COND_EN;
  __IO uint32_t K_SLOPE_SEL;
  __IO uint32_t K_DC_LPF;
  __IO uint32_t VAR_DCY_SLOPE0;                //offset address 0x0140
  __IO uint32_t VAR_DCY_SLOPE1;
  __IO uint32_t VAR_DX_EN_RANGE0;
  __IO uint32_t VAR_DX_EN_RANGE1;
  __IO uint32_t VAR_DCY0_EN;                   //offset address 0x0150
  __IO uint32_t VAR_DCY1_EN;
  __IO uint32_t NOISE_DET_EN;
  __IO uint32_t MAXMIN_LIM_SEL;
  __IO uint32_t RESERVED0;                     //offset address 0x0160
  __IO uint32_t TX_EDR3_DLY_SEL;
  __IO uint32_t GAU_DLY_CNT;
  __IO uint32_t GUARD_DLY_CNT;
  __IO uint32_t TX_EXT_SEL;                    //offset address 0x0170
  __IO uint32_t TX_RX_EN;
  __IO uint32_t x178;
  __IO uint32_t BT_PHY_ID;
  __IO uint32_t RX_RVS_IF_ROT;                 //0x0180
  __IO uint32_t TX_DPGA_GC;
  __IO uint32_t x188;
  __IO uint32_t x18c;
  __IO uint32_t EN_IQ_IMBALANCE_CMP;           //0x190
  __IO uint32_t IQ_RSSI_K_SEL;
  __IO uint32_t IQ_CMP_K_SEL;
  __IO uint32_t BLE_SYNC_MUX_OUT;
  __IO uint32_t RX_FE_RSSI_DEBUG;              //0x1A0
  __IO uint32_t SYNC_DETC_TH_SEL;
  __IO uint32_t x1c4_x1a8[8];
  __IO uint32_t DIS_PLD_CFO_CMP;
  __IO uint32_t EN_ANTI_INTF;
  __IO uint32_t MAXMIN0_LIM2;                  //offset address 0x01D0
  __IO uint32_t MAXMIN1_LIM2;
  __IO uint32_t CLR_TH;
  __IO uint32_t EN_CLR;
  __IO uint32_t DEM_DLY_BKOFF_GFSK2;           //offset address 0x01E0
  __IO uint32_t RX_EN_STR_ST1_WRT;
  __IO uint32_t EN_CFO_ACC_CLR;
  __IO uint32_t EN_CFO_CLR_AFTER_SYNC;
  __IO uint32_t CFO_ACC_SAT_LIM;               //offset address 0x01F0
  __IO uint32_t DCNOTCH_K2;
  __IO uint32_t DC_MODE;
  __IO uint32_t STR_XCORR_CUT;
  __IO uint32_t TH_SEL;                        //0x200
  __IO uint32_t ADC_CLR_DUR_SEL;
  __IO uint32_t SAT_CNT_TH_SEL;
  __IO uint32_t CNT_MODE;
  __IO uint32_t A_REG;                         //0x210
  __IO uint32_t C_REG;
  __IO uint32_t PD_CLR_DUR_SEL;
  __IO uint32_t DIN_SEL;
  __IO uint32_t PD_DET_MODE;                   //0x220
  __IO uint32_t BUF_L_SEL;
  __IO uint32_t TH_ANALOG;
  __IO uint32_t BLE_XCORR_WIN_TH;
  __IO uint32_t GFSK_XCORR_WIN_TH;             //0x230
  __IO uint32_t DPSK_XCORR_WIN_TH;
  __IO uint32_t IF_REG;
  __IO uint32_t IQ_IN_SWAP;
  __IO uint32_t FM_MODE;                       //0x240
  __IO uint32_t FM_RSSI;
  __IO uint32_t RMV2_DEEM;
  __IO uint32_t FM_STEREO;
  __IO uint32_t FM_CHHC_FILT;                  //0x250
  __IO uint32_t FM_LRHC_FILT;
  __IO uint32_t SPI_APB_SWITCH;
  __IO uint32_t CFO_EST_ACC_TRUNC;
  __IO uint32_t FSYNC_EARLY;                   //0x260
  __IO uint32_t CFO_CAP_FOR_DPSK;
  __IO uint32_t TX_IQ_MUX;
  __IO uint32_t FM_CORDIC_BP;
  __IO uint32_t TX_IF_REG;                     //0x270
  __IO uint32_t FM_DECIMATION;
  __IO uint32_t EXTRA_MU_COND_SEL;
  __IO uint32_t x27c;
  __IO uint32_t x3ff_x280[(0x400-0x280)/4];    //0x280

  __IO uint32_t ANALOGUE[0x60];                //offset address: 0x400-0x7FF
} HS_BTPHY_Type;

struct _HS_USB_Type
{
  __IO uint8_t      USB_ADDR;       //00
  __IO uint8_t      USB_POWER;      //01
  __IO uint16_t     USB_INTTX;      //02
  __IO uint16_t     USB_INTRX;      //04
  __IO uint8_t      USB_INTUSB;     //06
  __IO uint16_t     USB_INTTXEN;    //07
  __IO uint16_t     USB_INTRXEN;    //09
  __IO uint8_t      USB_INTUSBEN;   //0b
  __IO uint16_t     USB_FRAME;      //0c
  __IO uint8_t      USB_INDEX;      //0e
  __IO uint8_t      USB_DEVCTL;     //0f
  __IO uint8_t      USB_TXMAXP;     //10
  union
  {
    __IO uint16_t   USB_CSR0;       //11
    __IO uint16_t   USB_TXCSR;
  };
  __IO uint8_t      USB_RXMAXP;     //13
  __IO uint16_t     USB_RXCSR;      //14
  union
  {
    __IO uint8_t    USB_COUNT0;     //16
    __IO uint8_t    USB_RXCOUNT1;
  };
  __IO uint8_t      USB_RXCOUNT2;   //17
  __IO uint8_t      USB_TXTYPE;     //18
  union
  {
    __IO uint8_t    USB_NAKLIMIT0;  //19
    __IO uint8_t    USB_TXINTERVAL;
  };
  __IO uint8_t      USB_RXTYPE;     //1a
  __IO uint8_t      USB_RXINTERVAL; //1b
  __IO uint16_t     USB_TXFIFO;     //1c
  __IO uint16_t     USB_RXFIFO;     //1e
  __IO uint32_t     USB_EPxFIFO[4]; //20
} __attribute__ ((packed));

struct _HS_USB_DMA_Type
{
    __IO uint32_t   USB_DMA_INTR;
    __IO uint32_t   USB_DMA_CTL;
    __IO uint32_t   USB_DMA_ADDR;
    __IO uint32_t   USB_DMA_CNT;
};

typedef struct _HS_USB_Type     HS_USB_Type;
typedef struct _HS_USB_DMA_Type HS_USB_DMA_Type;


/**
 * @brief adc Interface
 */
typedef struct
{
  __IO uint32_t adc_intr;                 /* 0x00 */
  __IO uint32_t adc_intr_mask;
  __IO uint32_t adc_cfg0;
  __IO uint32_t adc_cfg1;
  __IO uint32_t adc_chselr;               /* 0x10 */
  __IO uint32_t adc_timing_cfg0;
  __IO uint32_t adc_timing_cfg1;
  __IO uint32_t adc_flag_fsm;
  __IO uint32_t adc_debug_reg0;           /* 0x20 */
  __IO uint32_t adc_debug_reg1;
  __IO uint32_t adc_ana_reg0;
  __IO uint32_t adc_ana_reg1;
  __IO uint32_t adc_ana_reg2;             /* 0x30 */
  __IO uint32_t adc_ana_reg3;
  __IO uint32_t adc_ana_reg4;
  __IO uint32_t adc_ana_reg5;
  __IO uint32_t adc_ana_reg6;             /* 0x40 */
  __IO uint32_t adc_ana_reg7;
  __IO uint32_t adc_ana_reg8;
  __IO uint32_t adc_data_reg0;
  __IO uint32_t adc_data_reg1;
  __IO uint32_t adc_data_reg2;
  __IO uint32_t adc_data_reg3;
  __IO uint32_t adc_data_reg4;
  __IO uint32_t adc_data_reg5;
  __IO uint32_t adc_data_reg6;
  __IO uint32_t adc_data_reg7;
  __IO uint32_t adc_data_reg8;
  __IO uint32_t adc_fifo_con;
  __IO uint32_t adc_dma_con;
  __IO uint32_t adc_rxfifo_th;
  __IO uint32_t adc_rxfifo_fl;
  __IO uint32_t adc_rxfifo_data;
  __IO uint32_t adc_debug_data;
  __IO uint32_t adc_debug_data_sample;
  __IO uint32_t adc_test_data;
} HS_ADC_Type;

/** 
 * @brief Analog and Digital interface, replace of HS_BTPHY_Type.ANALOGUE[]
 */

typedef struct
{
  /************ CODEC ************/
  /* reg<0> @0x40=word0 */
  __IO uint32_t MICBIAS_CC        : 1; /* change MICBIAS compensation method: b'0=w/o ext cap; ?b'1=w/ ext cap in 2uF */
  /* reg<1> */
  __IO uint32_t DRV_SEL_INPUT     : 1; /* select DRV input: 0=DAC; 1=PGA */
  /* reg<2> */
  __IO uint32_t EN_ADC_DITHER     : 1; /* enable ADC dithering: 1=enable; 0=disable */
  /* reg<3> */
  __IO uint32_t EN_ADC_DEM        : 1; /* enable ADC DEM:       1=enable; 0=disable */
  /* reg<6:4> */
  __IO uint32_t AUADC_TSTSEL      : 3; /* ADC test selection:   b'000=no test */
  /* reg<7> */
  __IO uint32_t RST_ADC           : 1; /* reset audio ADC:      0=no reset; 1=reset */
  /* reg<8> */
  __IO uint32_t SHRT_ADC          : 1; /* short audio ADC for test: 0=no short; 1=short */
  /* reg<9> */
  __IO uint32_t TST_AUADC_BIAS    : 1; /* enable ADC bias current test: 0=no test; 1=enable */
  /* reg<12:10> */
  __IO uint32_t PGA_GAIN          : 3; /* select audio PGA gain: b'000 (?b'010)
                                        [-6, -3, 0, 3, 6, 12, 18, 24]dB */
  /* reg<16:13> */
  __IO uint32_t RC_TUNE           : 4; /* tuning RC constant: b'1000 */
  /* reg<17> */
  __IO uint32_t SEL_ADC_INPUT     : 1; /* select ADC input: 0=normal input; 1=test input */
  /* reg<18> */
  __IO uint32_t SEL_PGA_INPUT     : 1; /* select PGA input: 0=mic; 1=line-in */
  /* reg<20:19> */
  __IO uint32_t LDO_AUD_CTRL      : 2; /* AUDIO LDO @2.8V adjust: b'01 */
  /* reg<22:21> */
  __IO uint32_t LDO_DRV_CTRL      : 2; /* DRV   LDO @2.8V adjust: b'01 */
  /* reg<23> */
  __IO uint32_t AUDIO_BIAS_TST    : 1; /* enable AUDIO bias current test: 0=no test; 1=enable */
  /* reg<26:24> */
  __IO uint32_t AUDIO_TSTSEL      : 3; /* audio top test selection: b'000=no test */
  /* reg<27> */
  __IO uint32_t REF_BYP           : 1; /* bypass reference's big-R for fast settling: 0 */
  /* reg<28> */
  __IO uint32_t REF_FAST          : 1; /* quick start for reference ramp-up: 0 */
  /* reg<30:29> */
  __IO uint32_t BIAS_SEL          : 2; /* adjust audio bias gen current: b'01
                                        [7.5, 10, 12.5, 15]uA */
  /* reg<31> */
  __IO uint32_t DET_SEL           : 1; /* select plug-in detect input: 0=select VGPIO1; 1=select VGPIO2 */

  /* reg<32> @0x44=word1 */
  __IO uint32_t TST_EN            : 1; /* enable audio test function: 0 */
  /* reg<33> */
  __IO uint32_t ADDA_SELF_TEST    : 1; /* enable AD-DA self test: 0=no test; 1=self test */
  /* reg<34> */
  __IO uint32_t EN_DAC_DEM        : 1; /* enable DAC DEM:        1=enable; 0=disable */
  /* reg<35> */
  __IO uint32_t DRV_OCP_EN        : 1; /* enable DRV OCP detect: 0=disable; 1=enable */
  /* reg<36> */
  __IO uint32_t DRV_GAIN          : 1; /* DRV gain control: b'0=-6dB; ?b'1=0dB */
  /* reg<40:37> */
  __IO uint32_t MICBIAS_TUNE      : 4; /* turning MICBIAS out: b'0000 */
  __IO uint32_t dummy_41          : 1;

  /************ PMU **************/
  /* reg<43:42> */
  __IO uint32_t DCDC_DUTY         : 2; /* DC-DC clock duty: b'00 (?b'01) */
  /* reg<45:44> */
  __IO uint32_t LDO_LOBUF_CTRL    : 2; /* LOBUF LDO adjust: b'01 (?b'11=lowest) */
  __IO uint32_t dummy_49_46       : 4;
  /* reg<51:50> */
  __IO uint32_t LDO_TRX_ADDA_ADJ  : 2; /* TRX AD/DA  LDO @1.5V adjust: b'01 */
  /* reg<53:52> */
  __IO uint32_t LDO_TRX_ANA_ADJ   : 2; /* TRX analog LDO @1.5V adjust: b'01 */
  /* reg<55:54> */
  __IO uint32_t LDO_TRX_RF_ADJ    : 2; /* TRX RF     LDO @1.5V adjust: b'01 */
  /* reg<57:56> */
  __IO uint32_t LDO_ANA_DIG_ADJ   : 2; /* Digital in Analog LDO @1.3V adjust: b'01 */
  /* reg<58> */
  __IO uint32_t CHG_I_ADJ         : 1; /* charger current adjust: 1=100mA; 0=350mA */
  /* reg<60:59> */
  __IO uint32_t LDO_DIG_ADJ       : 2; /* Digital    LDO @1.2V adjust: b'01
                                        [xx, 1.1, 1.2, 1.3]V */
  /* reg<62:61> */
  __IO uint32_t LDO_TOP_ADJ       : 2; /* Top layer  LDO @2.0V adjust: b'01 */

  /* reg<64:63> cross the word boudary */
  __IO uint32_t LDO_RF_ADJ_lo     : 1; /* RF         LDO @2.8V adjust: b'01 */
  /*    <64>    @0x48=word2 */
  __IO uint32_t LDO_RF_ADJ_hi     : 1;
  /* reg<66:65> */
  __IO uint32_t LDO_XTAL_ADJ      : 2; /* XTAL       LDO @2.8V adjust: b'01 */
  /* reg<68:67> */
  __IO uint32_t LDO_BTVCO_ADJ     : 2; /* BT VCO     LDO @2.2V adjust: b'01 */
  /* reg<70:69> */
  __IO uint32_t LDO_IO_ADJ        : 2; /* IO         LDO @2.2V adjust: b'01 */
  __IO uint32_t dummy_72_71       : 2;
  /* reg<75:73> */
  __IO uint32_t PMU_TEST_SEL      : 3; /* PMU test select: b'000 */
  /* reg<76> */
  __IO uint32_t PMU_TEST_EN       : 1; /* PMU test enable: 0 */
  /* reg<78:77> */
  __IO uint32_t LDO_ADCBIAS_ADJ   : 2; /* ADC bias and level-1 DAC LDO @2.5V adjust: b'01 */
  __IO uint32_t dummy_87_79       : 9;

  /************ FM ***************/
  /* reg<90:88> */
  __IO uint32_t VB_MIX            : 3; /* FM Mixer bias voltage: b'011 (?b'100) */
  __IO uint32_t dummy_95_91       : 5;
  /*    <96>    @0x4C=word3 */
  __IO uint32_t dummy_102_96      : 7;
  /* reg<106:103> */
  __IO uint32_t FM_PDET           : 4; /* FM AGC threshold: b'0110 */
  /* reg<108:107> */
  __IO uint32_t FM_PDET_SPACE     : 2; /* FM AGC latency space: b'10 */
  __IO uint32_t dummy_114_109     : 6;

  /************ RF RX ************/
  /* reg<117:115> */
  __IO uint32_t RXMIX_VB          : 3; /* BT MIX's DC bias voltage: b'011 (?b'100) */
  __IO uint32_t dummy_127_118     : 10;
  /*    <128>   @0x50=word4 */
  __IO uint32_t dummy_128         : 1;
  /* reg<129> */
  __IO uint32_t RXADC_SEL_INPUT   : 1; /* select RXADC input: 0=BT RX; 1=BT TX cali or FM */
  /* reg<130> */
  __IO uint32_t RXADC_SHRT        : 1; /* short RXADC input:  0=no short */
  /* reg<132:131> */
  __IO uint32_t RXADC_TST_DACBIAS : 2; /* test RXADC's feedback DAC bias current: 0=no test */
  /* reg<133> */
  __IO uint32_t RXADC_TST_IBIAS   : 1; /* test RXADC's bias current: 0=no test */
  /* reg<136:134> */
  __IO uint32_t RXADC_TSTSEL      : 3; /* select the RXADC's test output: b'000 (?b'111) */
  __IO uint32_t dummy_138_137     : 2;

  /************ RF TX ************/
  /* reg<142:139> */
  __IO uint32_t TXDAC_BW_CAL      : 4; /* TXDAC filter's RC tuning: b'0110 */
  /* reg<143> */
  __IO uint32_t RSEL_UM           : 1; /* TX UM's load resistor: b'0 (?b'1) */
  /* reg<145:144> */
  __IO uint32_t GSEL_PA           : 2; /* control BT PA's gain: b'00
                                        [0, 6, 12, xx]dB */
  /* reg<147:146> */
  __IO uint32_t ISEL_UM           : 2; /* control BT TX UM's current: b'10 */
  /* reg<149:148> */
  __IO uint32_t ISEL_TXPA1        : 2; /* control BT PA driver's bias currnet: b'01 */
  /* reg<151:150> */
  __IO uint32_t ISEL_TXPA2        : 2; /* control BT PA's bias currnet: b'01 */
  /* reg<153:152> */
  __IO uint32_t VSEL_UM           : 2; /* control BT TX UM's LO input's DC bias voltage: b'10 (?b'01) */
  /* reg<156:154> */
  __IO uint32_t VSEL_PA           : 3; /* control PA Cascode diode's bias voltage: b'100 */
  /* reg<157> */
  __IO uint32_t TXDAC_DEM_EN      : 1; /* enable TXDAC's DEM: 1-enable */
  /* reg<159:158> */
  __IO uint32_t TXDAC_GAIN        : 2; /* control TXDAC's gain by adjust current: b'01 */
  /* reg<161:160> @0x54=word5 */
  __IO uint32_t TXDAC_SWLDO_GAIN  : 2; /* adjust TXDAC's ?: b'01 */
  __IO uint32_t dummy_169_162     : 8;

  /************ system PLL *******/
  /* reg<171:170> */
  __IO uint32_t FDR_CTRL          : 2; /* system PLL's LDO control: b'01 */
  __IO uint32_t dummy_191_172     : 20;
  /*    <192>   @0x58=word6 */
  __IO uint32_t dummy_202_192     : 11;
  /* reg<203> */
  __IO uint32_t FTUNING_MN        : 1; /* system PLL's VCO ftuning from: 0=fsm; 1=reg */
  /* reg<211:204> */
  __IO uint32_t FTUNING_REG       : 8; /* system PLL's VCO ftuning reg: b'10000000 */
  /* reg<212> */
  __IO uint32_t CTUNING_MN        : 1; /* system PLL's VCO ctuning from: 0=fsm; 1=reg */
  /* reg<217:213> */
  __IO uint32_t CTUNING_REG       : 5; /* system PLL's VCO ctuning reg: b'10000 */
  /* reg<220:218> */
  __IO uint32_t PLL_LOCK_SEL      : 3; /* system PLL lock select: b'111=200us */
  __IO uint32_t dummy_221         : 1;

  /************ MISC ***************/
  /* reg<222> */
  __IO uint32_t PA_LDO_BW         : 1; /* control PA LDO's bandwidth: 0=narrow */
  __IO uint32_t dummy_223         : 1;
  /*    <224>   @0x5C=word7 */
  __IO uint32_t dummy_230_224     : 7;

  /* reg<232:231> */
  __IO uint32_t VREF_VC_DET       : 2; /* BT PLL's thermal compensation: b'01 */
  /* reg<234:233> */
  __IO uint32_t CON_CP_IB         : 2; /* BT PLL's CP bias current: b'01 */
  /* reg<236:235> */
  __IO uint32_t ICP_CTRL          : 2; /* BT PLL's CP current: b'01 */
  /* reg<237> */
  __IO uint32_t PD_LDOCALI_CAMP   : 1; /* LDO calibration compare power down: 1 */
  /* reg<238> */
  __IO uint32_t LDOCALI_SWAP      : 1; /* LDO calibration compare's input swap: 0 */
  /* reg<241:239> */
  __IO uint32_t EN_LDO            : 3; /* LDO calibration enable: b'000 */
  /* reg<244:242> */
  __IO uint32_t CON_VREF_PEAKDET  : 3; /* BT VCO peak-peak detect's reference voltage: b'010 */
  /* reg<246:245> */
  __IO uint32_t CON_LDO_DIV       : 2; /* control divider's LDO: b'01 */
  __IO uint32_t dummy_248_247     : 2;
  /* reg<250:249> */
  __IO uint32_t CON_LDO_MMD       : 2; /* BT PLL's MMD LDO control: b'01 */
  /* reg<252:251> */
  __IO uint32_t CON_LDO_PFD       : 2; /* BT PLL's PFD LDO control: b'01 */
  /* reg<254:253> */
  __IO uint32_t CON_LDO_CP        : 2; /* BT PLL's  CP LDO control: b'01 */
  /* reg<255> */
  __IO uint32_t ICON_XTAL         : 1; /* XTAL's current: 1=bigger */
  /* reg<257:256> @0x60=word8 */
  __IO uint32_t CON_LDO_XTAL      : 2; /* XTAL's LDO @1.2V: b'01 */
  /* reg<262:258> */
  __IO uint32_t CCON_XTAL         : 5; /* XTAL's load capacity: b'10000
                                        16=+ 0kHz
                                        12=+20kHz
                                         8=+40kHz
                                         4=+60kHz
                                        20=-10kHz
                                        24=-20kHz
                                        28=-30kHz
                                        31=-40kHz */
  /* reg<264:263> */
  __IO uint32_t RESCON_XTAL       : 2; /* XTAL's bias current: b'01 */
  /* reg<265> */
  __IO uint32_t RF_PLL_MODE       : 1; /* RF PLL mode: 1=BT; 0=FM */
  /* reg<266>, ENDIV2_N obsoleted */
  __IO uint32_t dummy_266         : 1;
  /* reg<268:267> */
  __IO uint32_t CON_LDO_VCO       : 2; /* BT VCO LDO control: b'01 */
  __IO uint32_t dummy_286_269     : 18;

  /* ADC: reg<288:287> cross word boudary */
  __IO uint32_t SEL_AMP_lo        : 1; /* SAR ADC's gain: b'01
                                        [x1, x2, x4, saturation] */
  /*    <288>   @0x64=word9 */
  __IO uint32_t SEL_AMP_hi        : 1;

  /* reg<289> */
  __IO uint32_t RF_PLL_TEST       : 1; /* RF PLL test: 1=no test */
  __IO uint32_t dummy_293_290     : 4;
  /* reg<294> */
  __IO uint32_t SYS_PLL_TEST_EN   : 1; /* system PLL test enable: b'1-enable; ?b'0=disable */
  /* reg<297:295> */
  __IO uint32_t RF_PLL_LDO_SEL    : 3; /* RF PLL's LDO test select: b'010 (?b'000) */
  /* reg<298> */
  __IO uint32_t RF_PLL_LDO_TEST   : 1; /* RF PLL's LDO test enable: 0-disable */

  /************ 6601's CODEC *****/
  /* reg<299> */
  __IO uint32_t EN_VCM_LDO        : 1; /* enable PA VCM LDO: 0=disable; 1=enable */
  /* reg<300> */
  __IO uint32_t SEL_PA_RES        : 1; /* select inner resistor for stability: 0=no; 1=yes */
  /* reg<301> */
  __IO uint32_t EN_PA_VCM         : 1; /* connect MICBIAS as PA VCM LDO reference: 0=no; 1=yes */
  /* reg<302> */
  __IO uint32_t EN_PA_SINGLE      : 1; /* audio PA output: 0=differential; 1=single-end */
  /* reg<303> */
  __IO uint32_t SEL_PKG           : 1; /* 0=dual path; 1=single path */
  __IO uint32_t dummy_317_304     : 14;
  /* reg<318> */
  __IO uint32_t PD_DRV1_R         : 1; /* power down right PA bias: b'1=down; ?b'0=up */
  /* reg<319> */
  __IO uint32_t PD_DRV1_L         : 1; /* power down left  PA bias: b'1=down; ?b'0=up */
} __attribute__ ((packed)) HS_ANA_REGS_Type;

typedef struct
{
  __IO uint32_t PD_CFG[4];              // 0x00
  __IO uint32_t COMMON_CFG[2];
  __IO uint32_t RC_CALIB_CNS;
  __IO uint32_t RX_FIL_CFG;
  __IO uint32_t MAIN_ST_CFG[2];
  __IO uint32_t ADCOC_CNS;
  __IO uint32_t TRX_COMP_CFG[2];
  __IO uint32_t LDO_CALIB_CNS;
  __IO uint32_t reserve0[2];
  union {
    __IO uint32_t COMMON_PACK[10];      // 0x10 * 4
    HS_ANA_REGS_Type REGS;
  };
  __IO uint32_t FM_CFG[2];
  __IO uint32_t FM_AGC_CFG[3];
  __IO uint32_t reserve1;
  __IO uint32_t RX_AGC_CFG[5];          // 0x20 * 4
  __IO uint32_t RX_AGC_OS_REG;
  __IO uint32_t RX_GAINC_LUT_REG[9];
  __IO uint32_t FILT_GAINC_LUT_REG;
  __IO uint32_t RF_AGC_LUT_REG[4];      // 0x30 * 4
  __IO uint32_t IF_AGC_LUT_REG[6];
  __IO uint32_t FILT_AGC_LUT_REG[2];
  __IO uint32_t reserve2[4];
  __IO uint32_t VCO_AFC_CFG[2];         // 0x40 * 4
  __IO uint32_t AFC_TWD;
  __IO uint32_t reserve3;
  __IO uint32_t VTRACK_CFG;
  __IO uint32_t PEAKDET_CFG;
  __IO uint32_t INTER_FREQ;
  __IO uint32_t COR_REG;
  __IO uint32_t SDM_CFG;
  __IO uint32_t LNA_CLOAD_CFG;
  __IO uint32_t AU_ANA_CFG[2];
  __IO uint32_t reserve4[0x24];
  __IO uint32_t DBG_IDX;                // 0x70 * 4
  __IO uint32_t DBG_RDATA;
  __IO uint32_t reserve5[14];
  __IO uint32_t DCOC_LUT_REG[51];       // 0x80 * 4
}HS_ANA_Type;



/**
 * Peripheral_registers_structures
 * @}
 */

/**
 * @addtogroup Peripheral_memory_map
 * @{
 */

#define SRAM0_BASE            ((uint32_t)0x20000000) /*!< SRAM base address in the alias region */
#define SRAM1_BASE            ((uint32_t)0x20040000) /*!< SRAM base address in the alias region */

#define SRAM_BB_BASE          ((uint32_t)0x12000000) /*!< SRAM base address in the bit-band region */
#define PERIPH_BB_BASE        ((uint32_t)0x42000000) /*!< Peripheral base address in the bit-band region */

/*!< Peripheral memory map */
#define APB1PERIPH_BASE       ((uint32_t)0x40000000)
#define APB2PERIPH_BASE       ((uint32_t)0x40000000)
#define AHBPERIPH_BASE        ((uint32_t)0x41000000)

#define SYS_BASE              ((uint32_t)0x40000000)
#define RTL_TEST_BASE         ((uint32_t)0x40000C0C)
#define CPM_PSO_BASE          ((uint32_t)0x40001000)
#define CPM_PMU_BASE          ((uint32_t)0x40002000)
#define MAC6200_BASE          ((uint32_t)0x40004000)
#define ANA_BASE              ((uint32_t)0x4000F000)
#define BTBB_BASE             ((uint32_t)0x40010000)
#define BTBB_LC_BASE          ((uint32_t)0x40016000)
#define BTBB_RF_BASE          ((uint32_t)0x40018000)
#define BTBB_LE_BASE          ((uint32_t)0x4001A000)
#define BTPHY_BASE            ((uint32_t)0x40020000)
#define ADC_BASE              ((uint32_t)0x4000f400)
#define BTPHY2_BASE           ((uint32_t)0x40020400)
#define UART0_BASE            ((uint32_t)0x40030000)
#define UART1_BASE            ((uint32_t)0x40040000)
#define SPI0_BASE             ((uint32_t)0x40050000)
#define SPI1_BASE             ((uint32_t)0x40060000)
#define I2C0_BASE             ((uint32_t)0x40070000)
#define I2S0_BASE             ((uint32_t)0x47000000)
#define I2S1_BASE             ((uint32_t)0x48000000)
#define CODEC_BASE            ((uint32_t)0x400B0000)
#define TIM0_BASE             ((uint32_t)0x400C0000)
#define TIM1_BASE             ((uint32_t)0x400C0100)
#define TIM2_BASE             ((uint32_t)0x400C0200)
#define WDT_BASE              ((uint32_t)0x400D0000)
#define PMU_BASE              ((uint32_t)0x400E0000)
#define RTC_BASE              ((uint32_t)0x400F0000)

#define DMA_CH0_BASE          ((uint32_t)0x41100000)
#define DMA_CH1_BASE          ((uint32_t)0x41100058)
#define DMA_CH2_BASE          ((uint32_t)0x411000B0)
#define DMA_CH3_BASE          ((uint32_t)0x41100108)
#define DMA_CH4_BASE          ((uint32_t)0x41100160)
#define DMA_CH5_BASE          ((uint32_t)0x411001B8)
#define DMA_CH6_BASE          ((uint32_t)0x41100210)
#define DMA_CH7_BASE          ((uint32_t)0x41100268)
#define DMAC_BASE             ((uint32_t)0x411002C0)
#define SF_BASE               ((uint32_t)0x42800000)
#define GPIO0_BASE            ((uint32_t)0x43000000)
#define GPIO1_BASE            ((uint32_t)0x44000000)
#define OTG_BASE              ((uint32_t)0x45000000)
#define SDHC_BASE             ((uint32_t)0x46000000)


/**
 * Peripheral_memory_map
 * @}
 */

/**
 * @addtogroup Peripheral_declaration
 * @{
 */

#define HS_SYS                ((HS_SYS_Type *) SYS_BASE)
#define HS_PSO                ((HS_PSO_CPM_Type *) CPM_PSO_BASE)
#define HS_RTL_TEST           ((HS_RTL_TEST_Type *)RTL_TEST_BASE)
#define HS_PMU_CPM            ((HS_PMU_CPM_Type *) CPM_PMU_BASE)
#define HS_MAC6200            ((HS_MAC6200_Type *) MAC6200_BASE)
#define HS_BTBB_LC            ((HS_BTBB_LC_Type *) BTBB_LC_BASE)
#define HS_BTBB_RF            ((HS_BTBB_RF_Type *) BTBB_RF_BASE)
#define HS_BTBB_LE            ((HS_BTBB_LE_Type *) BTBB_LE_BASE)
#define HS_BTPHY              ((HS_BTPHY_Type *) BTPHY_BASE)
#define HS_ADC                ((HS_ADC_Type *) ADC_BASE)
#define HS_UART0              ((HS_UART_Type *) UART0_BASE)
#define HS_UART1              ((HS_UART_Type *) UART1_BASE)
#define HS_SPI0               ((HS_SPI_Type *) SPI0_BASE)
#define HS_SPI1               ((HS_SPI_Type *) SPI1_BASE)
#define HS_I2C0               ((HS_I2C_Type *) I2C0_BASE)
#define HS_I2S_TX             ((HS_I2S_Type *) I2S0_BASE)
#define HS_I2S_RX             ((HS_I2S_Type *) I2S1_BASE)
#define HS_CODEC              ((HS_CODEC_Type *) CODEC_BASE)
#define HS_TIM0               ((HS_TIM_Type *) TIM0_BASE)
#define HS_TIM1               ((HS_TIM_Type *) TIM1_BASE)
#define HS_TIM2               ((HS_TIM_Type *) TIM2_BASE)
#define HS_WDT                ((HS_WDT_Type *) WDT_BASE)
#define HS_PMU                ((HS_PMU_Type *) PMU_BASE)
#define HS_RTC                ((HS_RTC_Type *) RTC_BASE)

#define HS_SF                 ((HS_SF_Type *) SF_BASE)
#define HS_GPIO0              ((HS_GPIO_Type *) GPIO0_BASE)
#define HS_GPIO1              ((HS_GPIO_Type *) GPIO1_BASE)
#define HS_SDHC               ((HS_SDHC_Type *) SDHC_BASE)
#define HS_DMA_CH0            ((HS_DMA_CH_Type *) DMA_CH0_BASE)
#define HS_DMA_CH1            ((HS_DMA_CH_Type *) DMA_CH1_BASE)
#define HS_DMA_CH2            ((HS_DMA_CH_Type *) DMA_CH2_BASE)
#define HS_DMA_CH3            ((HS_DMA_CH_Type *) DMA_CH3_BASE)
#define HS_DMA_CH4            ((HS_DMA_CH_Type *) DMA_CH4_BASE)
#define HS_DMA_CH5            ((HS_DMA_CH_Type *) DMA_CH5_BASE)
#define HS_DMA_CH6            ((HS_DMA_CH_Type *) DMA_CH6_BASE)
#define HS_DMA_CH7            ((HS_DMA_CH_Type *) DMA_CH7_BASE)
#define HS_DMAC               ((HS_DMAC_Type *) DMAC_BASE)
#define HS_USB                ((HS_USB_Type*)OTG_BASE)
#define HS_ANA                ((HS_ANA_Type*)ANA_BASE)

/**
 * Peripheral_declaration
 * @}
 */

/**
 * @addtogroup Exported_constants
 * @{
 */

  /**
   * @addtogroup Peripheral_Registers_Bits_Definition
   * @{
   */

/******************************************************************************/
/*                         Peripheral Registers_Bits_Definition               */
/******************************************************************************/

//******************************************************************************/
/*                                                                            */
/*                General Purpose and Alternate Function I/O                  */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for GPIO_INTTYPE register  *******************/
#define GPIO_INTTYPE_LOW_HIGH                 0
#define GPIO_INTTYPE_FALL_RISE                1

/*******************  Bit definition for GPIO_INTPOL register  *******************/
#define GPIO_INTPOL_LOW                       0
#define GPIO_INTPOL_HIGH                      1
#define GPIO_INTPOL_FALL                      0
#define GPIO_INTPOL_RISE                      1


/******************************************************************************/
/*                                                                            */
/*                             Audio CODEC                                    */
/*                                                                            */
/******************************************************************************/

/* Bitfields in IF_CTRL */
#define CODEC_I2S_IN		(0 << 7)	/* I2S to internal CODEC */
#define CODEC_I2S_OUT		(1 << 7)	/* I2S to external CODEC */
#define CODEC_TRANSMIT_EN	(1 << 6)	/* transmit data to I2S/PCM: record */
#define CODEC_RECEIVE_EN	(1 << 5)	/* receive data from I2S/PCM: play */
#define CODEC_IF_I2S		(0 << 4)	/* I2S interface */
#define CODEC_IF_PCM		(1 << 4)	/* PCM interface */
#define CODEC_PCM_INPUT_SEL_L	(0 << 3)	/* input from left track */
#define CODEC_PCM_INPUT_SEL_R	(1 << 3)	/* input from right track */
#define CODEC_PCM_OUTPUT_SEL_L	(0 << 1)	/* output to left track */
#define CODEC_PCM_OUTPUT_SEL_R	(1 << 1)	/* output to right track */
#define CODEC_I2S_INPUT_FST_L	(0 << 2)	/* input first from left track */
#define CODEC_I2S_INPUT_FST_R	(1 << 2)	/* input first from right track */
#define CODEC_I2S_OUTPUT_FST_L	(0 << 0)	/* output first to left track */
#define CODEC_I2S_OUTPUT_FST_R	(1 << 0)	/* output first to right track */

/* Bitfields in ADC_CTRL */
#define CODEC_ADC_SR_32K	(1 << 14)	/* sample rate base = {32k, 44.1k, 48k} */
#define CODEC_ADC_SR_44K	(1 << 13)
#define CODEC_ADC_SR_48K	(1 << 12)
#define CODEC_ADC_SR_DIV4	(1 << 10)	/* sample rate = sample rate base / {4,2,1} */
#define CODEC_ADC_SR_DIV2	(1 << 9)
#define CODEC_ADC_SR_DIV1	(1 << 8)
#define CODEC_ADC_CIC_SCALE_MASK (0x7 << 4)	/* the gain before filter */
#define CODEC_ADC_DC_EN		(1 << 3)	/* 1-DC offset is enable */
#define CODEC_DMIC_EN		(1 << 2)	/* 0-analog MIC, 1-digital MIC */
#define CODEC_ADC_SW_RESETn	(1 << 1)
#define CODEC_ADC_EN		(1 << 0)

/* Bitfields in DAC_CTRL */
#define CODEC_DAC_SR_96K	(1 << 7)	/* sample rate base = {95k, 32k, 44.1k, 48k} */
#define CODEC_DAC_SR_32K	(1 << 6)
#define CODEC_DAC_SR_44K	(1 << 5)
#define CODEC_DAC_SR_48K	(1 << 4)
#define CODEC_DAC_SR_DIV4	(1 << 2)	/* sample rate = sample rate base / {4,2,1} */
#define CODEC_DAC_SR_DIV2	(1 << 1)
#define CODEC_DAC_SR_DIV1	(1 << 0)
#define CODEC_DAC_FIR_CLR	(1 << 10)	/* 0-stop clear, 1-clear FIR SRAM */
#define CODEC_DAC_SW_RESETn	(1 << 9)
#define CODEC_DAC_EN		(1 << 8)

/* Bitfields in DAC_MOD_CTRL */
#define CODEC_DAC_DITHER_TPDF		(0 << 3)	/* use {TPDF,RPDF} dither */
#define CODEC_DAC_DITHER_RPDF		(1 << 3)
#define CODEC_DAC_DITHER_DYNAMIC	(0 << 2)	/* use {dynamic,static} dither */
#define CODEC_DAC_DITHER_STATIC		(1 << 2)
#define CODEC_DAC_DITHER_EN		(1 << 1)	/* 0-disable, 1-enable */
#define CODEC_DAC_DWA_OFF		(1 << 0)	/* 0-on,      1-off */

/* Bitfields in TEST_MODE */
#define CODEC_TEST_FULL_LOOP		(0x8)	/* ADC digitial output to DAC digitial input */
#define CODEC_TEST_DAC_PHY_MODE	(0x4)	/* GPIO as modulator data to analog */
#define CODEC_TEST_A2DLOOP		(0x2)	/* analog loop */
#define CODEC_TEST_DIGI_LOOP		(0x1)	/* digital loopback */

/* Bitfields in CODEC_GAIN_READ */
#define CODEC_DAC_GAIN_R_MASK		(0xF << 24)	/* gain control read out */
#define CODEC_DAC_GAIN_L_MASK		(0xF << 24)
#define CODEC_ADC_GAIN_R_MASK		(0xF << 24)
#define CODEC_ADC_GAIN_L_MASK		(0xF << 24)

/* Bitfields in ADC_DRC_CTRL1, DAC_DRC_CTRL1 */
#define CODEC_DRC_AT1(n)		((n) << 16)	/* DRC attack time */
#define CODEC_DRC_AT0(n)		((n) << 0)

/* Bitfields in ADC_DRC_CTRL2, DAC_DRC_CTRL2 */
#define CODEC_DRC_RT1(n)		((n) << 16)	/* DRC release time */
#define CODEC_DRC_RT0(n)		((n) << 0)

/* Bitfields in ADC_DRC_CTRL3, DAC_DRC_CTRL3 */
#define CODEC_DRC_GAIN_JITTER(n)	((n) << 28)	/* DRC gain hysteresis setting */
#define CODEC_DRC_NT(n)			((n) << 24)	/* DRC noise threhold */
#define CODEC_DRC_ET(n)			((n) << 16)	/* DRC expand threhold */
#define CODEC_DRC_CT(n)			((n) << 8)	/* DRC compress threhold */
#define CODEC_DRC_LT(n)			((n) << 2)	/* DRC limit threhold */
#define CODEC_DRC_MODE_MAX		(0x3 << 0)	/* the max of left and right as reference */
#define CODEC_DRC_MODE_R		(0x2 << 0)	/* right as reference */
#define CODEC_DRC_MODE_L		(0x1 << 0)	/* left as reference */
#define CODEC_DRC_MODE_DIS		(0x0 << 0)	/* DRC is disable */

/* Bitfields in ADC_DRC_CTRL4, DAC_DRC_CTRL4 */
#define CODEC_DRC_GAIN_MIN(n)		((n) << 24)	/* neg limit */
#define CODEC_DRC_GAIN_MAX(n)		((n) << 16)	/* pos limit */
#define CODEC_DRC_GAIN_NO		(0x0 << 10)	/* no gain */
#define CODEC_DRC_GAIN_ADD		(0x1 << 10)	/* add gain */
#define CODEC_DRC_ES_NOINV		(0 << 9)	/* 0-ES no invert, 1-ES invert */
#define CODEC_DRC_ES_INV		(1 << 9)
#define CODEC_DRC_NS_MODE_MUTE		(0 << 8)	/* 0-mute, 1-attenuate */
#define CODEC_DRC_NS_MODE_ATTEN		(1 << 8)
#define CODEC_DRC_DELAY(n)		((n) << 6)	/* 3,2,1,0-delay 12,8,4,0 samples */
#define CODEC_DRC_ES(n)			((n) << 3)	/* expand slop */
#define CODEC_DRC_CS(n)			((n) << 0)	/* compress slop */

/* Bitfields in ADC_VOL_CTRL, DAC_VOL_CTRL */
#define CODEC_VOL_UPDATE		(1 << 24)	/* volume control is updated when 1 */
#define CODEC_VOL_R(n)			((n) << 16)	/* right channle volume, 0.5dB/step */
#define CODEC_VOL_L(n)			((n) << 8)
#define CODEC_MUTE_RATE(n)		((n) << 4)	/* volume adjust speed in mute/unmute process */
#define CODEC_MIX_EN			(1 << 3)	/* left and right is mixed */
#define CODEC_MUTE_BYPASS		(1 << 2)	/* mute/unmute machanism is bypass */
#define CODEC_UNMUTE			(1 << 1)	/* gain gradaully increased from 0 to vol */
#define CODEC_MUTE			(1 << 0)	/* gain gradaully decreased to 0 */

/******************************************************************************/
/*                                                                            */
/*                             DMA Controller                                 */
/*                                                                            */
/******************************************************************************/

/* Bitfields in DW_PARAMS */
#define DW_PARAMS_NR_CHAN	8		/* number of channels */
#define DW_PARAMS_NR_MASTER	11		/* number of AHB masters */
#define DW_PARAMS_DATA_WIDTH(n)	(15 + 2 * (n))
#define DW_PARAMS_DATA_WIDTH1	15		/* master 1 data width */
#define DW_PARAMS_DATA_WIDTH2	17		/* master 2 data width */
#define DW_PARAMS_DATA_WIDTH3	19		/* master 3 data width */
#define DW_PARAMS_DATA_WIDTH4	21		/* master 4 data width */
#define DW_PARAMS_EN		28		/* encoded parameters */

/* Bitfields in DWC_PARAMS */
#define DWC_PARAMS_MBLK_EN	11		/* multi block transfer */
#define DWC_PARAMS_MAX_MULT_SIZE 16

/* Bitfields in CTL_LO */
#define DWC_CTLL_INT_EN		(1 << 0)	/* irqs enabled? */
#define DWC_CTLL_DST_WIDTH(n)	((n)<<1)	/* bytes per element */
#define DWC_CTLL_SRC_WIDTH(n)	((n)<<4)
#define DWC_CTLL_DST_INC	(0<<7)		/* DAR update/not */
#define DWC_CTLL_DST_DEC	(1<<7)
#define DWC_CTLL_DST_FIX	(2<<7)
#define DWC_CTLL_SRC_INC	(0<<7)		/* SAR update/not */
#define DWC_CTLL_SRC_DEC	(1<<9)
#define DWC_CTLL_SRC_FIX	(2<<9)
#define DWC_CTLL_DST_MSIZE(n)	((n)<<11)	/* burst, #elements */
#define DWC_CTLL_SRC_MSIZE(n)	((n)<<14)
#define DWC_CTLL_S_GATH_EN	(1 << 17)	/* src gather, !FIX */
#define DWC_CTLL_D_SCAT_EN	(1 << 18)	/* dst scatter, !FIX */
#define DWC_CTLL_FC(n)		((n) << 20)
#define DWC_CTLL_FC_M2M		(0 << 20)	/* mem-to-mem */
#define DWC_CTLL_FC_M2P		(1 << 20)	/* mem-to-periph */
#define DWC_CTLL_FC_P2M		(2 << 20)	/* periph-to-mem */
#define DWC_CTLL_FC_P2P		(3 << 20)	/* periph-to-periph */
/* plus 4 transfer types for peripheral-as-flow-controller */
#define DWC_CTLL_DMS(n)		((n)<<23)	/* dst master select */
#define DWC_CTLL_SMS(n)		((n)<<25)	/* src master select */
#define DWC_CTLL_LLP_D_EN	(1 << 27)	/* dest block chain */
#define DWC_CTLL_LLP_S_EN	(1 << 28)	/* src block chain */

/* Bitfields in CTL_HI */
#define DWC_CTLH_DONE		0x00001000
#define DWC_CTLH_BLOCK_TS_MASK	0x00000fff

/* Bitfields in CFG_LO. */
#define DWC_CFGL_CH_PRIOR_MASK	(0x7 << 5)	/* priority mask */
#define DWC_CFGL_CH_PRIOR(x)	((x) << 5)	/* priority */
#define DWC_CFGL_CH_SUSP	(1 << 8)	/* pause xfer */
#define DWC_CFGL_FIFO_EMPTY	(1 << 9)	/* pause xfer */
#define DWC_CFGL_HS_DST		(1 << 10)	/* handshake w/dst */
#define DWC_CFGL_HS_SRC		(1 << 11)	/* handshake w/src */
#define DWC_CFGL_MAX_BURST(x)	((x) << 20)
#define DWC_CFGL_RELOAD_SAR	(1 << 30)
#define DWC_CFGL_RELOAD_DAR	(1 << 31)
/* Platform-configurable bits in CFG_LO */
#define DWC_CFGL_LOCK_CH_XFER	(0 << 12)	/* scope of LOCK_CH */
#define DWC_CFGL_LOCK_CH_BLOCK	(1 << 12)
#define DWC_CFGL_LOCK_CH_XACT	(2 << 12)
#define DWC_CFGL_LOCK_BUS_XFER	(0 << 14)	/* scope of LOCK_BUS */
#define DWC_CFGL_LOCK_BUS_BLOCK	(1 << 14)
#define DWC_CFGL_LOCK_BUS_XACT	(2 << 14)
#define DWC_CFGL_LOCK_CH	(1 << 15)	/* channel lockout */
#define DWC_CFGL_LOCK_BUS	(1 << 16)	/* busmaster lockout */
#define DWC_CFGL_HS_DST_POL	(1 << 18)	/* dst handshake active low */
#define DWC_CFGL_HS_SRC_POL	(1 << 19)	/* src handshake active low */

/* Bitfields in CFG_HI. */
#define DWC_CFGH_DS_UPD_EN	(1 << 5)
#define DWC_CFGH_SS_UPD_EN	(1 << 6)
/* Platform-configurable bits in CFG_HI */
#define DWC_CFGH_FCMODE		(1 << 0)
#define DWC_CFGH_FIFO_MODE	(1 << 1)
#define DWC_CFGH_PROTCTL(x)	((x) << 2)
#define DWC_CFGH_SRC_PER(x)	((x) << 7)
#define DWC_CFGH_DST_PER(x)	((x) << 11)

/* Bitfields in SGR */
#define DWC_SGR_SGI(x)		((x) << 0)
#define DWC_SGR_SGC(x)		((x) << 20)

/* Bitfields in DSR */
#define DWC_DSR_DSI(x)		((x) << 0)
#define DWC_DSR_DSC(x)		((x) << 20)

/* Bitfields in CFG */
#define DW_CFG_DMA_EN		(1 << 0)

/******************************************************************************/
/*                                                                            */
/*                                    Timer                                   */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for TIM_CR1 register  ********************/
#define  TIM_CR1_CEN                         ((uint16_t)0x0001)            /*!<Counter enable */
#define  TIM_CR1_UDIS                        ((uint16_t)0x0002)            /*!<Update disable */
#define  TIM_CR1_URS                         ((uint16_t)0x0004)            /*!<Update request source */
#define  TIM_CR1_OPM                         ((uint16_t)0x0008)            /*!<One pulse mode */
#define  TIM_CR1_DIR                         ((uint16_t)0x0010)            /*!<Direction */

#define  TIM_CR1_CMS                         ((uint16_t)0x0060)            /*!<CMS[1:0] bits (Center-aligned mode selection) */
#define  TIM_CR1_CMS_0                       ((uint16_t)0x0020)            /*!<Bit 0 */
#define  TIM_CR1_CMS_1                       ((uint16_t)0x0040)            /*!<Bit 1 */

#define  TIM_CR1_ARPE                        ((uint16_t)0x0080)            /*!<Auto-reload preload enable */

#define  TIM_CR1_CKD                         ((uint16_t)0x0300)            /*!<CKD[1:0] bits (clock division) */
#define  TIM_CR1_CKD_0                       ((uint16_t)0x0100)            /*!<Bit 0 */
#define  TIM_CR1_CKD_1                       ((uint16_t)0x0200)            /*!<Bit 1 */

/*******************  Bit definition for TIM_CR2 register  ********************/
#define  TIM_CR2_CCPC                        ((uint16_t)0x0001)            /*!<Capture/Compare Preloaded Control */
#define  TIM_CR2_CCUS                        ((uint16_t)0x0004)            /*!<Capture/Compare Control Update Selection */
#define  TIM_CR2_CCDS                        ((uint16_t)0x0008)            /*!<Capture/Compare DMA Selection */

#define  TIM_CR2_MMS                         ((uint16_t)0x0070)            /*!<MMS[2:0] bits (Master Mode Selection) */
#define  TIM_CR2_MMS_0                       ((uint16_t)0x0010)            /*!<Bit 0 */
#define  TIM_CR2_MMS_1                       ((uint16_t)0x0020)            /*!<Bit 1 */
#define  TIM_CR2_MMS_2                       ((uint16_t)0x0040)            /*!<Bit 2 */

#define  TIM_CR2_TI1S                        ((uint16_t)0x0080)            /*!<TI1 Selection */
#define  TIM_CR2_OIS1                        ((uint16_t)0x0100)            /*!<Output Idle state 1 (OC1 output) */
#define  TIM_CR2_OIS1N                       ((uint16_t)0x0200)            /*!<Output Idle state 1 (OC1N output) */
#define  TIM_CR2_OIS2                        ((uint16_t)0x0400)            /*!<Output Idle state 2 (OC2 output) */
#define  TIM_CR2_OIS2N                       ((uint16_t)0x0800)            /*!<Output Idle state 2 (OC2N output) */
#define  TIM_CR2_OIS3                        ((uint16_t)0x1000)            /*!<Output Idle state 3 (OC3 output) */
#define  TIM_CR2_OIS3N                       ((uint16_t)0x2000)            /*!<Output Idle state 3 (OC3N output) */
#define  TIM_CR2_OIS4                        ((uint16_t)0x4000)            /*!<Output Idle state 4 (OC4 output) */

/*******************  Bit definition for TIM_SMCR register  *******************/
#define  TIM_SMCR_SMS                        ((uint16_t)0x0007)            /*!<SMS[2:0] bits (Slave mode selection) */
#define  TIM_SMCR_SMS_0                      ((uint16_t)0x0001)            /*!<Bit 0 */
#define  TIM_SMCR_SMS_1                      ((uint16_t)0x0002)            /*!<Bit 1 */
#define  TIM_SMCR_SMS_2                      ((uint16_t)0x0004)            /*!<Bit 2 */
#define _TIM_SMCR_SMS(n) ((((n) & 7) << 0) | (((n) >> 3) << 16))

#define  TIM_SMCR_TS                         ((uint16_t)0x0070)            /*!<TS[2:0] bits (Trigger selection) */
#define  TIM_SMCR_TS_0                       ((uint16_t)0x0010)            /*!<Bit 0 */
#define  TIM_SMCR_TS_1                       ((uint16_t)0x0020)            /*!<Bit 1 */
#define  TIM_SMCR_TS_2                       ((uint16_t)0x0040)            /*!<Bit 2 */
#define _TIM_SMCR_TS(n)                ((n) << 4)

#define  TIM_SMCR_MSM                        ((uint16_t)0x0080)            /*!<Master/slave mode */

#define  TIM_SMCR_ETF                        ((uint16_t)0x0F00)            /*!<ETF[3:0] bits (External trigger filter) */
#define  TIM_SMCR_ETF_0                      ((uint16_t)0x0100)            /*!<Bit 0 */
#define  TIM_SMCR_ETF_1                      ((uint16_t)0x0200)            /*!<Bit 1 */
#define  TIM_SMCR_ETF_2                      ((uint16_t)0x0400)            /*!<Bit 2 */
#define  TIM_SMCR_ETF_3                      ((uint16_t)0x0800)            /*!<Bit 3 */

#define  TIM_SMCR_ETPS                       ((uint16_t)0x3000)            /*!<ETPS[1:0] bits (External trigger prescaler) */
#define  TIM_SMCR_ETPS_0                     ((uint16_t)0x1000)            /*!<Bit 0 */
#define  TIM_SMCR_ETPS_1                     ((uint16_t)0x2000)            /*!<Bit 1 */

#define  TIM_SMCR_ECE                        ((uint16_t)0x4000)            /*!<External clock enable */
#define  TIM_SMCR_ETP                        ((uint16_t)0x8000)            /*!<External trigger polarity */

/*******************  Bit definition for TIM_DIER register  *******************/
#define  TIM_DIER_UIE                        ((uint16_t)0x0001)            /*!<Update interrupt enable */
#define  TIM_DIER_CC1IE                      ((uint16_t)0x0002)            /*!<Capture/Compare 1 interrupt enable */
#define  TIM_DIER_CC2IE                      ((uint16_t)0x0004)            /*!<Capture/Compare 2 interrupt enable */
#define  TIM_DIER_CC3IE                      ((uint16_t)0x0008)            /*!<Capture/Compare 3 interrupt enable */
#define  TIM_DIER_CC4IE                      ((uint16_t)0x0010)            /*!<Capture/Compare 4 interrupt enable */
#define  TIM_DIER_COMIE                      ((uint16_t)0x0020)            /*!<COM interrupt enable */
#define  TIM_DIER_TIE                        ((uint16_t)0x0040)            /*!<Trigger interrupt enable */
#define  TIM_DIER_BIE                        ((uint16_t)0x0080)            /*!<Break interrupt enable */
#define  TIM_DIER_UDE                        ((uint16_t)0x0100)            /*!<Update DMA request enable */
#define  TIM_DIER_CC1DE                      ((uint16_t)0x0200)            /*!<Capture/Compare 1 DMA request enable */
#define  TIM_DIER_CC2DE                      ((uint16_t)0x0400)            /*!<Capture/Compare 2 DMA request enable */
#define  TIM_DIER_CC3DE                      ((uint16_t)0x0800)            /*!<Capture/Compare 3 DMA request enable */
#define  TIM_DIER_CC4DE                      ((uint16_t)0x1000)            /*!<Capture/Compare 4 DMA request enable */
#define  TIM_DIER_COMDE                      ((uint16_t)0x2000)            /*!<COM DMA request enable */
#define  TIM_DIER_TDE                        ((uint16_t)0x4000)            /*!<Trigger DMA request enable */

#define _TIM_DIER_IRQ_MASK                  (TIM_DIER_UIE   |         \
                                             TIM_DIER_CC1IE |         \
                                             TIM_DIER_CC2IE |         \
                                             TIM_DIER_CC3IE |         \
                                             TIM_DIER_CC4IE |         \
                                             TIM_DIER_COMIE |         \
                                             TIM_DIER_TIE   |         \
                                             TIM_DIER_BIE)

/********************  Bit definition for TIM_SR register  ********************/
#define  TIM_SR_UIF                          ((uint16_t)0x0001)            /*!<Update interrupt Flag */
#define  TIM_SR_CC1IF                        ((uint16_t)0x0002)            /*!<Capture/Compare 1 interrupt Flag */
#define  TIM_SR_CC2IF                        ((uint16_t)0x0004)            /*!<Capture/Compare 2 interrupt Flag */
#define  TIM_SR_CC3IF                        ((uint16_t)0x0008)            /*!<Capture/Compare 3 interrupt Flag */
#define  TIM_SR_CC4IF                        ((uint16_t)0x0010)            /*!<Capture/Compare 4 interrupt Flag */
#define  TIM_SR_COMIF                        ((uint16_t)0x0020)            /*!<COM interrupt Flag */
#define  TIM_SR_TIF                          ((uint16_t)0x0040)            /*!<Trigger interrupt Flag */
#define  TIM_SR_BIF                          ((uint16_t)0x0080)            /*!<Break interrupt Flag */
#define  TIM_SR_CC1OF                        ((uint16_t)0x0200)            /*!<Capture/Compare 1 Overcapture Flag */
#define  TIM_SR_CC2OF                        ((uint16_t)0x0400)            /*!<Capture/Compare 2 Overcapture Flag */
#define  TIM_SR_CC3OF                        ((uint16_t)0x0800)            /*!<Capture/Compare 3 Overcapture Flag */
#define  TIM_SR_CC4OF                        ((uint16_t)0x1000)            /*!<Capture/Compare 4 Overcapture Flag */

/*******************  Bit definition for TIM_EGR register  ********************/
#define  TIM_EGR_UG                          ((uint8_t)0x01)               /*!<Update Generation */
#define  TIM_EGR_CC1G                        ((uint8_t)0x02)               /*!<Capture/Compare 1 Generation */
#define  TIM_EGR_CC2G                        ((uint8_t)0x04)               /*!<Capture/Compare 2 Generation */
#define  TIM_EGR_CC3G                        ((uint8_t)0x08)               /*!<Capture/Compare 3 Generation */
#define  TIM_EGR_CC4G                        ((uint8_t)0x10)               /*!<Capture/Compare 4 Generation */
#define  TIM_EGR_COMG                        ((uint8_t)0x20)               /*!<Capture/Compare Control Update Generation */
#define  TIM_EGR_TG                          ((uint8_t)0x40)               /*!<Trigger Generation */
#define  TIM_EGR_BG                          ((uint8_t)0x80)               /*!<Break Generation */

/******************  Bit definition for TIM_CCMR1 register  *******************/
#define  TIM_CCMR1_CC1S                      ((uint16_t)0x0003)            /*!<CC1S[1:0] bits (Capture/Compare 1 Selection) */
#define  TIM_CCMR1_CC1S_0                    ((uint16_t)0x0001)            /*!<Bit 0 */
#define  TIM_CCMR1_CC1S_1                    ((uint16_t)0x0002)            /*!<Bit 1 */
#define _TIM_CCMR1_CC1S(n) ((n) << 0)

#define  TIM_CCMR1_OC1FE                     ((uint16_t)0x0004)            /*!<Output Compare 1 Fast enable */
#define  TIM_CCMR1_OC1PE                     ((uint16_t)0x0008)            /*!<Output Compare 1 Preload enable */

#define  TIM_CCMR1_OC1M                      ((uint16_t)0x0070)            /*!<OC1M[2:0] bits (Output Compare 1 Mode) */
#define  TIM_CCMR1_OC1M_0                    ((uint16_t)0x0010)            /*!<Bit 0 */
#define  TIM_CCMR1_OC1M_1                    ((uint16_t)0x0020)            /*!<Bit 1 */
#define  TIM_CCMR1_OC1M_2                    ((uint16_t)0x0040)            /*!<Bit 2 */
#define _TIM_CCMR1_OC1M(n) (((n) & 7) << 4) //| (((n) >> 2) << 16))

#define  TIM_CCMR1_OC1CE                     ((uint16_t)0x0080)            /*!<Output Compare 1Clear Enable */

#define  TIM_CCMR1_CC2S                      ((uint16_t)0x0300)            /*!<CC2S[1:0] bits (Capture/Compare 2 Selection) */
#define  TIM_CCMR1_CC2S_0                    ((uint16_t)0x0100)            /*!<Bit 0 */
#define  TIM_CCMR1_CC2S_1                    ((uint16_t)0x0200)            /*!<Bit 1 */
#define _TIM_CCMR1_CC2S(n) ((n) << 8)

#define  TIM_CCMR1_OC2FE                     ((uint16_t)0x0400)            /*!<Output Compare 2 Fast enable */
#define  TIM_CCMR1_OC2PE                     ((uint16_t)0x0800)            /*!<Output Compare 2 Preload enable */

#define  TIM_CCMR1_OC2M                      ((uint16_t)0x7000)            /*!<OC2M[2:0] bits (Output Compare 2 Mode) */
#define  TIM_CCMR1_OC2M_0                    ((uint16_t)0x1000)            /*!<Bit 0 */
#define  TIM_CCMR1_OC2M_1                    ((uint16_t)0x2000)            /*!<Bit 1 */
#define  TIM_CCMR1_OC2M_2                    ((uint16_t)0x4000)            /*!<Bit 2 */
#define _TIM_CCMR1_OC2M(n) (((n) & 7) << 12) //| (((n) >> 2) << 24))

#define  TIM_CCMR1_OC2CE                     ((uint16_t)0x8000)            /*!<Output Compare 2 Clear Enable */

/*----------------------------------------------------------------------------*/

#define  TIM_CCMR1_IC1PSC                    ((uint16_t)0x000C)            /*!<IC1PSC[1:0] bits (Input Capture 1 Prescaler) */
#define  TIM_CCMR1_IC1PSC_0                  ((uint16_t)0x0004)            /*!<Bit 0 */
#define  TIM_CCMR1_IC1PSC_1                  ((uint16_t)0x0008)            /*!<Bit 1 */

#define  TIM_CCMR1_IC1F                      ((uint16_t)0x00F0)            /*!<IC1F[3:0] bits (Input Capture 1 Filter) */
#define  TIM_CCMR1_IC1F_0                    ((uint16_t)0x0010)            /*!<Bit 0 */
#define  TIM_CCMR1_IC1F_1                    ((uint16_t)0x0020)            /*!<Bit 1 */
#define  TIM_CCMR1_IC1F_2                    ((uint16_t)0x0040)            /*!<Bit 2 */
#define  TIM_CCMR1_IC1F_3                    ((uint16_t)0x0080)            /*!<Bit 3 */

#define  TIM_CCMR1_IC2PSC                    ((uint16_t)0x0C00)            /*!<IC2PSC[1:0] bits (Input Capture 2 Prescaler) */
#define  TIM_CCMR1_IC2PSC_0                  ((uint16_t)0x0400)            /*!<Bit 0 */
#define  TIM_CCMR1_IC2PSC_1                  ((uint16_t)0x0800)            /*!<Bit 1 */

#define  TIM_CCMR1_IC2F                      ((uint16_t)0xF000)            /*!<IC2F[3:0] bits (Input Capture 2 Filter) */
#define  TIM_CCMR1_IC2F_0                    ((uint16_t)0x1000)            /*!<Bit 0 */
#define  TIM_CCMR1_IC2F_1                    ((uint16_t)0x2000)            /*!<Bit 1 */
#define  TIM_CCMR1_IC2F_2                    ((uint16_t)0x4000)            /*!<Bit 2 */
#define  TIM_CCMR1_IC2F_3                    ((uint16_t)0x8000)            /*!<Bit 3 */

/******************  Bit definition for TIM_CCMR2 register  *******************/
#define  TIM_CCMR2_CC3S                      ((uint16_t)0x0003)            /*!<CC3S[1:0] bits (Capture/Compare 3 Selection) */
#define  TIM_CCMR2_CC3S_0                    ((uint16_t)0x0001)            /*!<Bit 0 */
#define  TIM_CCMR2_CC3S_1                    ((uint16_t)0x0002)            /*!<Bit 1 */
#define _TIM_CCMR2_CC3S(n)             ((n) << 0)

#define  TIM_CCMR2_OC3FE                     ((uint16_t)0x0004)            /*!<Output Compare 3 Fast enable */
#define  TIM_CCMR2_OC3PE                     ((uint16_t)0x0008)            /*!<Output Compare 3 Preload enable */

#define  TIM_CCMR2_OC3M                      ((uint16_t)0x0070)            /*!<OC3M[2:0] bits (Output Compare 3 Mode) */
#define  TIM_CCMR2_OC3M_0                    ((uint16_t)0x0010)            /*!<Bit 0 */
#define  TIM_CCMR2_OC3M_1                    ((uint16_t)0x0020)            /*!<Bit 1 */
#define  TIM_CCMR2_OC3M_2                    ((uint16_t)0x0040)            /*!<Bit 2 */
#define _TIM_CCMR2_OC3M(n) (((n) & 7) << 4) //| (((n) >> 2) << 16))

#define  TIM_CCMR2_OC3CE                     ((uint16_t)0x0080)            /*!<Output Compare 3 Clear Enable */

#define  TIM_CCMR2_CC4S                      ((uint16_t)0x0300)            /*!<CC4S[1:0] bits (Capture/Compare 4 Selection) */
#define  TIM_CCMR2_CC4S_0                    ((uint16_t)0x0100)            /*!<Bit 0 */
#define  TIM_CCMR2_CC4S_1                    ((uint16_t)0x0200)            /*!<Bit 1 */
#define _TIM_CCMR2_CC4S(n)             ((n) << 8)

#define  TIM_CCMR2_OC4FE                     ((uint16_t)0x0400)            /*!<Output Compare 4 Fast enable */
#define  TIM_CCMR2_OC4PE                     ((uint16_t)0x0800)            /*!<Output Compare 4 Preload enable */

#define  TIM_CCMR2_OC4M                      ((uint16_t)0x7000)            /*!<OC4M[2:0] bits (Output Compare 4 Mode) */
#define  TIM_CCMR2_OC4M_0                    ((uint16_t)0x1000)            /*!<Bit 0 */
#define  TIM_CCMR2_OC4M_1                    ((uint16_t)0x2000)            /*!<Bit 1 */
#define  TIM_CCMR2_OC4M_2                    ((uint16_t)0x4000)            /*!<Bit 2 */
#define _TIM_CCMR2_OC4M(n) (((n) & 7) << 12) //| (((n) >> 2) << 24))

#define  TIM_CCMR2_OC4CE                     ((uint16_t)0x8000)            /*!<Output Compare 4 Clear Enable */

/*----------------------------------------------------------------------------*/

#define  TIM_CCMR2_IC3PSC                    ((uint16_t)0x000C)            /*!<IC3PSC[1:0] bits (Input Capture 3 Prescaler) */
#define  TIM_CCMR2_IC3PSC_0                  ((uint16_t)0x0004)            /*!<Bit 0 */
#define  TIM_CCMR2_IC3PSC_1                  ((uint16_t)0x0008)            /*!<Bit 1 */

#define  TIM_CCMR2_IC3F                      ((uint16_t)0x00F0)            /*!<IC3F[3:0] bits (Input Capture 3 Filter) */
#define  TIM_CCMR2_IC3F_0                    ((uint16_t)0x0010)            /*!<Bit 0 */
#define  TIM_CCMR2_IC3F_1                    ((uint16_t)0x0020)            /*!<Bit 1 */
#define  TIM_CCMR2_IC3F_2                    ((uint16_t)0x0040)            /*!<Bit 2 */
#define  TIM_CCMR2_IC3F_3                    ((uint16_t)0x0080)            /*!<Bit 3 */

#define  TIM_CCMR2_IC4PSC                    ((uint16_t)0x0C00)            /*!<IC4PSC[1:0] bits (Input Capture 4 Prescaler) */
#define  TIM_CCMR2_IC4PSC_0                  ((uint16_t)0x0400)            /*!<Bit 0 */
#define  TIM_CCMR2_IC4PSC_1                  ((uint16_t)0x0800)            /*!<Bit 1 */

#define  TIM_CCMR2_IC4F                      ((uint16_t)0xF000)            /*!<IC4F[3:0] bits (Input Capture 4 Filter) */
#define  TIM_CCMR2_IC4F_0                    ((uint16_t)0x1000)            /*!<Bit 0 */
#define  TIM_CCMR2_IC4F_1                    ((uint16_t)0x2000)            /*!<Bit 1 */
#define  TIM_CCMR2_IC4F_2                    ((uint16_t)0x4000)            /*!<Bit 2 */
#define  TIM_CCMR2_IC4F_3                    ((uint16_t)0x8000)            /*!<Bit 3 */

/*******************  Bit definition for TIM_CCER register  *******************/
#define  TIM_CCER_CC1E                       ((uint16_t)0x0001)            /*!<Capture/Compare 1 output enable */
#define  TIM_CCER_CC1P                       ((uint16_t)0x0002)            /*!<Capture/Compare 1 output Polarity */
#define  TIM_CCER_CC1NE                      ((uint16_t)0x0004)            /*!<Capture/Compare 1 Complementary output enable */
#define  TIM_CCER_CC1NP                      ((uint16_t)0x0008)            /*!<Capture/Compare 1 Complementary output Polarity */
#define  TIM_CCER_CC2E                       ((uint16_t)0x0010)            /*!<Capture/Compare 2 output enable */
#define  TIM_CCER_CC2P                       ((uint16_t)0x0020)            /*!<Capture/Compare 2 output Polarity */
#define  TIM_CCER_CC2NE                      ((uint16_t)0x0040)            /*!<Capture/Compare 2 Complementary output enable */
#define  TIM_CCER_CC2NP                      ((uint16_t)0x0080)            /*!<Capture/Compare 2 Complementary output Polarity */
#define  TIM_CCER_CC3E                       ((uint16_t)0x0100)            /*!<Capture/Compare 3 output enable */
#define  TIM_CCER_CC3P                       ((uint16_t)0x0200)            /*!<Capture/Compare 3 output Polarity */
#define  TIM_CCER_CC3NE                      ((uint16_t)0x0400)            /*!<Capture/Compare 3 Complementary output enable */
#define  TIM_CCER_CC3NP                      ((uint16_t)0x0800)            /*!<Capture/Compare 3 Complementary output Polarity */
#define  TIM_CCER_CC4E                       ((uint16_t)0x1000)            /*!<Capture/Compare 4 output enable */
#define  TIM_CCER_CC4P                       ((uint16_t)0x2000)            /*!<Capture/Compare 4 output Polarity */
#define  TIM_CCER_CC4NP                      ((uint16_t)0x8000)            /*!<Capture/Compare 4 Complementary output Polarity */

/*******************  Bit definition for TIM_CNT register  ********************/
#define  TIM_CNT_CNT                         ((uint16_t)0xFFFF)            /*!<Counter Value */

/*******************  Bit definition for TIM_PSC register  ********************/
#define  TIM_PSC_PSC                         ((uint16_t)0xFFFF)            /*!<Prescaler Value */

/*******************  Bit definition for TIM_ARR register  ********************/
#define  TIM_ARR_ARR                         ((uint16_t)0xFFFF)            /*!<actual auto-reload Value */

/*******************  Bit definition for TIM_RCR register  ********************/
#define  TIM_RCR_REP                         ((uint8_t)0xFF)               /*!<Repetition Counter Value */

/*******************  Bit definition for TIM_CCR1 register  *******************/
#define  TIM_CCR1_CCR1                       ((uint16_t)0xFFFF)            /*!<Capture/Compare 1 Value */
/*----------------------------------------------------------------------------*/

#define  TIM_CCMR1_IC1PSC                    ((uint16_t)0x000C)            /*!<IC1PSC[1:0] bits (Input Capture 1 Prescaler) */
#define  TIM_CCMR1_IC1PSC_0                  ((uint16_t)0x0004)            /*!<Bit 0 */
#define  TIM_CCMR1_IC1PSC_1                  ((uint16_t)0x0008)            /*!<Bit 1 */

#define  TIM_CCMR1_IC1F                      ((uint16_t)0x00F0)            /*!<IC1F[3:0] bits (Input Capture 1 Filter) */
#define  TIM_CCMR1_IC1F_0                    ((uint16_t)0x0010)            /*!<Bit 0 */
#define  TIM_CCMR1_IC1F_1                    ((uint16_t)0x0020)            /*!<Bit 1 */
#define  TIM_CCMR1_IC1F_2                    ((uint16_t)0x0040)            /*!<Bit 2 */
#define  TIM_CCMR1_IC1F_3                    ((uint16_t)0x0080)            /*!<Bit 3 */

#define  TIM_CCMR1_IC2PSC                    ((uint16_t)0x0C00)            /*!<IC2PSC[1:0] bits (Input Capture 2 Prescaler) */
#define  TIM_CCMR1_IC2PSC_0                  ((uint16_t)0x0400)            /*!<Bit 0 */
#define  TIM_CCMR1_IC2PSC_1                  ((uint16_t)0x0800)            /*!<Bit 1 */

#define  TIM_CCMR1_IC2F                      ((uint16_t)0xF000)            /*!<IC2F[3:0] bits (Input Capture 2 Filter) */
#define  TIM_CCMR1_IC2F_0                    ((uint16_t)0x1000)            /*!<Bit 0 */
#define  TIM_CCMR1_IC2F_1                    ((uint16_t)0x2000)            /*!<Bit 1 */
#define  TIM_CCMR1_IC2F_2                    ((uint16_t)0x4000)            /*!<Bit 2 */
#define  TIM_CCMR1_IC2F_3                    ((uint16_t)0x8000)            /*!<Bit 3 */

/******************  Bit definition for TIM_CCMR2 register  *******************/
#define  TIM_CCMR2_CC3S                      ((uint16_t)0x0003)            /*!<CC3S[1:0] bits (Capture/Compare 3 Selection) */
#define  TIM_CCMR2_CC3S_0                    ((uint16_t)0x0001)            /*!<Bit 0 */
#define  TIM_CCMR2_CC3S_1                    ((uint16_t)0x0002)            /*!<Bit 1 */

#define  TIM_CCMR2_OC3FE                     ((uint16_t)0x0004)            /*!<Output Compare 3 Fast enable */
#define  TIM_CCMR2_OC3PE                     ((uint16_t)0x0008)            /*!<Output Compare 3 Preload enable */

#define  TIM_CCMR2_OC3M                      ((uint16_t)0x0070)            /*!<OC3M[2:0] bits (Output Compare 3 Mode) */
#define  TIM_CCMR2_OC3M_0                    ((uint16_t)0x0010)            /*!<Bit 0 */
#define  TIM_CCMR2_OC3M_1                    ((uint16_t)0x0020)            /*!<Bit 1 */
#define  TIM_CCMR2_OC3M_2                    ((uint16_t)0x0040)            /*!<Bit 2 */

#define  TIM_CCMR2_OC3CE                     ((uint16_t)0x0080)            /*!<Output Compare 3 Clear Enable */

#define  TIM_CCMR2_CC4S                      ((uint16_t)0x0300)            /*!<CC4S[1:0] bits (Capture/Compare 4 Selection) */
#define  TIM_CCMR2_CC4S_0                    ((uint16_t)0x0100)            /*!<Bit 0 */
#define  TIM_CCMR2_CC4S_1                    ((uint16_t)0x0200)            /*!<Bit 1 */

#define  TIM_CCMR2_OC4FE                     ((uint16_t)0x0400)            /*!<Output Compare 4 Fast enable */
#define  TIM_CCMR2_OC4PE                     ((uint16_t)0x0800)            /*!<Output Compare 4 Preload enable */

#define  TIM_CCMR2_OC4M                      ((uint16_t)0x7000)            /*!<OC4M[2:0] bits (Output Compare 4 Mode) */
#define  TIM_CCMR2_OC4M_0                    ((uint16_t)0x1000)            /*!<Bit 0 */
#define  TIM_CCMR2_OC4M_1                    ((uint16_t)0x2000)            /*!<Bit 1 */
#define  TIM_CCMR2_OC4M_2                    ((uint16_t)0x4000)            /*!<Bit 2 */

#define  TIM_CCMR2_OC4CE                     ((uint16_t)0x8000)            /*!<Output Compare 4 Clear Enable */

/*----------------------------------------------------------------------------*/

#define  TIM_CCMR2_IC3PSC                    ((uint16_t)0x000C)            /*!<IC3PSC[1:0] bits (Input Capture 3 Prescaler) */
#define  TIM_CCMR2_IC3PSC_0                  ((uint16_t)0x0004)            /*!<Bit 0 */
#define  TIM_CCMR2_IC3PSC_1                  ((uint16_t)0x0008)            /*!<Bit 1 */

#define  TIM_CCMR2_IC3F                      ((uint16_t)0x00F0)            /*!<IC3F[3:0] bits (Input Capture 3 Filter) */
#define  TIM_CCMR2_IC3F_0                    ((uint16_t)0x0010)            /*!<Bit 0 */
#define  TIM_CCMR2_IC3F_1                    ((uint16_t)0x0020)            /*!<Bit 1 */
#define  TIM_CCMR2_IC3F_2                    ((uint16_t)0x0040)            /*!<Bit 2 */
#define  TIM_CCMR2_IC3F_3                    ((uint16_t)0x0080)            /*!<Bit 3 */

#define  TIM_CCMR2_IC4PSC                    ((uint16_t)0x0C00)            /*!<IC4PSC[1:0] bits (Input Capture 4 Prescaler) */
#define  TIM_CCMR2_IC4PSC_0                  ((uint16_t)0x0400)            /*!<Bit 0 */
#define  TIM_CCMR2_IC4PSC_1                  ((uint16_t)0x0800)            /*!<Bit 1 */

#define  TIM_CCMR2_IC4F                      ((uint16_t)0xF000)            /*!<IC4F[3:0] bits (Input Capture 4 Filter) */
#define  TIM_CCMR2_IC4F_0                    ((uint16_t)0x1000)            /*!<Bit 0 */
#define  TIM_CCMR2_IC4F_1                    ((uint16_t)0x2000)            /*!<Bit 1 */
#define  TIM_CCMR2_IC4F_2                    ((uint16_t)0x4000)            /*!<Bit 2 */
#define  TIM_CCMR2_IC4F_3                    ((uint16_t)0x8000)            /*!<Bit 3 */

/*******************  Bit definition for TIM_CCER register  *******************/
#define  TIM_CCER_CC1E                       ((uint16_t)0x0001)            /*!<Capture/Compare 1 output enable */
#define  TIM_CCER_CC1P                       ((uint16_t)0x0002)            /*!<Capture/Compare 1 output Polarity */
#define  TIM_CCER_CC1NE                      ((uint16_t)0x0004)            /*!<Capture/Compare 1 Complementary output enable */
#define  TIM_CCER_CC1NP                      ((uint16_t)0x0008)            /*!<Capture/Compare 1 Complementary output Polarity */
#define  TIM_CCER_CC2E                       ((uint16_t)0x0010)            /*!<Capture/Compare 2 output enable */
#define  TIM_CCER_CC2P                       ((uint16_t)0x0020)            /*!<Capture/Compare 2 output Polarity */
#define  TIM_CCER_CC2NE                      ((uint16_t)0x0040)            /*!<Capture/Compare 2 Complementary output enable */
#define  TIM_CCER_CC2NP                      ((uint16_t)0x0080)            /*!<Capture/Compare 2 Complementary output Polarity */
#define  TIM_CCER_CC3E                       ((uint16_t)0x0100)            /*!<Capture/Compare 3 output enable */
#define  TIM_CCER_CC3P                       ((uint16_t)0x0200)            /*!<Capture/Compare 3 output Polarity */
#define  TIM_CCER_CC3NE                      ((uint16_t)0x0400)            /*!<Capture/Compare 3 Complementary output enable */
#define  TIM_CCER_CC3NP                      ((uint16_t)0x0800)            /*!<Capture/Compare 3 Complementary output Polarity */
#define  TIM_CCER_CC4E                       ((uint16_t)0x1000)            /*!<Capture/Compare 4 output enable */
#define  TIM_CCER_CC4P                       ((uint16_t)0x2000)            /*!<Capture/Compare 4 output Polarity */
#define  TIM_CCER_CC4NP                      ((uint16_t)0x8000)            /*!<Capture/Compare 4 Complementary output Polarity */

/*******************  Bit definition for TIM_CNT register  ********************/
#define  TIM_CNT_CNT                         ((uint16_t)0xFFFF)            /*!<Counter Value */

/*******************  Bit definition for TIM_PSC register  ********************/
#define  TIM_PSC_PSC                         ((uint16_t)0xFFFF)            /*!<Prescaler Value */

/*******************  Bit definition for TIM_ARR register  ********************/
#define  TIM_ARR_ARR                         ((uint16_t)0xFFFF)            /*!<actual auto-reload Value */

/*******************  Bit definition for TIM_RCR register  ********************/
#define  TIM_RCR_REP                         ((uint8_t)0xFF)               /*!<Repetition Counter Value */

/*******************  Bit definition for TIM_CCR1 register  *******************/
#define  TIM_CCR1_CCR1                       ((uint16_t)0xFFFF)            /*!<Capture/Compare 1 Value */

/*******************  Bit definition for TIM_CCR2 register  *******************/
#define  TIM_CCR2_CCR2                       ((uint16_t)0xFFFF)            /*!<Capture/Compare 2 Value */

/*******************  Bit definition for TIM_CCR3 register  *******************/
#define  TIM_CCR3_CCR3                       ((uint16_t)0xFFFF)            /*!<Capture/Compare 3 Value */

/*******************  Bit definition for TIM_CCR4 register  *******************/
#define  TIM_CCR4_CCR4                       ((uint16_t)0xFFFF)            /*!<Capture/Compare 4 Value */

/*******************  Bit definition for TIM_BDTR register  *******************/
#define  TIM_BDTR_DTG                        ((uint16_t)0x00FF)            /*!<DTG[0:7] bits (Dead-Time Generator set-up) */
#define  TIM_BDTR_DTG_0                      ((uint16_t)0x0001)            /*!<Bit 0 */
#define  TIM_BDTR_DTG_1                      ((uint16_t)0x0002)            /*!<Bit 1 */
#define  TIM_BDTR_DTG_2                      ((uint16_t)0x0004)            /*!<Bit 2 */
#define  TIM_BDTR_DTG_3                      ((uint16_t)0x0008)            /*!<Bit 3 */
#define  TIM_BDTR_DTG_4                      ((uint16_t)0x0010)            /*!<Bit 4 */
#define  TIM_BDTR_DTG_5                      ((uint16_t)0x0020)            /*!<Bit 5 */
#define  TIM_BDTR_DTG_6                      ((uint16_t)0x0040)            /*!<Bit 6 */
#define  TIM_BDTR_DTG_7                      ((uint16_t)0x0080)            /*!<Bit 7 */

#define  TIM_BDTR_LOCK                       ((uint16_t)0x0300)            /*!<LOCK[1:0] bits (Lock Configuration) */
#define  TIM_BDTR_LOCK_0                     ((uint16_t)0x0100)            /*!<Bit 0 */
#define  TIM_BDTR_LOCK_1                     ((uint16_t)0x0200)            /*!<Bit 1 */

#define  TIM_BDTR_OSSI                       ((uint16_t)0x0400)            /*!<Off-State Selection for Idle mode */
#define  TIM_BDTR_OSSR                       ((uint16_t)0x0800)            /*!<Off-State Selection for Run mode */
#define  TIM_BDTR_BKE                        ((uint16_t)0x1000)            /*!<Break enable */
#define  TIM_BDTR_BKP                        ((uint16_t)0x2000)            /*!<Break Polarity */
#define  TIM_BDTR_AOE                        ((uint16_t)0x4000)            /*!<Automatic Output enable */
#define  TIM_BDTR_MOE                        ((uint16_t)0x8000)            /*!<Main Output enable */

/*******************  Bit definition for TIM_DCR register  ********************/
#define  TIM_DCR_DBA                         ((uint16_t)0x001F)            /*!<DBA[4:0] bits (DMA Base Address) */
#define  TIM_DCR_DBA_0                       ((uint16_t)0x0001)            /*!<Bit 0 */
#define  TIM_DCR_DBA_1                       ((uint16_t)0x0002)            /*!<Bit 1 */
#define  TIM_DCR_DBA_2                       ((uint16_t)0x0004)            /*!<Bit 2 */
#define  TIM_DCR_DBA_3                       ((uint16_t)0x0008)            /*!<Bit 3 */
#define  TIM_DCR_DBA_4                       ((uint16_t)0x0010)            /*!<Bit 4 */

#define  TIM_DCR_DBL                         ((uint16_t)0x1F00)            /*!<DBL[4:0] bits (DMA Burst Length) */
#define  TIM_DCR_DBL_0                       ((uint16_t)0x0100)            /*!<Bit 0 */
#define  TIM_DCR_DBL_1                       ((uint16_t)0x0200)            /*!<Bit 1 */
#define  TIM_DCR_DBL_2                       ((uint16_t)0x0400)            /*!<Bit 2 */
#define  TIM_DCR_DBL_3                       ((uint16_t)0x0800)            /*!<Bit 3 */
#define  TIM_DCR_DBL_4                       ((uint16_t)0x1000)            /*!<Bit 4 */

/*******************  Bit definition for TIM_DMAR register  *******************/
#define  TIM_DMAR_DMAB                       ((uint16_t)0xFFFF)            /*!<DMA register for burst accesses */

/*******************  Bit definition for TIM_OR register  *********************/
#define TIM_OR_TI4_RMP                       ((uint16_t)0x00C0)            /*!<TI4_RMP[1:0] bits (TIM5 Input 4 remap) */
#define TIM_OR_TI4_RMP_0                     ((uint16_t)0x0040)            /*!<Bit 0 */
#define TIM_OR_TI4_RMP_1                     ((uint16_t)0x0080)            /*!<Bit 1 */
#define TIM_OR_ITR1_RMP                      ((uint16_t)0x0C00)            /*!<ITR1_RMP[1:0] bits (TIM2 Internal trigger 1 remap) */
#define TIM_OR_ITR1_RMP_0                    ((uint16_t)0x0400)            /*!<Bit 0 */
#define TIM_OR_ITR1_RMP_1                    ((uint16_t)0x0800)            /*!<Bit 1 */

/******************************************************************************/
/*                                                                            */
/*                             Real-Time Clock                                */
/*                                                                            */
/******************************************************************************/
/********************  Bits definition for RTC_TR register  *******************/
#define RTC_TR_PM                            ((uint32_t)0x00400000)
#define RTC_TR_HT                            ((uint32_t)0x00300000)
#define RTC_TR_HT_0                          ((uint32_t)0x00100000)
#define RTC_TR_HT_1                          ((uint32_t)0x00200000)
#define RTC_TR_HU                            ((uint32_t)0x000F0000)
#define RTC_TR_HU_0                          ((uint32_t)0x00010000)
#define RTC_TR_HU_1                          ((uint32_t)0x00020000)
#define RTC_TR_HU_2                          ((uint32_t)0x00040000)
#define RTC_TR_HU_3                          ((uint32_t)0x00080000)
#define RTC_TR_MNT                           ((uint32_t)0x00007000)
#define RTC_TR_MNT_0                         ((uint32_t)0x00001000)
#define RTC_TR_MNT_1                         ((uint32_t)0x00002000)
#define RTC_TR_MNT_2                         ((uint32_t)0x00004000)
#define RTC_TR_MNU                           ((uint32_t)0x00000F00)
#define RTC_TR_MNU_0                         ((uint32_t)0x00000100)
#define RTC_TR_MNU_1                         ((uint32_t)0x00000200)
#define RTC_TR_MNU_2                         ((uint32_t)0x00000400)
#define RTC_TR_MNU_3                         ((uint32_t)0x00000800)
#define RTC_TR_ST                            ((uint32_t)0x00000070)
#define RTC_TR_ST_0                          ((uint32_t)0x00000010)
#define RTC_TR_ST_1                          ((uint32_t)0x00000020)
#define RTC_TR_ST_2                          ((uint32_t)0x00000040)
#define RTC_TR_SU                            ((uint32_t)0x0000000F)
#define RTC_TR_SU_0                          ((uint32_t)0x00000001)
#define RTC_TR_SU_1                          ((uint32_t)0x00000002)
#define RTC_TR_SU_2                          ((uint32_t)0x00000004)
#define RTC_TR_SU_3                          ((uint32_t)0x00000008)

/********************  Bits definition for RTC_DR register  *******************/
#define RTC_DR_YT                            ((uint32_t)0x00F00000)
#define RTC_DR_YT_0                          ((uint32_t)0x00100000)
#define RTC_DR_YT_1                          ((uint32_t)0x00200000)
#define RTC_DR_YT_2                          ((uint32_t)0x00400000)
#define RTC_DR_YT_3                          ((uint32_t)0x00800000)
#define RTC_DR_YU                            ((uint32_t)0x000F0000)
#define RTC_DR_YU_0                          ((uint32_t)0x00010000)
#define RTC_DR_YU_1                          ((uint32_t)0x00020000)
#define RTC_DR_YU_2                          ((uint32_t)0x00040000)
#define RTC_DR_YU_3                          ((uint32_t)0x00080000)
#define RTC_DR_WDU                           ((uint32_t)0x0000E000)
#define RTC_DR_WDU_0                         ((uint32_t)0x00002000)
#define RTC_DR_WDU_1                         ((uint32_t)0x00004000)
#define RTC_DR_WDU_2                         ((uint32_t)0x00008000)
#define RTC_DR_MT                            ((uint32_t)0x00001000)
#define RTC_DR_MU                            ((uint32_t)0x00000F00)
#define RTC_DR_MU_0                          ((uint32_t)0x00000100)
#define RTC_DR_MU_1                          ((uint32_t)0x00000200)
#define RTC_DR_MU_2                          ((uint32_t)0x00000400)
#define RTC_DR_MU_3                          ((uint32_t)0x00000800)
#define RTC_DR_DT                            ((uint32_t)0x00000030)
#define RTC_DR_DT_0                          ((uint32_t)0x00000010)
#define RTC_DR_DT_1                          ((uint32_t)0x00000020)
#define RTC_DR_DU                            ((uint32_t)0x0000000F)
#define RTC_DR_DU_0                          ((uint32_t)0x00000001)
#define RTC_DR_DU_1                          ((uint32_t)0x00000002)
#define RTC_DR_DU_2                          ((uint32_t)0x00000004)
#define RTC_DR_DU_3                          ((uint32_t)0x00000008)

/********************  Bits definition for RTC_CR register  *******************/
#define RTC_CR_COE                           ((uint32_t)0x00800000)
#define RTC_CR_OSEL                          ((uint32_t)0x00600000)
#define RTC_CR_OSEL_0                        ((uint32_t)0x00200000)
#define RTC_CR_OSEL_1                        ((uint32_t)0x00400000)
#define RTC_CR_POL                           ((uint32_t)0x00100000)
#define RTC_CR_COSEL                         ((uint32_t)0x00080000)
#define RTC_CR_BCK                           ((uint32_t)0x00040000)
#define RTC_CR_SUB1H                         ((uint32_t)0x00020000)
#define RTC_CR_ADD1H                         ((uint32_t)0x00010000)
#define RTC_CR_TSIE                          ((uint32_t)0x00008000)
#define RTC_CR_WUTIE                         ((uint32_t)0x00004000)
#define RTC_CR_ALRBIE                        ((uint32_t)0x00002000)
#define RTC_CR_ALRAIE                        ((uint32_t)0x00001000)
#define RTC_CR_TSE                           ((uint32_t)0x00000800)
#define RTC_CR_WUTE                          ((uint32_t)0x00000400)
#define RTC_CR_ALRBE                         ((uint32_t)0x00000200)
#define RTC_CR_ALRAE                         ((uint32_t)0x00000100)
#define RTC_CR_DCE                           ((uint32_t)0x00000080)
#define RTC_CR_FMT                           ((uint32_t)0x00000040)
#define RTC_CR_BYPSHAD                       ((uint32_t)0x00000020)
#define RTC_CR_REFCKON                       ((uint32_t)0x00000010)
#define RTC_CR_TSEDGE                        ((uint32_t)0x00000008)
#define RTC_CR_WUCKSEL                       ((uint32_t)0x00000007)
#define RTC_CR_WUCKSEL_0                     ((uint32_t)0x00000001)
#define RTC_CR_WUCKSEL_1                     ((uint32_t)0x00000002)
#define RTC_CR_WUCKSEL_2                     ((uint32_t)0x00000004)

/********************  Bits definition for RTC_ISR register  ******************/
#define RTC_ISR_RECALPF                      ((uint32_t)0x00010000)
#define RTC_ISR_TAMP1F                       ((uint32_t)0x00002000)
#define RTC_ISR_TSOVF                        ((uint32_t)0x00001000)
#define RTC_ISR_TSF                          ((uint32_t)0x00000800)
#define RTC_ISR_WUTF                         ((uint32_t)0x00000400)
#define RTC_ISR_ALRBF                        ((uint32_t)0x00000200)
#define RTC_ISR_ALRAF                        ((uint32_t)0x00000100)
#define RTC_ISR_INIT                         ((uint32_t)0x00000080)
#define RTC_ISR_INITF                        ((uint32_t)0x00000040)
#define RTC_ISR_RSF                          ((uint32_t)0x00000020)
#define RTC_ISR_INITS                        ((uint32_t)0x00000010)
#define RTC_ISR_SHPF                         ((uint32_t)0x00000008)
#define RTC_ISR_WUTWF                        ((uint32_t)0x00000004)
#define RTC_ISR_ALRBWF                       ((uint32_t)0x00000002)
#define RTC_ISR_ALRAWF                       ((uint32_t)0x00000001)

/********************  Bits definition for RTC_PRER register  *****************/
#define RTC_PRER_PREDIV_A                    ((uint32_t)0x007F0000)
#define RTC_PRER_PREDIV_S                    ((uint32_t)0x00001FFF)

/********************  Bits definition for RTC_WUTR register  *****************/
#define RTC_WUTR_WUT                         ((uint32_t)0x0000FFFF)

/********************  Bits definition for RTC_CALIBR register  ***************/
#define RTC_CALIBR_DCS                       ((uint32_t)0x00000080)
#define RTC_CALIBR_DC                        ((uint32_t)0x0000001F)

/********************  Bits definition for RTC_ALRMAR register  ***************/
#define RTC_ALRMAR_MSK4                      ((uint32_t)0x80000000)
#define RTC_ALRMAR_WDSEL                     ((uint32_t)0x40000000)
#define RTC_ALRMAR_DT                        ((uint32_t)0x30000000)
#define RTC_ALRMAR_DT_0                      ((uint32_t)0x10000000)
#define RTC_ALRMAR_DT_1                      ((uint32_t)0x20000000)
#define RTC_ALRMAR_DU                        ((uint32_t)0x0F000000)
#define RTC_ALRMAR_DU_0                      ((uint32_t)0x01000000)
#define RTC_ALRMAR_DU_1                      ((uint32_t)0x02000000)
#define RTC_ALRMAR_DU_2                      ((uint32_t)0x04000000)
#define RTC_ALRMAR_DU_3                      ((uint32_t)0x08000000)
#define RTC_ALRMAR_MSK3                      ((uint32_t)0x00800000)
#define RTC_ALRMAR_PM                        ((uint32_t)0x00400000)
#define RTC_ALRMAR_HT                        ((uint32_t)0x00300000)
#define RTC_ALRMAR_HT_0                      ((uint32_t)0x00100000)
#define RTC_ALRMAR_HT_1                      ((uint32_t)0x00200000)
#define RTC_ALRMAR_HU                        ((uint32_t)0x000F0000)
#define RTC_ALRMAR_HU_0                      ((uint32_t)0x00010000)
#define RTC_ALRMAR_HU_1                      ((uint32_t)0x00020000)
#define RTC_ALRMAR_HU_2                      ((uint32_t)0x00040000)
#define RTC_ALRMAR_HU_3                      ((uint32_t)0x00080000)
#define RTC_ALRMAR_MSK2                      ((uint32_t)0x00008000)
#define RTC_ALRMAR_MNT                       ((uint32_t)0x00007000)
#define RTC_ALRMAR_MNT_0                     ((uint32_t)0x00001000)
#define RTC_ALRMAR_MNT_1                     ((uint32_t)0x00002000)
#define RTC_ALRMAR_MNT_2                     ((uint32_t)0x00004000)
#define RTC_ALRMAR_MNU                       ((uint32_t)0x00000F00)
#define RTC_ALRMAR_MNU_0                     ((uint32_t)0x00000100)
#define RTC_ALRMAR_MNU_1                     ((uint32_t)0x00000200)
#define RTC_ALRMAR_MNU_2                     ((uint32_t)0x00000400)
#define RTC_ALRMAR_MNU_3                     ((uint32_t)0x00000800)
#define RTC_ALRMAR_MSK1                      ((uint32_t)0x00000080)
#define RTC_ALRMAR_ST                        ((uint32_t)0x00000070)
#define RTC_ALRMAR_ST_0                      ((uint32_t)0x00000010)
#define RTC_ALRMAR_ST_1                      ((uint32_t)0x00000020)
#define RTC_ALRMAR_ST_2                      ((uint32_t)0x00000040)
#define RTC_ALRMAR_SU                        ((uint32_t)0x0000000F)
#define RTC_ALRMAR_SU_0                      ((uint32_t)0x00000001)
#define RTC_ALRMAR_SU_1                      ((uint32_t)0x00000002)
#define RTC_ALRMAR_SU_2                      ((uint32_t)0x00000004)
#define RTC_ALRMAR_SU_3                      ((uint32_t)0x00000008)

/********************  Bits definition for RTC_ALRMBR register  ***************/
#define RTC_ALRMBR_MSK4                      ((uint32_t)0x80000000)
#define RTC_ALRMBR_WDSEL                     ((uint32_t)0x40000000)
#define RTC_ALRMBR_DT                        ((uint32_t)0x30000000)
#define RTC_ALRMBR_DT_0                      ((uint32_t)0x10000000)
#define RTC_ALRMBR_DT_1                      ((uint32_t)0x20000000)
#define RTC_ALRMBR_DU                        ((uint32_t)0x0F000000)
#define RTC_ALRMBR_DU_0                      ((uint32_t)0x01000000)
#define RTC_ALRMBR_DU_1                      ((uint32_t)0x02000000)
#define RTC_ALRMBR_DU_2                      ((uint32_t)0x04000000)
#define RTC_ALRMBR_DU_3                      ((uint32_t)0x08000000)
#define RTC_ALRMBR_MSK3                      ((uint32_t)0x00800000)
#define RTC_ALRMBR_PM                        ((uint32_t)0x00400000)
#define RTC_ALRMBR_HT                        ((uint32_t)0x00300000)
#define RTC_ALRMBR_HT_0                      ((uint32_t)0x00100000)
#define RTC_ALRMBR_HT_1                      ((uint32_t)0x00200000)
#define RTC_ALRMBR_HU                        ((uint32_t)0x000F0000)
#define RTC_ALRMBR_HU_0                      ((uint32_t)0x00010000)
#define RTC_ALRMBR_HU_1                      ((uint32_t)0x00020000)
#define RTC_ALRMBR_HU_2                      ((uint32_t)0x00040000)
#define RTC_ALRMBR_HU_3                      ((uint32_t)0x00080000)
#define RTC_ALRMBR_MSK2                      ((uint32_t)0x00008000)
#define RTC_ALRMBR_MNT                       ((uint32_t)0x00007000)
#define RTC_ALRMBR_MNT_0                     ((uint32_t)0x00001000)
#define RTC_ALRMBR_MNT_1                     ((uint32_t)0x00002000)
#define RTC_ALRMBR_MNT_2                     ((uint32_t)0x00004000)
#define RTC_ALRMBR_MNU                       ((uint32_t)0x00000F00)
#define RTC_ALRMBR_MNU_0                     ((uint32_t)0x00000100)
#define RTC_ALRMBR_MNU_1                     ((uint32_t)0x00000200)
#define RTC_ALRMBR_MNU_2                     ((uint32_t)0x00000400)
#define RTC_ALRMBR_MNU_3                     ((uint32_t)0x00000800)
#define RTC_ALRMBR_MSK1                      ((uint32_t)0x00000080)
#define RTC_ALRMBR_ST                        ((uint32_t)0x00000070)
#define RTC_ALRMBR_ST_0                      ((uint32_t)0x00000010)
#define RTC_ALRMBR_ST_1                      ((uint32_t)0x00000020)
#define RTC_ALRMBR_ST_2                      ((uint32_t)0x00000040)
#define RTC_ALRMBR_SU                        ((uint32_t)0x0000000F)
#define RTC_ALRMBR_SU_0                      ((uint32_t)0x00000001)
#define RTC_ALRMBR_SU_1                      ((uint32_t)0x00000002)
#define RTC_ALRMBR_SU_2                      ((uint32_t)0x00000004)
#define RTC_ALRMBR_SU_3                      ((uint32_t)0x00000008)

/********************  Bits definition for RTC_WPR register  ******************/
#define RTC_WPR_KEY                          ((uint32_t)0x000000FF)

/********************  Bits definition for RTC_SSR register  ******************/
#define RTC_SSR_SS                           ((uint32_t)0x0000FFFF)

/********************  Bits definition for RTC_SHIFTR register  ***************/
#define RTC_SHIFTR_SUBFS                     ((uint32_t)0x00007FFF)
#define RTC_SHIFTR_ADD1S                     ((uint32_t)0x80000000)

/********************  Bits definition for RTC_TSTR register  *****************/
#define RTC_TSTR_PM                          ((uint32_t)0x00400000)
#define RTC_TSTR_HT                          ((uint32_t)0x00300000)
#define RTC_TSTR_HT_0                        ((uint32_t)0x00100000)
#define RTC_TSTR_HT_1                        ((uint32_t)0x00200000)
#define RTC_TSTR_HU                          ((uint32_t)0x000F0000)
#define RTC_TSTR_HU_0                        ((uint32_t)0x00010000)
#define RTC_TSTR_HU_1                        ((uint32_t)0x00020000)
#define RTC_TSTR_HU_2                        ((uint32_t)0x00040000)
#define RTC_TSTR_HU_3                        ((uint32_t)0x00080000)
#define RTC_TSTR_MNT                         ((uint32_t)0x00007000)
#define RTC_TSTR_MNT_0                       ((uint32_t)0x00001000)
#define RTC_TSTR_MNT_1                       ((uint32_t)0x00002000)
#define RTC_TSTR_MNT_2                       ((uint32_t)0x00004000)
#define RTC_TSTR_MNU                         ((uint32_t)0x00000F00)
#define RTC_TSTR_MNU_0                       ((uint32_t)0x00000100)
#define RTC_TSTR_MNU_1                       ((uint32_t)0x00000200)
#define RTC_TSTR_MNU_2                       ((uint32_t)0x00000400)
#define RTC_TSTR_MNU_3                       ((uint32_t)0x00000800)
#define RTC_TSTR_ST                          ((uint32_t)0x00000070)
#define RTC_TSTR_ST_0                        ((uint32_t)0x00000010)
#define RTC_TSTR_ST_1                        ((uint32_t)0x00000020)
#define RTC_TSTR_ST_2                        ((uint32_t)0x00000040)
#define RTC_TSTR_SU                          ((uint32_t)0x0000000F)
#define RTC_TSTR_SU_0                        ((uint32_t)0x00000001)
#define RTC_TSTR_SU_1                        ((uint32_t)0x00000002)
#define RTC_TSTR_SU_2                        ((uint32_t)0x00000004)
#define RTC_TSTR_SU_3                        ((uint32_t)0x00000008)

/********************  Bits definition for RTC_TSDR register  *****************/
#define RTC_TSDR_WDU                         ((uint32_t)0x0000E000)
#define RTC_TSDR_WDU_0                       ((uint32_t)0x00002000)
#define RTC_TSDR_WDU_1                       ((uint32_t)0x00004000)
#define RTC_TSDR_WDU_2                       ((uint32_t)0x00008000)
#define RTC_TSDR_MT                          ((uint32_t)0x00001000)
#define RTC_TSDR_MU                          ((uint32_t)0x00000F00)
#define RTC_TSDR_MU_0                        ((uint32_t)0x00000100)
#define RTC_TSDR_MU_1                        ((uint32_t)0x00000200)
#define RTC_TSDR_MU_2                        ((uint32_t)0x00000400)
#define RTC_TSDR_MU_3                        ((uint32_t)0x00000800)
#define RTC_TSDR_DT                          ((uint32_t)0x00000030)
#define RTC_TSDR_DT_0                        ((uint32_t)0x00000010)
#define RTC_TSDR_DT_1                        ((uint32_t)0x00000020)
#define RTC_TSDR_DU                          ((uint32_t)0x0000000F)
#define RTC_TSDR_DU_0                        ((uint32_t)0x00000001)
#define RTC_TSDR_DU_1                        ((uint32_t)0x00000002)
#define RTC_TSDR_DU_2                        ((uint32_t)0x00000004)
#define RTC_TSDR_DU_3                        ((uint32_t)0x00000008)

/********************  Bits definition for RTC_TSSSR register  ****************/
#define RTC_TSSSR_SS                         ((uint32_t)0x0000FFFF)

/********************  Bits definition for RTC_CAL register  *****************/
#define RTC_CALR_CALP                        ((uint32_t)0x00008000)
#define RTC_CALR_CALW8                       ((uint32_t)0x00004000)
#define RTC_CALR_CALW16                      ((uint32_t)0x00002000)
#define RTC_CALR_CALM                        ((uint32_t)0x000001FF)
#define RTC_CALR_CALM_0                      ((uint32_t)0x00000001)
#define RTC_CALR_CALM_1                      ((uint32_t)0x00000002)
#define RTC_CALR_CALM_2                      ((uint32_t)0x00000004)
#define RTC_CALR_CALM_3                      ((uint32_t)0x00000008)
#define RTC_CALR_CALM_4                      ((uint32_t)0x00000010)
#define RTC_CALR_CALM_5                      ((uint32_t)0x00000020)
#define RTC_CALR_CALM_6                      ((uint32_t)0x00000040)
#define RTC_CALR_CALM_7                      ((uint32_t)0x00000080)
#define RTC_CALR_CALM_8                      ((uint32_t)0x00000100)

/********************  Bits definition for RTC_TAFCR register  ****************/
#define RTC_TAFCR_ALARMOUTTYPE               ((uint32_t)0x00040000)
#define RTC_TAFCR_TSINSEL                    ((uint32_t)0x00020000)
#define RTC_TAFCR_TAMPINSEL                  ((uint32_t)0x00010000)
#define RTC_TAFCR_TAMPPUDIS                  ((uint32_t)0x00008000)
#define RTC_TAFCR_TAMPPRCH                   ((uint32_t)0x00006000)
#define RTC_TAFCR_TAMPPRCH_0                 ((uint32_t)0x00002000)
#define RTC_TAFCR_TAMPPRCH_1                 ((uint32_t)0x00004000)
#define RTC_TAFCR_TAMPFLT                    ((uint32_t)0x00001800)
#define RTC_TAFCR_TAMPFLT_0                  ((uint32_t)0x00000800)
#define RTC_TAFCR_TAMPFLT_1                  ((uint32_t)0x00001000)
#define RTC_TAFCR_TAMPFREQ                   ((uint32_t)0x00000700)
#define RTC_TAFCR_TAMPFREQ_0                 ((uint32_t)0x00000100)
#define RTC_TAFCR_TAMPFREQ_1                 ((uint32_t)0x00000200)
#define RTC_TAFCR_TAMPFREQ_2                 ((uint32_t)0x00000400)
#define RTC_TAFCR_TAMPTS                     ((uint32_t)0x00000080)
#define RTC_TAFCR_TAMPIE                     ((uint32_t)0x00000004)
#define RTC_TAFCR_TAMP1TRG                   ((uint32_t)0x00000002)
#define RTC_TAFCR_TAMP1E                     ((uint32_t)0x00000001)

/********************  Bits definition for RTC_ALRMASSR register  *************/
#define RTC_ALRMASSR_MASKSS                  ((uint32_t)0x0F000000)
#define RTC_ALRMASSR_MASKSS_0                ((uint32_t)0x01000000)
#define RTC_ALRMASSR_MASKSS_1                ((uint32_t)0x02000000)
#define RTC_ALRMASSR_MASKSS_2                ((uint32_t)0x04000000)
#define RTC_ALRMASSR_MASKSS_3                ((uint32_t)0x08000000)
#define RTC_ALRMASSR_SS                      ((uint32_t)0x00007FFF)

/********************  Bits definition for RTC_ALRMBSSR register  *************/
#define RTC_ALRMBSSR_MASKSS                  ((uint32_t)0x0F000000)
#define RTC_ALRMBSSR_MASKSS_0                ((uint32_t)0x01000000)
#define RTC_ALRMBSSR_MASKSS_1                ((uint32_t)0x02000000)
#define RTC_ALRMBSSR_MASKSS_2                ((uint32_t)0x04000000)
#define RTC_ALRMBSSR_MASKSS_3                ((uint32_t)0x08000000)
#define RTC_ALRMBSSR_SS                      ((uint32_t)0x00007FFF)

/******************************************************************************/
/*                                                                            */
/*                           WATCHDOG                                         */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for WDG_CR register  *********************/
#define  WDT_CR_EN                          (1UL << 0)          /*!< enable wdt */
#define  WDT_CR_RMOD_RST                    (0UL << 1)          /*!< generate system reset */
#define  WDT_CR_RMOD_INT                    (1UL << 1)          /*!< generate interrupt at 1st */
#define  WDT_CR_RPL_2                       (0x0 << 2)          /*!< reset pulse length: 2 pclk cycles */
#define  WDT_CR_RPL_4                       (0x1 << 2)
#define  WDT_CR_RPL_8                       (0x2 << 2)
#define  WDT_CR_RPL_16                      (0x3 << 2)
#define  WDT_CR_RPL_32                      (0x4 << 2)
#define  WDT_CR_RPL_64                      (0x5 << 2)
#define  WDT_CR_RPL_128                     (0x6 << 2)
#define  WDT_CR_RPL_256                     (0x7 << 2)

#define  WDT_CLOCK_EN_VALUE                 0x03
#define  WDT_COUNTER_RESTART_KICK_VALUE     0x76
#define  WDT_CONTROL_REG_WDT_EN_MASK        0x01
/******************************************************************************/
/*                                                                            */
/*                          SD host conroller                                 */
/*                                                                            */
/******************************************************************************/

/* Control register defines */
#define SDHC_CTRL_USE_IDMAC		(1UL << 25)
#define SDHC_CTRL_ENABLE_OD_PULLUP	(1UL << 24) //1-open-drain
#define SDHC_CTRL_CARD_VOLTAGE_B(n)	((n) << 20) //optinal for regulator-B
#define SDHC_CTRL_CARD_VOLTAGE_A(n)	((n) << 16)
#define SDHC_CTRL_CEATA_INT_EN		(1UL << 11) //CE-ATA
#define SDHC_CTRL_SEND_AS_CCSD		(1UL << 10) //CE-ATA
#define SDHC_CTRL_SEND_CCSD		(1UL << 9)  //CE-ATA
#define SDHC_CTRL_ABRT_READ_DATA	(1UL << 8)  //for SDIO suspend occurs
#define SDHC_CTRL_SEND_IRQ_RESP		(1UL << 7)  //send MMC CMD40's response
#define SDHC_CTRL_READ_WAIT		(1UL << 6)  //0-clear 1-assert for SDIO
#define SDHC_CTRL_DMA_ENABLE		(1UL << 5)
#define SDHC_CTRL_INT_ENABLE		(1UL << 4)
#define SDHC_CTRL_DMA_RESET		(1UL << 2)  //auto clear for all reset bits
#define SDHC_CTRL_FIFO_RESET		(1UL << 1)
#define SDHC_CTRL_RESET			(1UL << 0)

/*******************  Bit definition for SDHC_PWREN register  *****************/
#define SDHC_PWREN_ENABLE		(1UL << 0)  //optional for power switch

/*******************  Bit definition for SDHC_CLKDIV register *****************/
#define CLK_DIVIDER0_MSK        ((uint32)0xFF)                             /*!< clock divider: Fcclk_out = Fcclk / (CLK_DIVIDER0 * 2) */

/*******************  Bit definition for SDHC_CLKSRC register *****************/
#define CLK_SOURCE_MSK          ((uint32)0x3)                             /*!< clock source is always 0, i.e. CLK_DIVIDER0 */

/* Clock Enable register defines */
#define SDHC_CLKEN_LOW_PWR		(1UL << 16) //1-stop clock if card idle
#define SDHC_CLKEN_ENABLE		(1UL << 0)

/* time-out register defines */
#define SDHC_TMOUT_DATA(n)		((n) << 8)
#define SDHC_TMOUT_DATA_MSK		0xFFFFFF00
#define SDHC_TMOUT_RESP(n)		((n) & 0xFF)
#define SDHC_TMOUT_RESP_MSK		0xFF

/* card-type register defines */
#define SDHC_CTYPE_8BIT		(1UL << 16)
#define SDHC_CTYPE_4BIT		(1UL << 0)
#define SDHC_CTYPE_1BIT		0

/* Interrupt status & mask register defines */
#define SDHC_INT_SDIO(n)		(1UL << (16 + (n)))
#define SDHC_INT_EBE			(1UL << 15) //End Bit Error
#define SDHC_INT_ACD			(1UL << 14) //Auto Command Done
#define SDHC_INT_SBE			(1UL << 13) //Start Bit Error
#define SDHC_INT_HLE			(1UL << 12) //Hardware Locked Error
#define SDHC_INT_FRUN			(1UL << 11) //FIFO UnderrunOverrun Error
#define SDHC_INT_HTO			(1UL << 10) //Data Starvation
#define SDHC_INT_DTO			(1UL << 9) //Data Read TimeOut
#define SDHC_INT_RTO			(1UL << 8) //Response TimeOut
#define SDHC_INT_DCRC			(1UL << 7) //Data CRC Error
#define SDHC_INT_RCRC			(1UL << 6) //Response CRC Error
#define SDHC_INT_RXDR			(1UL << 5) //Receive FIFO Data Request
#define SDHC_INT_TXDR			(1UL << 4) //Transmit FIFO Data Request
#define SDHC_INT_DATA_OVER		(1UL << 3) //Data Transfer Over
#define SDHC_INT_CMD_DONE		(1UL << 2) //Command Done
#define SDHC_INT_RESP_ERR		(1UL << 1) //Response Error
#define SDHC_INT_CD			(1UL << 0) //Card Detect
#define SDHC_INT_ERROR			0xbfc2

/* Command register defines */
#define SDHC_CMD_START			(1UL << 31)
#define SDHC_CMD_CCS_EXP		(1UL << 23)
#define SDHC_CMD_HOLD				(1UL << 29)
#define SDHC_CMD_CEATA_RD		(1UL << 22)
#define SDHC_CMD_UPD_CLK		(1UL << 21)
#define SDHC_CMD_INIT			(1UL << 15)
#define SDHC_CMD_STOP			(1UL << 14)
#define SDHC_CMD_PRV_DAT_WAIT		(1UL << 13)
#define SDHC_CMD_SEND_STOP		(1UL << 12)
#define SDHC_CMD_STRM_MODE		(1UL << 11) //0-block; 1-stream mode
#define SDHC_CMD_DAT_WR			(1UL << 10) //0-read;  1-write
#define SDHC_CMD_DAT_EXP		(1UL << 9)  //0-no data expected
#define SDHC_CMD_RESP_CRC		(1UL << 8)
#define SDHC_CMD_RESP_LONG		(1UL << 7)
#define SDHC_CMD_RESP_EXP		(1UL << 6)
#define SDHC_CMD_INDX(n)		((n) & 0x1F)

/* Status register defines */
#define SDHC_STATUS_DMA_REQ		(1UL << 31)
#define SDHC_STATUS_DMA_ACK		(1UL << 30)
#define SDHC_GET_FCNT(x)		(((x)>>17) & 0x1FFF)
#define SDHC_STATUS_DATA0_BUSY		(1UL << 9) //card busy:    !cdata[0]
#define SDHC_STATUS_DATA3_CD		(1UL << 8) //card present: !cdata[3]
#define SDHC_STATUS_FIFO_FULL		(1UL << 3)
#define SDHC_STATUS_FIFO_EMPTY		(1UL << 2)

/* Internal DMAC interrupt defines */
#define SDHC_IDMAC_INT_AI		(1UL << 9) //abnormal: FBE, DU, CES
#define SDHC_IDMAC_INT_NI		(1UL << 8) //normal:   TI, RI
#define SDHC_IDMAC_INT_CES		(1UL << 5) //card error interrupt
#define SDHC_IDMAC_INT_DU		(1UL << 4) //descriptor unavailable
#define SDHC_IDMAC_INT_FBE		(1UL << 2) //fatal bus error
#define SDHC_IDMAC_INT_RI		(1UL << 1)
#define SDHC_IDMAC_INT_TI		(1UL << 0)

/* Internal DMAC bus mode bits */
#define SDHC_IDMAC_ENABLE		(1UL << 7)
#define SDHC_IDMAC_FB			(1UL << 1) //fixed burst
#define SDHC_IDMAC_SWRESET		(1UL << 0)

/******************************************************************************/
/*                                                                            */
/*                          USB OTG FS                                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                        Serial Peripheral Interface                         */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for SPI_CTRL0 register  ******************/
#define  SPI_CTRL0_DFS_MSK                   ((uint16_t)0x000F)            /*!< data frame size = n+1 */
#define  SPI_CTRL0_DFS_SFT                   0
#define  SPI_CTRL0_DFS(x)                    (((x) - 1) << SPI_CTRL0_DFS_SFT)
#define  SPI_CTRL0_FRF_MOTO                  (0x0 << 4)                    /*!< frame format: motorola SPI */
#define  SPI_CTRL0_FRF_TI                    (0x1 << 4)                    /*!<               TI SSP */
#define  SPI_CTRL0_FRF_NS                    (0x2 << 4)                    /*!<               NS Microwire */
#define  SPI_CTRL0_SCPH                      (1UL << 6)                    /*!< serial clock phase */
#define  SPI_CTRL0_SCPOL                     (1UL << 7)                    /*!< serial clock polarity */
#define  SPI_CTRL0_TMOD_TXRX                 (0x0 << 8)                    /*!< transer mode: transmit & receive */
#define  SPI_CTRL0_TMOD_TX                   (0x1 << 8)                    /*!<               transmit */
#define  SPI_CTRL0_TMOD_RX                   (0x2 << 8)                    /*!<               receive */
#define  SPI_CTRL0_TMOD_EEPROM               (0x3 << 8)                    /*!<               eeprom */
#define  SPI_CTRL0_SLV_OE                    (1UL << 10)                   /*!< 0: enabele, 1: disable slave txd */
#define  SPI_CTRL0_SRL                       (1UL << 11)                   /*!< 0: normal,  1: loopback */
#define  SPI_CTRL0_CFS_MSK                   ((uint16_t)0xF000)            /*!< control frame size = n+1 */
#define  SPI_CTRL0_CFS_SFT                   12
#define  SPI_CTRL0_CFS(x)                    (((x) - 1) << SPI_CTRL0_CFS_SFT)

/*******************  Bit definition for SPI_CTRL1 register  ******************/
#define  SPI_CTRL1_NDF(x)                    ((x) - 1)                     /*!< number of data frames = n + 1 */

/*******************  Bit definition for SPI_SSIENR register  ****************/
#define  SPI_SSIENR_EN                       (1UL << 0)                    /*!< 0: disable, halt transfer, flush FIFOs, program CTRLx; 1: enabel */

/*******************  Bit definition for SPI_MWCR register  ******************/

/*******************  Bit definition for SPI_SER register  *******************/
#define  SPI_SER(n)                          (1UL << (n))                  /*!< slave select enable */

/*******************  Bit definition for SPI_BAUDR register  *****************/
#define  SPI_BAUDR_SCKDV_MSK                 ((uint16)0xFFFF)              /*!< clock divider: Fspi_clk = Fsclk / SCKDV */

/*******************  Bit definition for SPI_TXFTLR register ****************/
#define  SPI_TXFTLR_TFT(n)                   (n)                           /*!< transmit FIFO threshold: txe_intr if <= n */

/*******************  Bit definition for SPI_RXFTLR register ****************/
#define  SPI_RXFTLR_RFT(n)                   (n)                           /*!< receive FIFO threshold: rxf_intr if > n */

/*******************  Bit definition for SPI_SR register ********************/
#define  SPI_SR_BUSY                         (1UL << 0)                    /*!< 0: idle or disabled; 1: transfering data */
#define  SPI_SR_TFNF                         (1UL << 1)                    /*!< 0: tFIFO is full;      1: not full */
#define  SPI_SR_TFE                          (1UL << 2)                    /*!< 0: tFIFO is not empty; 1: empty */
#define  SPI_SR_RFNE                         (1UL << 3)                    /*!< 0: rFIFO is empty;     1: not empty */
#define  SPI_SR_RFF                          (1UL << 4)                    /*!< 0: rFIFO is not full;  1: full */
#define  SPI_SR_TXE                          (1UL << 5)                    /*!< 0: no error;         1: transmission error */
#define  SPI_SR_DCOL                         (1UL << 6)                    /*!< 0: no error;         1: transmit data collision error */

/*******************  Bit definition for SPI_IMR register *******************/
#define  SPI_IMR_TXEIM                       (1UL << 0)                    /*!< tFIFO empty interrupt mask */
#define  SPI_IMR_TXOIM                       (1UL << 1)                    /*!< tFIFO overflow interrupt mask */
#define  SPI_IMR_RXUIM                       (1UL << 2)                    /*!< rFIFO underflow interrupt mask */
#define  SPI_IMR_RXOIM                       (1UL << 3)                    /*!< rFIFO overflow interrupt mask */
#define  SPI_IMR_RXFIM                       (1UL << 4)                    /*!< rFIFO full interrupt mask */
#define  SPI_IMR_MSTIM                       (1UL << 5)                    /*!< multi-maskter contention interrupt mask */

/*******************  Bit definition for SPI_ISR register *******************/
/*******************  Bit definition for SPI_RISR register ******************/

/******************************************************************************/
/*                                                                            */
/*                      Inter-integrated Circuit Interface                    */
/*                                                                            */
/******************************************************************************/

/* Control register defines */
#define I2C_CON_MASTER                       (1UL << 0)
#define I2C_CON_SPEED_STD                    (1UL << 1)
#define I2C_CON_SPEED_FAST                   (2UL << 1)
#define I2C_CON_10BITADDR_SLAVE              (1UL << 3)
#define I2C_CON_10BITADDR_MASTER             (1UL << 4)
#define I2C_CON_RESTART_EN                   (1UL << 5)
#define I2C_CON_SLAVE_DISABLE                (1UL << 6)

/* Interrupt status & mask registers defines */
#define I2C_INTR_RX_UNDER                    (1UL << 0)
#define I2C_INTR_RX_OVER                     (1UL << 1)
#define I2C_INTR_RX_FULL                     (1UL << 2)
#define I2C_INTR_TX_OVER                     (1UL << 3)
#define I2C_INTR_TX_EMPTY                    (1UL << 4)
#define I2C_INTR_RD_REQ                      (1UL << 5)
#define I2C_INTR_TX_ABRT                     (1UL << 6)
#define I2C_INTR_RX_DONE                     (1UL << 7)
#define I2C_INTR_ACTIVITY                    (1UL << 8)
#define I2C_INTR_STOP_DET                    (1UL << 9)
#define I2C_INTR_START_DET                   (1UL << 10)
#define I2C_INTR_GEN_CALL                    (1UL << 11)

/* Status register defines */
#define I2C_STATUS_ACTIVITY                  (1UL << 0)

/* Abort source register defines */
#define I2C_TX_ABRT_7B_ADDR_NOACK            (1UL << 0)
#define I2C_TX_ABRT_10ADDR1_NOACK            (1UL << 1)
#define I2C_TX_ABRT_10ADDR2_NOACK            (1UL << 2)
#define I2C_TX_ABRT_TXDATA_NOACK             (1UL << 3)
#define I2C_TX_ABRT_GCALL_NOACK              (1UL << 4)
#define I2C_TX_ABRT_GCALL_READ               (1UL << 5)
#define I2C_TX_ABRT_SBYTE_ACKDET             (1UL << 7)
#define I2C_TX_ABRT_SBYTE_NORSTRT            (1UL << 9)
#define I2C_TX_ABRT_10B_RD_NORSTRT           (1UL << 10)
#define I2C_TX_ABRT_MASTER_DIS               (1UL << 11)
#define I2C_TX_ARB_LOST                      (1UL << 12)

#define I2C_DMA_CR_TDMAE                     (1UL << 1)
#define I2C_DMA_CR_RDMAE                     (1UL << 0)
#define I2C_CON1_TX_ENABLE                   (0UL << 12)
#define I2C_CON1_RX_ENABLE                   (1UL << 12)
#define I2C_CON1_READBYTES_UPDATE            (1UL << 16)
#define I2C_CON1_CLEAR_I2C_ENABLE            (1UL << 31)
/******************************************************************************/
/*                                                                            */
/*                     Universal Asynchronous Receiver Transmitter            */
/*                                                                            */
/******************************************************************************/
/*
 * These are the definitions for the FIFO Control Register
 */
#define UART_FCR_FIFO_EN 	0x01 /* Fifo enable */
#define UART_FCR_CLEAR_RCVR	0x02 /* Clear the RCVR FIFO */
#define UART_FCR_CLEAR_XMIT	0x04 /* Clear the XMIT FIFO */
#define UART_FCR_DMA_SELECT	0x08 /* For DMA applications */
#define UART_FCR_TRIGGER_MASK	0xC0 /* Mask for the FIFO trigger range */
#define UART_FCR_TRIGGER_1	0x00 /* Mask for trigger set at 1 */
#define UART_FCR_TRIGGER_4	0x40 /* Mask for trigger set at 4 */
#define UART_FCR_TRIGGER_8	0x80 /* Mask for trigger set at 8 */
#define UART_FCR_TRIGGER_14	0xC0 /* Mask for trigger set at 14 */

#define UART_FCR_RXSR		0x02 /* Receiver soft reset */
#define UART_FCR_TXSR		0x04 /* Transmitter soft reset */

/*
 * These are the definitions for the Modem Control Register
 */
#define UART_MCR_DTR	0x01		/* DTR   */
#define UART_MCR_RTS	0x02		/* RTS   */
#define UART_MCR_OUT1	0x04		/* Out 1 */
#define UART_MCR_OUT2	0x08		/* Out 2 */
#define UART_MCR_LOOP	0x10		/* Enable loopback test mode */

#define UART_MCR_DMA_EN	0x04
#define UART_MCR_TX_DFR	0x08

/*
 * These are the definitions for the Line Control Register
 *
 * Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting
 * UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
 */
#define UART_LCR_WLS_MSK 0x03		/* character length select mask */
#define UART_LCR_WLS_5	0x00		/* 5 bit character length */
#define UART_LCR_WLS_6	0x01		/* 6 bit character length */
#define UART_LCR_WLS_7	0x02		/* 7 bit character length */
#define UART_LCR_WLS_8	0x03		/* 8 bit character length */
#define UART_LCR_STB	0x04		/* Number of stop Bits, off = 1, on = 1.5 or 2) */
#define UART_LCR_PEN	0x08		/* Parity eneble */
#define UART_LCR_EPS	0x10		/* Even Parity Select */
#define UART_LCR_STKP	0x20		/* Stick Parity */
#define UART_LCR_SBRK	0x40		/* Set Break */
#define UART_LCR_BKSE	0x80		/* Bank select enable */
#define UART_LCR_DLAB	0x80		/* Divisor latch access bit */

/*
 * These are the definitions for the Line Status Register
 */
#define UART_LSR_DR	0x01		/* Data ready */
#define UART_LSR_OE	0x02		/* Overrun */
#define UART_LSR_PE	0x04		/* Parity error */
#define UART_LSR_FE	0x08		/* Framing error */
#define UART_LSR_BI	0x10		/* Break */
#define UART_LSR_THRE	0x20		/* Xmit holding register empty */
#define UART_LSR_TEMT	0x40		/* Xmitter empty */
#define UART_LSR_ERR	0x80		/* Error */

#define UART_MSR_DCD	0x80		/* Data Carrier Detect */
#define UART_MSR_RI	0x40		/* Ring Indicator */
#define UART_MSR_DSR	0x20		/* Data Set Ready */
#define UART_MSR_CTS	0x10		/* Clear to Send */
#define UART_MSR_DDCD	0x08		/* Delta DCD */
#define UART_MSR_TERI	0x04		/* Trailing edge ring indicator */
#define UART_MSR_DDSR	0x02		/* Delta DSR */
#define UART_MSR_DCTS	0x01		/* Delta CTS */

/*
 * These are the definitions for the Interrupt Identification Register
 */
#define UART_IIR_NO_INT	0x01	/* No interrupts pending */
#define UART_IIR_ID	0x0F	/* Mask for the interrupt ID */

#define UART_IIR_MSI	0x00	/* Modem status interrupt */
#define UART_IIR_THRI	0x02	/* Transmitter holding register empty */
#define UART_IIR_RDI	0x04	/* Receiver data interrupt */
#define UART_IIR_RLSI	0x06	/* Receiver line status interrupt */
#define UART_IIR_BDI	0x07	/* busy detect */
#define UART_IIR_CTI	0x0C	/* character timeout */

/*
 * These are the definitions for the Interrupt Enable Register
 */
#define UART_IER_MSI	0x08	/* Enable Modem status interrupt */
#define UART_IER_RLSI	0x04	/* Enable receiver line status interrupt */
#define UART_IER_THRI	0x02	/* Enable Transmitter holding register int. */
#define UART_IER_RDI	0x01	/* Enable receiver data interrupt */

/* useful defaults for LCR */
#define UART_LCR_8N1	0x03

/**
 * Peripheral_Registers_Bits_Definition
 * @}
 */

 /**
  * Exported_constants
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __HS66xx_H */

/**
 * CMSIS
 * @}
 */


/******************* (C) COPYRIGHT 2014 HunterSun **************END OF FILE****/
