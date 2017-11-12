/*
 * 说明： HS6200相关寄存器声明
 *
 *	Yao
 *  HunterSun Electronics Co., Ltd.
 */

#ifndef HS6200_REG_H
#define HS6200_REG_H

#include "HS6200_types.h"
#include "cfg_define.h"

/*
 * 对HS6200的寄存器进行扩充，BANK1寄存器的最高位为1，而BANK0为实际地址。 
 * 
 */
#define HS6200_BANK_BIT    BIT7      /*bank 指示位*/
#define HS6200_BANK1       BIT7
#define HS6200_BANK0       0x00  

//HS6200 Bank0 Register Addr
/**************************************************/               
#define HS6200_BANK0_CONFIG                     0x00 // 'Config' register address
#define HS6200_BANK0_EN_AA                      0x01 // 'Enable Auto Acknowledgment' register address
#define HS6200_BANK0_EN_RXADDR                  0x02 // 'Enabled RX addresses' register address
#define HS6200_BANK0_SETUP_AW                   0x03 // 'Setup address width' register address
#define HS6200_BANK0_SETUP_RETR                 0x04 // 'Setup Auto. Retrans' register address
#define HS6200_BANK0_RF_CH                      0x05 // 'RF channel' register address
#define HS6200_BANK0_RF_SETUP                   0x06 // 'RF setup' register address
#define HS6200_BANK0_STATUS                     0x07 // 'Status' register address
#define HS6200_BANK0_OBSERVE_TX                 0x08 // 'Observe TX' register address
#define HS6200_BANK0_RPD                        0x09 // 'Received Power Detector' register address
#define HS6200_BANK0_RX_ADDR_P0                 0x0A // 'RX address pipe0' register address
#define HS6200_BANK0_RX_ADDR_P1                 0x0B // 'RX address pipe1' register address
#define HS6200_BANK0_RX_ADDR_P2                 0x0C // 'RX address pipe2' register address
#define HS6200_BANK0_RX_ADDR_P3                 0x0D // 'RX address pipe3' register address
#define HS6200_BANK0_RX_ADDR_P4                 0x0E // 'RX address pipe4' register address
#define HS6200_BANK0_RX_ADDR_P5                 0x0F // 'RX address pipe5' register address
#define HS6200_BANK0_TX_ADDR                    0x10 // 'TX address' register address
#define HS6200_BANK0_RX_PW_P0                   0x11 // 'RX payload width, pipe0' register address
#define HS6200_BANK0_RX_PW_P1                   0x12 // 'RX payload width, pipe1' register address
#define HS6200_BANK0_RX_PW_P2                   0x13 // 'RX payload width, pipe2' register address
#define HS6200_BANK0_RX_PW_P3                   0x14 // 'RX payload width, pipe3' register address
#define HS6200_BANK0_RX_PW_P4                   0x15 // 'RX payload width, pipe4' register address
#define HS6200_BANK0_RX_PW_P5                   0x16 // 'RX payload width, pipe5' register address
#define HS6200_BANK0_FIFO_STATUS                0x17 // 'FIFO Status Register' register address
#define HS6200_BANK0_DYNPD                      0x1C // 'Enable dynamic payload length' register address
#define HS6200_BANK0_FEATURE                    0x1D // 'Feature' register address
#define HS6200_BANK0_SETUP_VALUE                0x1E
#define HS6200_BANK0_PRE_GURD                   0x1F

//HS6200 Bank1 register
#define HS6200_BANK1_RESERVED0            0x00             //reserved
#define HS6200_BANK1_PLL_CTL0             0x01
#define HS6200_BANK1_PLL_CTL1             0x02
#define HS6200_BANK1_CAL_CTL              0x03
#define HS6200_BANK1_A_CNT_REG            0x04
#define HS6200_BANK1_B_CNT_REG            0x05
#define HS6200_BANK1_RESERVED1            0x06             //reserved
#define HS6200_BANK1_STATUS               0x07
#define HS6200_BANK1_RESERVED2            0x08             //reserved
#define HS6200_BANK1_CHAN                 0x09
#define HS6200_BANK1_IF_FREQ              0x0A
#define HS6200_BANK1_AFC_COR              0x0B
#define HS6200_BANK1_FDEV                 0x0C
#define HS6200_BANK1_DAC_RANGE            0x0D
#define HS6200_BANK1_DAC_IN               0x0E
#define HS6200_BANK1_CTUNING              0x0F
#define HS6200_BANK1_FTUNING              0x10
#define HS6200_BANK1_RX_CTRL              0x11
#define HS6200_BANK1_FAGC_CTRL            0x12
#define HS6200_BANK1_FAGC_CTRL_1          0x13           //HS6200B1新增  wangs 2013-12-26
#define HS6200_BANK1_DAC_CAL_LOW          0x17
#define HS6200_BANK1_DAC_CAL_HI           0x18
#define HS6200_BANK1_RESERVED3            0x19            //reserved   
#define HS6200_BANK1_DOC_DACI             0x1A
#define HS6200_BANK1_DOC_DACQ             0x1B
#define HS6200_BANK1_AGC_CTRL             0x1C
#define HS6200_BANK1_AGC_GAIN             0x1D
#define HS6200_BANK1_RF_IVGEN             0x1E
#define HS6200_BANK1_TEST_PKDET           0x1F


