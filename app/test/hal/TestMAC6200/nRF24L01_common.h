#ifndef __nRF24L01_COMMON_H__
#define __nRF24L01_COMMON_H__


/**************************************************
                 nRF24L01 内部资源
**************************************************/

/**************************************************
                 nRF24L01 时序
**************************************************/
// nRF24LE1 specific
#define nRF24LE1_Tpd2stby_XCLK 	            150           // us, the time of Power Donw mode to Standby mode, with external clock, for nRF24LE1
#define nRF24LE1_Tpd2stby_XOSC 	            4.5           // ms, the time of Power Donw mode to Standby mode, with external crystal, for nRF24LE1
// endspecific
#define nRF24L01_Tpd2stby                   4.5          // ms, the time of Power Donw mode to Standby mode
#define nRF24L01_Tstby2a                    130          // us, the time of Standby modes to TX/RX mode
#define nRF24L01_Thce                       10           // us, the time of Minimum CE high
#define nRF24L01_Tpece2csn                  4            // us, the time from CE positive edge to CSN low
#define nRF24L01_Tdelay_AGC                 40           // us, the time of AGC
/**************************************************
                 nRF24L01 指令
**************************************************/
#define nRF24L01_R_REGISTER                 0x00 // Define read command to register
#define nRF24L01_W_REGISTER                 0x20 // Define write command to register
#define nRF24L01_R_RX_PAYLOAD               0x61 // Define RX payload register address
#define nRF24L01_W_TX_PAYLOAD               0xA0 // Define TX payload register address
#define nRF24L01_FLUSH_TX                   0xE1 // Define flush TX register command
#define nRF24L01_FLUSH_RX                   0xE2 // Define flush RX register command
#define nRF24L01_REUSE_TX_PL                0xE3 // Define reuse TX payload register command
#define nRF24L01_ACTIVATE                   0x50 // Define ACTIVATE features register command
#define nRF24L01_R_RX_PL_WID                0x60 // Define read RX payload width register command
#define nRF24L01_W_ACK_PAYLOAD              0xA8 // Define write ACK payload register command
#define nRF24L01_W_TX_PAYLOAD_NOACK         0xB0 // Define disable TX ACK for one time register command
#define nRF24L01_NOP                        0xFF // Define No Operation, might be used to read status register
/**************************************************
               nRF24L01 寄存器地址
**************************************************/               
#define nRF24L01_CONFIG                     0x00 // 'Config' register address
#define nRF24L01_EN_AA                      0x01 // 'Enable Auto Acknowledgment' register address
#define nRF24L01_EN_RXADDR                  0x02 // 'Enabled RX addresses' register address
#define nRF24L01_SETUP_AW                   0x03 // 'Setup address width' register address
#define nRF24L01_SETUP_RETR                 0x04 // 'Setup Auto. Retrans' register address
#define nRF24L01_RF_CH                      0x05 // 'RF channel' register address
#define nRF24L01_RF_SETUP                   0x06 // 'RF setup' register address
#define nRF24L01_STATUS                     0x07 // 'Status' register address
#define nRF24L01_OBSERVE_TX                 0x08 // 'Observe TX' register address
#define nRF24L01_RPD                        0x09 // 'Received Power Detector' register address
#define nRF24L01_RX_ADDR_P0                 0x0A // 'RX address pipe0' register address
#define nRF24L01_RX_ADDR_P1                 0x0B // 'RX address pipe1' register address
#define nRF24L01_RX_ADDR_P2                 0x0C // 'RX address pipe2' register address
#define nRF24L01_RX_ADDR_P3                 0x0D // 'RX address pipe3' register address
#define nRF24L01_RX_ADDR_P4                 0x0E // 'RX address pipe4' register address
#define nRF24L01_RX_ADDR_P5                 0x0F // 'RX address pipe5' register address
#define nRF24L01_TX_ADDR                    0x10 // 'TX address' register address
#define nRF24L01_RX_PW_P0                   0x11 // 'RX payload width, pipe0' register address
#define nRF24L01_RX_PW_P1                   0x12 // 'RX payload width, pipe1' register address
#define nRF24L01_RX_PW_P2                   0x13 // 'RX payload width, pipe2' register address
#define nRF24L01_RX_PW_P3                   0x14 // 'RX payload width, pipe3' register address
#define nRF24L01_RX_PW_P4                   0x15 // 'RX payload width, pipe4' register address
#define nRF24L01_RX_PW_P5                   0x16 // 'RX payload width, pipe5' register address
#define nRF24L01_FIFO_STATUS                0x17 // 'FIFO Status Register' register address
#define nRF24L01_DYNPD                      0x1C // 'Enable dynamic payload length' register address
#define nRF24L01_FEATURE                    0x1D // 'Feature' register address

//----------------------------------------------------
// HS6200 specific
#define HS6200_SETUP_TIME                   0x1E