//HS6200B1 修改的寄存器
#define HS6200B1_BANK1_LINE               0x00
#define HS6200B1_BANK1_STATE              0x08
#define HS6200B1_FAGC_CTRL_1              0x13



//wrire 1 to Bank0 all register
#define HS6200_BANK0_CONFIG_1_VALUE       	0x7F
#define HS6200_BANK0_EN_AA_1_VALUE          0x3F	
#define HS6200_BANK0_EN_RXADDR_1_VALUE		  0x3F
#define HS6200_BANK0_SETUP_AW_1_VALUE		    0x03
#define HS6200_BANK0_SETUP_RETR_1_VALUE     0xFF
#define HS6200_BANK0_RF_CH_1_VALUE          0x7F
#define HS6200_BANK0_RF_SETUP_1_VALUE       0xFF // or 0x0f
#define HS6200_BANK0_STATUS_1_VALUE         0x7F
#define HS6200_BANK0_OBSERVE_TX_1_VALUE     0xFF
#define HS6200_BANK0_RPD_1_VALUE            0x01
#define HS6200_BANK0_RX_ADDR_P0_1_VALUE     0xFFFFFFFFFF
#define HS6200_BANK0_RX_ADDR_P1_1_VALUE     0xFFFFFFFFFF
#define HS6200_BANK0_RX_ADDR_P2_1_VALUE     0xFF
#define HS6200_BANK0_RX_ADDR_P3_1_VALUE     0xFF
#define HS6200_BANK0_RX_ADDR_P4_1_VALUE     0xFF
#define HS6200_BANK0_RX_ADDR_P5_1_VALUE     0xFF
#define HS6200_BANK0_TX_ADDR_1_VALUE        0xFFFFFFFFFF
#define HS6200_BANK0_RX_PW_P0_1_VALUE       0x3F
#define HS6200_BANK0_RX_PW_P1_1_VALUE       0x3F
#define HS6200_BANK0_RX_PW_P2_1_VALUE       0x3F
#define HS6200_BANK0_RX_PW_P3_1_VALUE       0x3F
#define HS6200_BANK0_RX_PW_P4_1_VALUE       0x3F
#define HS6200_BANK0_RX_PW_P5_1_VALUE       0x3F
#define HS6200_BANK0_FIFO_STATUS_1_VALUE    0x73
#define HS6200_BANK0_DYNPD_1_VALUE          0x3F
#define HS6200_BANK0_FEATURE_1_VALUE        0x07
#define HS6200_BANK0_SETUP_VALUE_1_VALUE    0xFFFF
#define HS6200_BANK0_PRE_GURD_1_VALUE       0x7F 

//wrire 0 to Bank0 all register
#define HS6200_BANK0_CONFIG_0_VALUE     	0x00
#define HS6200_BANK0_EN_AA_0_VALUE          0x00	
#define HS6200_BANK0_EN_RXADDR_0_VALUE		0x00
#define HS6200_BANK0_SETUP_AW_0_VALUE		0x00
#define HS6200_BANK0_SETUP_RETR_0_VALUE     0x00
#define HS6200_BANK0_RF_CH_0_VALUE          0x00
#define HS6200_BANK0_RF_SETUP_0_VALUE       0x00
#define HS6200_BANK0_STATUS_0_VALUE         0x00
#define HS6200_BANK0_OBSERVE_TX_0_VALUE     0x00
#define HS6200_BANK0_RPD_0_VALUE            0x00
#define HS6200_BANK0_RX_ADDR_P0_0_VALUE     0x0000000000
#define HS6200_BANK0_RX_ADDR_P1_0_VALUE     0x0000000000
#define HS6200_BANK0_RX_ADDR_P2_0_VALUE     0x00
#define HS6200_BANK0_RX_ADDR_P3_0_VALUE     0x00
#define HS6200_BANK0_RX_ADDR_P4_0_VALUE     0x00
#define HS6200_BANK0_RX_ADDR_P5_0_VALUE     0x00
#define HS6200_BANK0_TX_ADDR_0_VALUE        0x0000000000
#define HS6200_BANK0_RX_PW_P0_0_VALUE       0x00
#define HS6200_BANK0_RX_PW_P1_0_VALUE       0x00
#define HS6200_BANK0_RX_PW_P2_0_VALUE       0x00
#define HS6200_BANK0_RX_PW_P3_0_VALUE       0x00
#define HS6200_BANK0_RX_PW_P4_0_VALUE       0x00
#define HS6200_BANK0_RX_PW_P5_0_VALUE       0x00
#define HS6200_BANK0_FIFO_STATUS_0_VALUE    0x00
#define HS6200_BANK0_DYNPD_0_VALUE          0x00
#define HS6200_BANK0_FEATURE_0_VALUE        0x00
#define HS6200_BANK0_SETUP_VALUE_0_VALUE    0x0000
#define HS6200_BANK0_PRE_GURD_0_VALUE       0x00 