/**************************************************
               nRF24L01 寄存器复位值
**************************************************/               
#define nRF24L01_CONFIG_RESET_VALUE         0x08
#define nRF24L01_EN_AA_RESET_VALUE          0x3f
#define nRF24L01_EN_RXADDR_RESET_VALUE      0x03
#define nRF24L01_SETUP_AW_RESET_VALUE       0x03
#define nRF24L01_SETUP_RETR_RESET_VALUE     0x03
#define nRF24L01_RF_CH_RESET_VALUE          0x02
#define nRF24L01_RF_SETUP_RESET_VALUE       0x0e // or 0x0f
#define nRF24L01_STATUS_RESET_VALUE         0x0e
#define nRF24L01_OBSERVE_TX_RESET_VALUE     0x00
#define nRF24L01_RPD_RESET_VALUE            0x00
#define nRF24L01_RX_ADDR_P0_RESET_VALUE     0xE7E7E7E7E7
#define nRF24L01_RX_ADDR_P0_RESET_VALUE_H4  0xE7E7E7E7
#define nRF24L01_RX_ADDR_P0_RESET_VALUE_L1  0xE7
#define nRF24L01_RX_ADDR_P1_RESET_VALUE     0xC2C2C2C2C2
#define nRF24L01_RX_ADDR_P1_RESET_VALUE_H4  0xC2C2C2C2
#define nRF24L01_RX_ADDR_P1_RESET_VALUE_L1  0xC2
#define nRF24L01_RX_ADDR_P2_RESET_VALUE     0xC3
#define nRF24L01_RX_ADDR_P3_RESET_VALUE     0xC4
#define nRF24L01_RX_ADDR_P4_RESET_VALUE     0xC5
#define nRF24L01_RX_ADDR_P5_RESET_VALUE     0xC6
#define nRF24L01_TX_ADDR_RESET_VALUE        0xE7E7E7E7E7
#define nRF24L01_TX_ADDR_RESET_VALUE_H4     0xE7E7E7E7
#define nRF24L01_TX_ADDR_RESET_VALUE_L1     0xE7
#define nRF24L01_RX_PW_P0_RESET_VALUE       0x00
#define nRF24L01_RX_PW_P1_RESET_VALUE       0x00
#define nRF24L01_RX_PW_P2_RESET_VALUE       0x00
#define nRF24L01_RX_PW_P3_RESET_VALUE       0x00
#define nRF24L01_RX_PW_P4_RESET_VALUE       0x00
#define nRF24L01_RX_PW_P5_RESET_VALUE       0x00
#define nRF24L01_FIFO_STATUS_RESET_VALUE    0x11
#define nRF24L01_DYNPD_RESET_VALUE          0x00
#define nRF24L01_FEATURE_RESET_VALUE        0x00
/**************************************************
                 nRF24L01 寄存器位
**************************************************/
// CONFIG
#define nRF24L01_MASK_RX_DR                 0x40
#define nRF24L01_MASK_TX_DS                 0x20
#define nRF24L01_MASK_MAX_RT                0x10
#define nRF24L01_EN_CRC                     0x08
#define nRF24L01_CRCO                       0x04
#define nRF24L01_CRCO_1_BYTES               0x00
#define nRF24L01_CRCO_2_BYTES               0x04
#define nRF24L01_PWR_UP_MASK                0x02
#define nRF24L01_PWR_UP_CODE                0x02
#define nRF24L01_PWR_DWN_CODE               0x00
#define nRF24L01_PWR_UP                     0x02
#define nRF24L01_PWR_DWN                    0x00
#define nRF24L01_PRIM_RX                    0x01
#define nRF24L01_PRX                        0x01
#define nRF24L01_PTX                        0x00
// EN_AA                                    
#define nRF24L01_EN_AA_P5                   0x20
#define nRF24L01_EN_AA_P4                   0x10
#define nRF24L01_EN_AA_P3                   0x08
#define nRF24L01_EN_AA_P2                   0x04
#define nRF24L01_EN_AA_P1                   0x02
#define nRF24L01_EN_AA_P0                   0x01
// EN_RXADDR                                
#define nRF24L01_ERX_P5                     0x20
#define nRF24L01_ERX_P4                     0x10
#define nRF24L01_ERX_P3                     0x08
#define nRF24L01_ERX_P2                     0x04
#define nRF24L01_ERX_P1                     0x02
#define nRF24L01_ERX_P0                     0x01
// SETUP_AW                                 
#define nRF24L01_AW_BITS                    0x03
#define nRF24L01_AW_3_BYTES                 0x01
#define nRF24L01_AW_4_BYTES                 0x02
#define nRF24L01_AW_5_BYTES                 0x03
// SETUP_RETR: ARD | ARC                    
#define nRF24L01_ARD_BITS                   0xF0
#define nRF24L01_ARD_250_US                 0x00
#define nRF24L01_ARD_500_US                 0x01
#define nRF24L01_ARD_750_US                 0x02
#define nRF24L01_ARD_1000_US                0x03
#define nRF24L01_ARD_1250_US                0x04
#define nRF24L01_ARD_1500_US                0x05
#define nRF24L01_ARD_1750_US                0x06
#define nRF24L01_ARD_2000_US                0x07
#define nRF24L01_ARD_2250_US                0x08
#define nRF24L01_ARD_2500_US                0x09
#define nRF24L01_ARD_2750_US                0x0A
#define nRF24L01_ARD_3000_US                0x0B
#define nRF24L01_ARD_3250_US                0x0C
#define nRF24L01_ARD_3500_US                0x0D
#define nRF24L01_ARD_3750_US                0x0E
#define nRF24L01_ARD_4000_US                0x0F
#define nRF24L01_ARC_BITS                   0x0F
#define nRF24L01_AUTORETR_DISABLED          0x00
#define nRF24L01_ARC_00                     0x00
#define nRF24L01_ARC_01                     0x01
#define nRF24L01_ARC_02                     0x02
#define nRF24L01_ARC_03                     0x03
#define nRF24L01_ARC_04                     0x04
#define nRF24L01_ARC_05                     0x05
#define nRF24L01_ARC_06                     0x06
#define nRF24L01_ARC_07                     0x07
#define nRF24L01_ARC_08                     0x08
#define nRF24L01_ARC_09                     0x09
#define nRF24L01_ARC_10                     0x0A
#define nRF24L01_ARC_11                     0x0B
#define nRF24L01_ARC_12                     0x0C
#define nRF24L01_ARC_13                     0x0D
#define nRF24L01_ARC_14                     0x0E
#define nRF24L01_ARC_15                     0x0F
// RF_CH: 0~125, F0=2400+RF_CH[MHz]=2.400GHz~2.525GHz.
#define nRF24L01_RF_CH_BITS                 0x7F
                                            