//reset bank0 all register
//其他的复位值见《nRF24L01_common.h》文件.
#define HS6200_BANK0_SETUP_VALUE_RESET_VALUE          0x2828
#define HS6200_BANK0_PRE_GURD_RESET_VALUE             0x12 

//reset bank1 all register
#define HS6200_BANK1_RESERVED0_RESET_VALUE            0x00
#define HS6200_BANK1_PLL_CTL0_RESET_VALUE             0x00000000
#define HS6200_BANK1_PLL_CTL1_RESET_VALUE             0x00005A0A
#define HS6200_BANK1_CAL_CTL_RESET_VALUE              0x1800
#define HS6200_BANK1_A_CNT_REG_RESET_VALUE            0x00
#define HS6200_BANK1_B_CNT_REG_RESET_VALUE            0x00
#define HS6200_BANK1_RESERVED1_RESET_VALUE            0x00
#define HS6200_BANK1_STATUS_RESET_VALUE               0x0E
#define HS6200_BANK1_RESERVED2_RESET_VALUE            0x00
#define HS6200_BANK1_CHAN_RESET_VALUE                 0x00000000
#define HS6200_BANK1_IF_FREQ_RESET_VALUE              0x000000
#define HS6200_BANK1_AFC_COR_RESET_VALUE              0x000000
#define HS6200_BANK1_FDEV_RESET_VALUE                 0x00
#define HS6200_BANK1_DAC_RANGE_RESET_VALUE            0x00
#define HS6200_BANK1_DAC_IN_RESET_VALUE               0x00
#define HS6200_BANK1_CTUNING_RESET_VALUE              0x00
#define HS6200_BANK1_FTUNING_RESET_VALUE              0x00
#define HS6200_BANK1_RX_CTRL_RESET_VALUE              0x3809C152
#define HS6200_BANK1_FAGC_CTRL_RESET_VALUE            0x00004000
#define HS6200_BANK1_DAC_CAL_LOW_RESET_VALUE          0x00
#define HS6200_BANK1_DAC_CAL_HI_RESET_VALUE           0x00
#define HS6200_BANK1_RESERVED3_RESET_VALUE            0x00
#define HS6200_BANK1_DOC_DACI_RESET_VALUE             0x00
#define HS6200_BANK1_DOC_DACQ_RESET_VALUE             0x00
#define HS6200_BANK1_AGC_CTRL_RESET_VALUE             0x00099A00
#define HS6200_BANK1_AGC_GAIN_RESET_VALUE             0x9CCBC101
#define HS6200_BANK1_RF_IVGEN_RESET_VALUE             0x22206428
#define HS6200_BANK1_TEST_PKDET_RESET_VALUE           0x2100102A	


#define HS6200_BANK1_ALL_REGISTER_WIDTH               0x3B

//wrire 1 to Bank1 all register

//write 0 to Bank0 all register


//-------------------------------位定义-----------------------------------

#define HS6200_BANK1_PLL_RST_CNT	  BIT5
#define HS6200_BANK1_PLL_RSTn_PFD	  BIT5


/*-----------------------------------1.test point------------------------------------------*/
#define HS6200_BANK1_TEST_PACKET_TEST_EN           BIT7 

#define HS6200_BANK1_TEST_PACKET_TEST_MODE 		   (BIT7+BIT6+BIT5)
#define HS6200_BANK1_TEST_PACKET_TEST_MODE_010     BIT6
#define HS6200_BANK1_TEST_PACKET_TEST_MODE_100     BIT7
#define HS6200_BANK1_TEST_PACKET_TEST_MODE_001     BIT5
 
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1        (BIT4+BIT3+BIT2)
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000   	0x00
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001    BIT2

#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0        (BIT1+BIT0)
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_00		0x00
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_11		(BIT1+BIT0)
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_10		BIT1
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_01		BIT0
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_0x		0x00
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_1x     BIT1
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_xx		0x00


/*HS6200B1新增模拟测试内容 wangs2013-12-25*/
#define HS6200B1_BANK1_TEST_PACKET_TEST_MODE 		   (BIT6+BIT5)
#define HS6200B1_BANK1_TEST_PACKET_TEST_MODE_01       BIT5
#define HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10       BIT6

#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_010    BIT3
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_011    (BIT3+BIT2)
#define HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_100    BIT4

/*END wangs 2013-12-25*/


/*-----------------------------2.HS6200 frequency configuration---------------------------*/
//HS6200 SDM_EN
#define HS6200_BANK1_SDM_EN                 BIT7
#define HS6200_BANK1_SDM_DITH_EN            BIT6  
    
#define HS6200_BANK1_SDM_DITH_IN            BIT7 
#define HS6200_BANK1_SDM_DITH_STAGE_1       0x00
#define HS6200_BANK1_SDM_DITH_STAGE_2 		BIT7
//set frequency offset
#define HS6200_BANK1_FDEV_160KHZ            0x14
#define HS6200_BANK1_FDEV_250KHZ            0x20
#define HS6200_BANK1_FDEV_320KHZ            0x29

#define HS6200_BANK1_PLL_FOFST_SEL          (BIT0+BIT1)
#define HS6200_BANK1_PLL_FOFST_SEL_160KHZ    0x00
#define HS6200_BANK1_PLL_FOFST_SEL_250KHZ	 BIT0
#define HS6200_BANK1_PLL_FOFST_SEL_320KHZ	 BIT1

//data rate
#define HS6200_BANK0_DR         (BIT5+BIT3) 
#define HS6200_BANK0_DR_250K    BIT5  
#define HS6200_BANK0_DR_500K	(BIT5+BIT3)
#define HS6200_BANK0_DR_1M		0x00
#define HS6200_BANK0_DR_2M		BIT3

#define HS6200_BANK1_PLL_TEST_EN      BIT7


/*----------------------------------3.RX gain----------------------------------------*/
#define HS6200_BANK1_RX_GAIN_CONFIG_AGC_GAIN_MN         BIT7 

#define HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN	        (BIT6+BIT5+BIT4+BIT3) 
#define HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN_4dB        BIT3
#define HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN_16dB       BIT4
#define HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN_28dB       BIT5
#define HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN_40dB       BIT6

/*HS6200B1新增模拟测试内容*/
#define HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN	          (BIT4+BIT3) 
#define HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN_4dB        0x00
#define HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN_16dB       BIT3
#define HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN_28dB       BIT4
#define HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN_40dB       (BIT4+BIT3)
//END wangs 2013-12-25


//gain filter
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2       (BIT2+BIT1+BIT0)
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_001   BIT0
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_010   BIT1
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_100   BIT2

#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1        (BIT7+BIT6+BIT5+BIT4+BIT3+BIT2+BIT1+BIT0)
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT7   BIT7
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT6   BIT6
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT5   BIT5
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT4   BIT4
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT3   BIT3
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT2   BIT2
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT1   BIT1
#define HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT0   BIT0

/*----------------------------4.peak detector threshold--------------------------*/
#define HS6200_BNAK1_PKDET_VREF           (BIT7+BIT6+BIT5+BIT4+BIT3+BIT2+BIT1+BIT0)
#define HS6200_BNAK1_PKDET_VREF_BIT7       BIT7
#define HS6200_BNAK1_PKDET_VREF_BIT6       BIT6
#define HS6200_BNAK1_PKDET_VREF_BIT5       BIT5
#define HS6200_BNAK1_PKDET_VREF_BIT4       BIT4
#define HS6200_BNAK1_PKDET_VREF_BIT3       BIT3
#define HS6200_BNAK1_PKDET_VREF_BIT2       BIT2
#define HS6200_BNAK1_PKDET_VREF_BIT1       BIT1   
#define HS6200_BNAK1_PKDET_VREF_BIT0       BIT0