// RF_SETUP                                 
#define nRF24L01_CONT_WAVE                  0x80
#define nRF24L01_RF_DR_LOW                  0x20
#define nRF24L01_RF_DR_HIGH                 0x08
#define nRF24L01_RF_DR_250_Kbps             0x20
#define nRF24L01_RF_DR_1_Mbps               0x00
#define nRF24L01_RF_DR_2_Mbps               0x08
#define nRF24L01_PLL_LOCK                   0x10
#define nRF24L01_RF_PWR_n18_dBm             0x00
#define nRF24L01_RF_PWR_n12_dBm             0x02
#define nRF24L01_RF_PWR_n6_dBm              0x04
#define nRF24L01_RF_PWR_0_dBm               0x06
#define nRF24L01_LNA_HCURR                  0x01
                                            
// STATUS                                   
#define nRF24L01_RX_DR                      0x40
#define nRF24L01_TX_DS                      0x20
#define nRF24L01_MAX_RT                     0x10
#define nRF24L01_RX_P_NO                    0x0E
#define nRF24L01_RX_FIFO_EMPTY              0x0E
#define nRF24L01_TX_FIFO_FULL               0x01 // 与 FIFO_STATUS 中 TX_FULL 标志位重复
                                                 
// OBSERVE_TX                                    
#define nRF24L01_PLOS_CNT                   0xF0 
#define nRF24L01_ARC_CNT                    0x0F 
// PRD                                           
#define nRF24L01_RPD_BITS                   0x01 
// RX_PW_PX                                      
#define nRF24L01_RX_PW_BITS                 0x3F 
                                                 
// FIFO_STATUS                                   
#define nRF24L01_FIFO_STATUS_TX_REUSE       0x40 
#define nRF24L01_FIFO_STATUS_TX_FULL        0x20 // 与 STATUS 中 TX_FULL 标志位重复
#define nRF24L01_FIFO_STATUS_TX_EMPTY       0x10 
#define nRF24L01_FIFO_STATUS_RX_FULL        0x02 
#define nRF24L01_FIFO_STATUS_RX_EMPTY       0x01 
                                                 
// DYNPD                                         
#define nRF24L01_DPL_P5                     0x20 // 启用动长通道
#define nRF24L01_DPL_P4                     0x10 // 启用动长通道
#define nRF24L01_DPL_P3                     0x08 // 启用动长通道
#define nRF24L01_DPL_P2                     0x04 // 启用动长通道
#define nRF24L01_DPL_P1                     0x02 // 启用动长通道
#define nRF24L01_DPL_P0                     0x01 // 启用动长通道
                                                 
// FEATURE                                       
#define nRF24L01_EN_DPL                     0x04 // 启用动态载长
#define nRF24L01_EN_DPL_MASK                0x04 
#define nRF24L01_EN_DPL_CODE                0x04 
                                                 
#define nRF24L01_EN_ACK_PAY                 0x02 // 启用有载应答
#define nRF24L01_EN_DYN_ACK                 0x01 // 启用动态应答
/**************************************************
                 nRF24L01 数据结构
**************************************************/
typedef struct
{
  unsigned long h4;
  unsigned char l1;
} nRF24L01_PipeAddr_tag;

/**************************************************/
#endif // __nRF24L01_COMMON_H__