//HS6200B1 新增 wangs 2013-12-26
#define HS6200_BNAK1_PKDET_ACTIVATE_EN        BIT7
#define HS6200_BNAK1_NEW_PKDET_VREFC          (BIT4+BIT3+BIT2)
#define HS6200_BNAK1_NEW_PKDET_VREFC_BIT4     BIT4
#define HS6200_BNAK1_NEW_PKDET_VREFC_BIT3     BIT3
#define HS6200_BNAK1_NEW_PKDET_VREFC_BIT2     BIT2

#define HS6200_BNAK1_NEW_PKDET_VREF2C         (BIT1+BIT0)
#define HS6200_BNAK1_NEW_PKDET_VREF2C_BIT1    BIT1
#define HS6200_BNAK1_NEW_PKDET_VREF2C_BIT0    BIT0
//END wangs 2013-12-26


/*------------------------------5.PA power-----------------------------------*/
#define HS6200_BANK0_PA_POWER              (BIT2+BIT1+BIT0) 
#define HS6200_BANK0_PA_POWER_n18dBm        0x00
#define HS6200_BANK0_PA_POWER_n12dBm        BIT1
#define HS6200_BANK0_PA_POWER_n6dBm        	BIT2
#define HS6200_BANK0_PA_POWER_0dBm        	(BIT2+BIT1)
#define HS6200_BANK0_PA_POWER_5dBm          BIT0  

//HS6200B1 wangs 2013-12-27
#define HS6200B1_BNAK1_PA_POWER_VOLTAGE     BIT7   
#define HS6200B1_BANK0_PA_POWER             (BIT6+BIT2+BIT1+BIT0)
#define HS6200B1_BANK0_PA_POWER_n18dBm      BIT0
#define HS6200B1_BANK0_PA_POWER_n12dBm      BIT1
#define HS6200B1_BANK0_PA_POWER_n6dBm       BIT2
#define HS6200B1_BANK0_PA_POWER_0dBm        BIT6
#define HS6200B1_BANK0_PA_POWER_5dBm        (BIT6+BIT2+BIT1+BIT0)
//END wangs 2013-12-27


/*------------------------6.calibration manual mode-------------------------*/
//ctuing and ftuning
#define HS6200_BANK1_CTUNING_MN_ON             BIT1
#define HS6200_BANK1_FTUNING_MN_ON             BIT0   

//VCO LDO calibration 
#define HS6200_BANK1_VCO_LDO_CAL_MN            BIT7
#define HS6200_BANK1_VCO_LDO_CAL_REG           (BIT0+BIT1+BIT2)
//DAC gain calibration
#define HS6200_BANK1_DAC_GAIN_CAL_MN           BIT5 

//DC offset calibration
#define HS6200_BANK1_DOC_DAC_MN          BIT1 

// carrier calibration
#define HS6200_BANK1_CAL_EN                    BIT2
#define HS6200_BANK1_DAC_CALMODE_REG           BIT5

/*-----------------------------7.Continus carrier transmit-------------------------------*/
//Cont wave
#define HS6200_BANK0_CONT_WAVE                BIT7  
#define HS6200_BANK0_PRX                      BIT0 

//PLL DAC
#define HS6200_BANK1_PLL_DAC_EN               BIT6  
//Bp_Cp_Diox
#define HS6200_BANK1_BP_CP_DIOX               BIT3

//VC_Det switch
#define HS6200_BANK1_VC_DET                   BIT4

//Pll ICP Sel
#define HS6200_BANK1_PLL_ICP_SEL             (BIT1+BIT0) 
#define HS6200_BANK1_PLL_ICP_SEL_80			 0x00
#define HS6200_BANK1_PLL_ICP_SEL_120		 BIT0
#define HS6200_BANK1_PLL_ICP_SEL_160		 BIT1
#define HS6200_BANK1_PLL_ICP_SEL_200		 (BIT1+BIT0)

//PLL VDIV2 Sel
#define HS6200_BANK1_PLL_VDIV2_SEL            (BIT3+BIT2)
#define HS6200_BANK1_PLL_VDIV2_SEL_550mV       0x00
#define HS6200_BANK1_PLL_VDIV2_SEL_600mV       BIT2
#define HS6200_BANK1_PLL_VDIV2_SEL_650mV       BIT3
#define HS6200_BANK1_PLL_VDIV2_SEL_700mV      (BIT3+BIT2)


/*-------------------------------8.continus test pattern transmit-----------------------------*/
#define HS6200_BANK1_TEST_PAT_EN              BIT7

#define HS6200_BANK1_SOFT_RST_EN              BIT7    




//status 寄存器中的Bank指示位
#define STATUS_BANK1          BIT7 
#define STATUS_BANK0          0x00    


#endif  /*HS6200_REG_H*/

/*----------------------------End Of File------------------------------*/
