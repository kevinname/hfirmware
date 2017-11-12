//
// HS6200Test Application Protocol between the Host and USB Device.
// 详情请参阅《HS6200Test 主机与设备间应用协议》
//
#ifndef HS6200TEST_APPLICATION_PROTOCOL_H
#define HS6200TEST_APPLICATION_PROTOCOL_H

// Chip Space
#define HS6200_SPACE          0x00
#define HS6200B1_SPACE        0x30
//#define HS6530_SPACE          0x60



//------------------------------- PID---------------------------------                                                
#define PID_COMM                0x01
#define PID_SETUP               0x02
#define PID_SPI                 0x03
#define PID_SPEC                0x04
#define PID_DATA                0x05
#define PID_ACK                 0x06
#define PID_NAK                 0x07
#define PID_STALL               0x08
#define PID_DIGITAL_TEST        0x09
#define PID_ANALOG_TEST         0x0A 
#define PID_SYSTEM_TEST         0x0B
#define PID_SET					0x0C
#define PID_GET					0x0D
#define PID_DEBUG				0x0E

                                               
// RF Devices                   
#define RF_DEV_0                0x00
#define RF_DEV_1                0x01
                                
// COMM packet: PLT, payload type                     
#define TXPL                    0x00        // <Command(1), Addr_Width(1, [1, 3-5]), address((1, [0-5]), (3-5)), data(1-32)>
#define TXPLNAK                 0x01        // TX Payload NOACK
#define ACKPL                   0x02        // <Command(1), Addr_Width(1, [1]), PipeNum(1, [0-5]), data(1-32)>

#define COMM_INIT               0x03	

#define COMM_PIPE               0x01       /*TXPL和ACKPL 后跟0x01表示后面指的是通道号，如果是0x03，0x04，0x05，则后面跟的是地址*/
#define COMM_ADDR_WIDTH3        0x03
#define COMM_ADDR_WIDTH4        0x04
#define COMM_ADDR_WIDTH5        0x05


// PIPEs
#define PIPE0                   0x00
#define PIPE1                   0x01
#define PIPE2                   0x02
#define PIPE3                   0x03
#define PIPE4                   0x04
#define PIPE5                   0x05


//--------------------------------- SETUP packet: Setting types-----------------------------------                    
// Setup Misc commands
#define SET_POWER_UP               0x01        // <Command(1)>
#define SET_PWOER_DOWN             0x02        // <Command(1)>
#define SET_CRC                    0x03        // <Command(1), Value(1, [0-2])>
#define SET_ARD                    0x04        // <Command(1), Value(1, [0-15])>
#define SET_ARC                    0x05        // <Command(1), Value(1, [0-15])>
#define SET_RF_CH                  0x06        // <Command(1), Value(1, [0-125])>
#define SET_RF_AIR_DATARATE        0x07        // <Command(1), Value(1, [0-3])>
#define SET_RF_OUTPUT_POWER        0x08        // <Command(1), Value(1, [0-4])>
#define SET_INT_MASK_RX_DR         0x09        // <Command(1), Value(1, [TRUE/FALSE])>
#define SET_INT_MASK_TX_DS         0x0a        // <Command(1), Value(1, [TRUE/FALSE])>
#define SET_INT_MASK_MAX_RT        0x0b        // <Command(1), Value(1, [TRUE/FALSE])>
                                   
// Setup Pipe commands             
#define SET_PIPE_ADDR_WIDTH        0x0c        // <Command(1), Value(1, [3,4,5])>
#define SET_PIPE_TX_ADDR           0x0d        // <Command(1), Value(3-5)>
#define SET_PIPE_EN                0x0e        // <Command(1), PipeNum(1, [0-5]), Value(1, [TRUE/FALSE])> 
#define SET_PIPE_AUTO_ACK          0x0f        // <Command(1), PipeNum(1, [0-5]), Value(1, [TRUE/FALSE])>
#define SET_PIPE_DPL               0x10        // <Command(1), PipeNum(1, [0-5]), Value(1, [TRUE/FALSE])>
#define SET_PIPE_ADDR              0x11        // <Command(1), PipeNum(1, [0-5]), Value(3-5)>
#define SET_PIPE_STATIC_PLW        0x12        // <Command(1), PipeNum(1, [0-5]), Value(1, [TRUE/FALSE])>
// Setup Communication Mode commands
#define SET_PTX                    0x13        // <Command(1)>
#define SET_PRX                    0x14
#define SET_SPL                    0x15
#define SET_DPL                    0x16
#define SET_SA                     0x17
#define SET_DA                     0x18
#define SET_NAK                    0x19
#define SET_ACK                    0x1a
#define SET_AE                     0x1b
#define SET_AP                     0x1c
#define SET_NAR                    0x1d
#define SET_AR                     0x1e
#define SET_NMR                    0x1f
#define SET_MR                     0x20
// Setup Predefined Communication Mode command
// COMMODE -- COMMunication MODE
#define SET_COMMODE                0x21        // <Command(1), Value(1, [COMMODE_XXX])>
#define SET_CE_LOW_BEFORE_W_REG    0x22
#define SET_CAL_AFTER_ACK		   0x23

// Predefined Communication Mode Values                  
//#define COMMODE_SPL_SA_NAK_ZZ       0x00 // 静长静无答	
//#define COMMODE_SB                  COMMODE_SPL_SA_NAK_ZZ
//#define COMMODE_SPL_SA_ACK_AE       0x02 // 静长静空答
//#define COMMODE_SPL_DA_ACK_AE       0x06 // 静长动空答
//#define COMMODE_DPL_SA_ACK_AE       0x0a // 动长静空答
//#define COMMODE_DPL_SA_ACK_AP       0x0b // 动长静载答
//#define COMMODE_DPL_DA_ACK_AE       0x0e // 动长动空答
//#define COMMODE_DPL_DA_ACK_AP       0x0f // 动长动载答

// Setup MCU commands
#define SET_MCU_MASK_EX0           0x80        // <Command(1), Value(1, [TRUE/FALSE])>
#define SET_MCU_MASK_EX1           0x81        // <Command(1), Value(1, [TRUE/FALSE])> 


// Values
#define CRC_0                      0x00
#define CRC_1                      0x01
#define CRC_2                      0x02

#define ARD_250us                  0x00
#define ARD_500us                  0x01
#define ARD_750us                  0x02
#define ARD_1000us                 0x03
#define ARD_1250us                 0x04
#define ARD_1500us                 0x05
#define ARD_1750us                 0x06
#define ARD_2000us                 0x07
#define ARD_2250us                 0x08
#define ARD_2500us                 0x09
#define ARD_2750us                 0x0a
#define ARD_3000us                 0x0b
#define ARD_3250us                 0x0c
#define ARD_3500us                 0x0d
#define ARD_3750us                 0x0e
#define ARD_4000us                 0x0f

//#define RF_DR_250Kbps              0x00
//#define RF_DR_500Kbps              0x01
//#define RF_DR_1Mbps                0x02
//#define RF_DR_2Mbps                0x03
//
//#define RF_POWER_n18dBm            0x00
//#define RF_POWER_n12dBm            0x01
//#define RF_POWER_n6dBm             0x02
//#define RF_POWER_0dBm              0x03
//#define RF_POWER_5dBm              0x04

//----------------------------- SPI instruction packet: SPI commands--------------------------------

// #include <nRF24L01_common.h>

//-----------------------------SPEC packet: Specific commands------------------------------------
#define SC_RF_NOP                  0x00
#define SC_TEST_REG_RST_VAL        0x01
#define SC_TEST_REG_RW_CMP         0x02
#define SC_POWERDOWN               0x03
#define SC_POWERUP                 0x04
#define SC_STANDBYI                0x05
#define SC_STANDBYII               0x06
#define SC_CE_HIGH                 0x07
#define SC_CE_HIGH_PULSE           0x08
#define SC_REUSE_TX_PL             0x09
#define SC_FLUSH_TX_FIFO           0x0a
#define SC_FLUSH_RX_FIFO           0x0b
#define SC_W_TX_PAYLOAD            0x0c
#define SC_W_TX_PAYLOAD_NOACK      0x0d
#define SC_W_ACK_PAYLOAD           0x0e
#define SC_ACTIVATE                0x0f
#define SC_RD_PLOS_CNT             0x1b
#define SC_RD_ARC_CNT              0x1c
#define SC_RD_RPD                  0x1d
#define SC_RD_FIFO_STATUS          0x1e
#define SC_RD_CURRENT_CONFIG_WORD  0x1f
#define SC_TEST_PRESSURE           0x20
#define SC_TEST_DATA_COMP_DPL      0x21
#define SC_TEST_DATA_COMP_SPL      0x22
#define SC_TEST_CONT_WAVE_MODE     0x23
#define SC_TEST_SEARCH_DIRTY_RF_CH 0x24
#define SC_TEST_SEARCH_CLEAN_RF_CH 0x25
#define SC_CE_LOW                  0x26




//// SPEC instruction packet: SPEC commands
//#define SC_RST_BANK0_ALL_REG        0xA6
//#define SC_READ_BANK0_ALL_REG       0xA7 
//#define SC_RST_BANK1_ALL_REG	    0xA8 
//#define SC_READ_BANK1_ALL_REG   	0xA9
//#define SC_WRITE1_BANK0_ALL_REG     0xAA
//#define SC_WRITE0_BANK0_ALL_REG     0xAB
//#define SC_WRITE1_BANK1_ALL_REG     0xAC
//#define SC_WRITE0_BANK1_ALL_REG     0xAD



//------------------------------------Digital Test-----------------------------------------
// DT: DIGITAL_TEST Commands:
// L1 commands:
#define DT_SPI_TEST                0x01
#define DT_REG_TEST                0x02
#define DT_MISC_TEST               0x03
#define DT_PIPE_TEST               0x04


// L2 commands:
// 1. SPI: SPI Test
#define DT_SPI_R_REGISTER          0x00
// 2. REG: Register Test
#define DT_REG_READ                0x01
#define DT_REG_READ_ALL            0x02
#define DT_REG_WRITE               0x03
#define DT_REG_WRITE_ALL_0         0x04	           /* BANK1 没有实现 */
#define DT_REG_WRITE_ALL_1         0x05			   /* BANK1 没有实现*/
#define DT_REG_RESET               0x06
#define DT_REG_RST_VAL             0x07
#define DT_REG_R_VS_W              0x08
// 3. MISC: Miscellaneous Test
#define DT_MISC_CE_HIGH            0x01
#define DT_MISC_CE_LOW             0x02
#define DT_MISC_CE_HIGH_PULSE      0x03
#define DT_MISC_SOFT_RESET		   0x04

// -------------------------------------ANALOG_TEST----------------------------
// L1 commands:
#define AT_TEST_PT_CFG             0x01
#define AT_FREQ_CFG                0x02
#define AT_RX_GAIN_CFG             0x03
#define AT_PEAK_DTT_THR            0x04
#define AT_PA_PWR_CFG              0x05
#define AT_CAL_MAN_MODE            0x06
#define AT_CONT_CARRIER_TX         0x07
#define AT_CONT_TESTPAT_TX         0x08

// ----------1. TPC: Test Point Configuration --------------
#define AT_TPC_NONE                0x00
#define AT_TPC_LNA_bias            0x01
#define AT_TPC_Mixer_CM_voltage    0x02
#define AT_TPC_TIA_I_CM_voltage    0x03
#define AT_TPC_TIA_Q_CM_voltage    0x04
#define AT_TPC_TIA_output          0x05
#define AT_TPC_1_2v_LDO_of_LO      0x06
#define AT_TPC_1_3v_LDO_of_LO      0x07
#define AT_TPC_DIV2_CM_voltage     0x08
#define AT_TPC_LO_control_voltage  0x09
#define AT_TPC_FBCLK_of_LO         0x0A
#define AT_TPC_Crystal_output      0x0B
#define AT_TPC_Filter_output       0x0C

/*HS6200B1新增测试内容 wangs 2013-12-25*/
/* HS6200B1 */
#define HS6200B1_AT_TPC_NONE               (HS6200B1_SPACE + 0x00)
#define HS6200B1_AT_TPC_Fbclk_b            (HS6200B1_SPACE + 0x01)
#define HS6200B1_AT_TPC_Crystal_clock      (HS6200B1_SPACE + 0x02)
#define HS6200B1_AT_TPC_Old_pkdet_ip	   (HS6200B1_SPACE + 0x03)
#define HS6200B1_AT_TPC_Old_pkdet_qp	   (HS6200B1_SPACE + 0x04)
#define HS6200B1_AT_TPC_New_pkdet_1		   (HS6200B1_SPACE + 0x05)
#define HS6200B1_AT_TPC_New_pkdet_0		   (HS6200B1_SPACE + 0x06)
#define HS6200B1_AT_TPC_Filter_ip		   (HS6200B1_SPACE + 0x07)
#define HS6200B1_AT_TPC_Filter_in		   (HS6200B1_SPACE + 0x08)
#define HS6200B1_AT_TPC_Filter_qp		   (HS6200B1_SPACE + 0x09)
#define HS6200B1_AT_TPC_Filter_qn		   (HS6200B1_SPACE + 0x0A)
#define HS6200B1_AT_TPC_Mixer_ip		   (HS6200B1_SPACE + 0x0B)
#define HS6200B1_AT_TPC_Mixer_in		   (HS6200B1_SPACE + 0x0C)
#define HS6200B1_AT_TPC_Mixer_qp		   (HS6200B1_SPACE + 0x0D)
#define HS6200B1_AT_TPC_Mixer_qn		   (HS6200B1_SPACE + 0x0E)
#define HS6200B1_AT_TPC_VCO_LDO			   (HS6200B1_SPACE + 0x0F)
#define HS6200B1_AT_TPC_LO_BUF_LDO		   (HS6200B1_SPACE + 0x10)
#define HS6200B1_AT_TPC_Bandgap 		   (HS6200B1_SPACE + 0x11)
#define HS6200B1_AT_TPC_Vctrl			   (HS6200B1_SPACE + 0x12)
#define HS6200B1_AT_TPC_LNA_bias		   (HS6200B1_SPACE + 0x13)
#define HS6200B1_AT_TPC_Mixer_bias		   (HS6200B1_SPACE + 0x14)
#define HS6200B1_AT_TPC_Mixer_LDO		   (HS6200B1_SPACE + 0x15)
#define HS6200B1_AT_TPC_LNA_LDO			   (HS6200B1_SPACE + 0x16)
#define HS6200B1_AT_TPC_Filter_LDO		   (HS6200B1_SPACE + 0x17)
#define HS6200B1_AT_TPC_XTAL_BUF_LDO	   (HS6200B1_SPACE + 0x18)
/*END wangs 2013-12-25*/

// -----------2. FC: Frequency Configuration--------------
#define AT_FC_SET_FREQ             0x01
#define AT_FC_SDM_ON               0x02
#define AT_FC_SDM_OFF              0x03
#define AT_FC_DITHER_ON            0x04
#define AT_FC_DITHER_OFF           0x05
#define AT_FC_DITHER_STAGE_1       0x06
#define AT_FC_DITHER_STAGE_2       0x07
#define AT_FC_FO_160K              0x08
#define AT_FC_FO_250K              0x09
#define AT_FC_FO_320K              0x0A
#define AT_FC_RF_DR_250K           0x0B
#define AT_FC_RF_DR_500K           0x0C
#define AT_FC_RF_DR_1M             0x0D
#define AT_FC_RF_DR_2M             0x0E
#define AT_FC_IF                   0x0F
#define AT_FC_AB_CNT_REG           0x10
#define AT_FC_PLL_TEST_EN_ON       0x11
#define AT_FC_PLL_TEST_EN_OFF      0x12


//----------3. RXGC: RX Gain Configuration---------------
#define AT_RXGC_LNA_GAIN_4dB       0x01
#define AT_RXGC_LNA_GAIN_16dB      0x02
#define AT_RXGC_LNA_GAIN_28dB      0x03
#define AT_RXGC_LNA_GAIN_40dB      0x04
                                   
#define AT_RXGC_FILTER_GAIN_6dB    0x05
#define AT_RXGC_FILTER_GAIN_8dB    0x06
#define AT_RXGC_FILTER_GAIN_10dB   0x07    
#define AT_RXGC_FILTER_GAIN_12dB   0x08
#define AT_RXGC_FILTER_GAIN_14dB   0x09
#define AT_RXGC_FILTER_GAIN_16dB   0x0A
#define AT_RXGC_FILTER_GAIN_18dB   0x0B    
#define AT_RXGC_FILTER_GAIN_20dB   0x0C
#define AT_RXGC_FILTER_GAIN_22dB   0x0D
#define AT_RXGC_FILTER_GAIN_24dB   0x0E
#define AT_RXGC_FILTER_GAIN_26dB   0x0F
#define AT_RXGC_FILTER_GAIN_28dB   0x10
#define AT_RXGC_FILTER_GAIN_30dB   0x11
#define AT_RXGC_FILTER_GAIN_32dB   0x12
#define AT_RXGC_FILTER_GAIN_34dB   0x13    
#define AT_RXGC_FILTER_GAIN_36dB   0x14
#define AT_RXGC_FILTER_GAIN_38dB   0x15
#define AT_RXGC_FILTER_GAIN_40dB   0x16
#define AT_RXGC_FILTER_GAIN_42dB   0x17    
#define AT_RXGC_FILTER_GAIN_44dB   0x18
#define AT_RXGC_FILTER_GAIN_46dB   0x19

#define AT_RXGC_AGC_GAIN_MN_ON     0x1A
#define AT_RXGC_AGC_GAIN_MN_OFF    0x1B

/* HS6200B1 */
#define HS6200B1_AT_RXGC_LNA_GAIN_4dB       (HS6200B1_SPACE + 0x01)
#define HS6200B1_AT_RXGC_LNA_GAIN_16dB      (HS6200B1_SPACE + 0x02)
#define HS6200B1_AT_RXGC_LNA_GAIN_28dB      (HS6200B1_SPACE + 0x03)
#define HS6200B1_AT_RXGC_LNA_GAIN_40dB      (HS6200B1_SPACE + 0x04)

//------------4. PDT: Peak Detector Threshold--------------
#define AT_PDT_50mV                0x01
#define AT_PDT_100mV               0x02
#define AT_PDT_150mV               0x03
#define AT_PDT_200mV               0x04
#define AT_PDT_250mV               0x05
#define AT_PDT_300mV               0x06
#define AT_PDT_350mV               0x07
#define AT_PDT_400mV               0x08

/* HS6200B1 */
#define HS6200B1_AT_PDT_AGC_SEL_OLD  (HS6200B1_SPACE + 0x00)
#define HS6200B1_AT_PDT_AGC_SEL_NEW  (HS6200B1_SPACE + 0x01)
#define HS6200B1_AT_PDT_VH_VL        (HS6200B1_SPACE + 0x02) // followed by a byte of VH_VL value, in which VH occupies the high half byte and VL occupies the low half byte.
#define AT_PDT_VH0                 0x00
#define AT_PDT_VH1                 0x01
#define AT_PDT_VH2                 0x02
#define AT_PDT_VH3                 0x03
#define AT_PDT_VH4                 0x04

#define AT_PDT_VL0                 0x00
#define AT_PDT_VL1                 0x01
#define AT_PDT_VL2                 0x02


//-------------5. PAPC: PA Power Configuration-------------
#define AT_PAPC_n18dBm             0x01
#define AT_PAPC_n12dBm             0x02
#define AT_PAPC_n6dBm              0x03
#define AT_PAPC_0dBm               0x04
#define AT_PAPC_5dBm               0x05
#define AT_PAPC_R                  0x06
/* HS6200B1 */
#define HS6200B1_AT_PAPC_n18dBm          (HS6200B1_SPACE + 0x01)
#define HS6200B1_AT_PAPC_n12dBm          (HS6200B1_SPACE + 0x02)
#define HS6200B1_AT_PAPC_n6dBm           (HS6200B1_SPACE + 0x03)
#define HS6200B1_AT_PAPC_0dBm            (HS6200B1_SPACE + 0x04)
#define HS6200B1_AT_PAPC_5dBm            (HS6200B1_SPACE + 0x05)
#define HS6200B1_AT_PAPC_R               (HS6200B1_SPACE + 0x06)
#define HS6200B1_AT_PAPC_PA_VOLTAGE_1V8  (HS6200B1_SPACE + 0x07)
#define HS6200B1_AT_PAPC_PA_VOLTAGE_3V   (HS6200B1_SPACE + 0x08)

// ------------6. CMM: Calibration Manual Mode ------------
#define AT_CMM_CTUNING_MN_ON       0x01
#define AT_CMM_CTUNING_MN_OFF      0x02
#define AT_CMM_FTUNING_MN_ON       0x03
#define AT_CMM_FTUNING_MN_OFF      0x04
#define AT_CMM_VCO_LDO_CAL_MN_ON   0x05
#define AT_CMM_VCO_LDO_CAL_MN_OFF  0x06
#define AT_CMM_DAC_RANGE_MN_ON     0x07
#define AT_CMM_DAC_RANGE_MN_OFF    0x08
#define AT_CMM_DOC_DAC_MN_ON       0x09
#define AT_CMM_DOC_DAC_MN_OFF      0x0A
#define AT_CMM_CARRIER_CAL_MN      0x0B
#define AT_CMM_CTUNING_REG_R       0x0C
#define AT_CMM_CTUNING_REG_W       0x0D
#define AT_CMM_FTUNING_REG_R       0x0E
#define AT_CMM_FTUNING_REG_W       0x0F
#define AT_CMM_VCO_LDO_CAL_REG_R   0x10
#define AT_CMM_VCO_LDO_CAL_REG_W   0x11
#define AT_CMM_DAC_RANGE_R         0x12
#define AT_CMM_DAC_RANGE_W         0x13
#define AT_CMM_DOC_DACI_R          0x14
#define AT_CMM_DOC_DACI_W          0x15
#define AT_CMM_DOC_DACQ_R          0x16
#define AT_CMM_DOC_DACQ_W          0x17
// -----------7. CCT: Continuous Carrier Transmit----------
#define AT_CCT_CONT_WAVE_START     0x01
#define AT_CCT_CONT_WAVE_STOP      0x02
#define AT_CCT_PLL_DAC_ON          0x03
#define AT_CCT_PLL_DAC_OFF         0x04
#define AT_CCT_BP_CP_DIO_ON        0x05
#define AT_CCT_BP_CP_DIO_OFF       0x06
#define AT_CCT_VC_DET_ON           0x07
#define AT_CCT_VC_DET_OFF          0x08

#define AT_CCT_PLL_ICP_SEL_80uA    0x09
#define AT_CCT_PLL_ICP_SEL_120uA   0x0A
#define AT_CCT_PLL_ICP_SEL_160uA   0x0B
#define AT_CCT_PLL_ICP_SEL_200uA   0x0C

#define AT_CCT_PLL_VDIV2_SEL_550mV 0x0D
#define AT_CCT_PLL_VDIV2_SEL_600mV 0x0E
#define AT_CCT_PLL_VDIV2_SEL_650mV 0x0F
#define AT_CCT_PLL_VDIV2_SEL_700mV 0x10

//---------8. CTT: Continuous TestPattern Transmit---------
#define AT_CTT_TESTPAT_START       0x01
#define AT_CTT_TESTPAT_STOP        0x02
#define AT_CTT_TESTPAT             0x03
#define AT_CTT_TESTPAT_PN          0x04


// -------------------------------------------ST: SYSTEM_TEST-----------------------------------------------
// L1 Commands:
#define ST_XXX                     0x01
//--------------------PID_SET-------------------------
//--------------------PID_GET-------------------------
//--------------------PID_DEBUG-----------------------
#define TYPE_REG						0x01
#define TYPE_PLD						0x02
#define TYPE_SDS						0x03
#define TYPE_CMD						0x04
#define TYPE_ERR						0x05
#define TYPE_DBG						0x06


//--------------------Self-Defined Struct-------------------------
// ?????: ????????? [0x18 : 0x1A]
//#define ACK_PLD					   0x18
//#define TX_PLD					   0x19
//#define RX_PLD					   0x1A


// ????? (SDS: Self-Defined Struct)
// TYPE_SDS
#define SDS_CONFIG					0x01

typedef struct
{
	unsigned char config;		// mask_rx_dr, mask_tx_ds, mask_max_rt, crc0/1/2, power_up/down, prx/ptx.
	unsigned char setup_retr;	// ard, arc.
	unsigned char rf_ch;
	unsigned char rf_setup;		//cont_wave, pll_lock, rf_air_datarate, rf_output_power
	unsigned char feature;		// en_dpl, en_ack_pay, en_dyn_ack
	
	unsigned char other0;		// ce_low_before_w_reg<bit7>, cal_after_ack<bit6>, flush_tx_when_max_rt<bit5>, ce_high<bit4>, transmit_by_ce_high<bit3>, nop_after_w_tx_payload<bit2>
	unsigned char other1;		// mcu_mask_ex1<bit7>, mcu_mask_ex0<bit6>

	// pipe
	unsigned char setup_aw;
	unsigned char tx_addr[5];

	unsigned char en_rxaddr;
	unsigned char en_aa;
	unsigned char dynpd;
	unsigned char rx_addr_p0[5];
	unsigned char rx_addr_p1[5];
	unsigned char rx_addr_p2;
	unsigned char rx_addr_p3;
	unsigned char rx_addr_p4;
	unsigned char rx_addr_p5;
	unsigned char rx_pw_p0;
	unsigned char rx_pw_p1;
	unsigned char rx_pw_p2;
	unsigned char rx_pw_p3;
	unsigned char rx_pw_p4;
	unsigned char rx_pw_p5;

} SDS_CONFIG_t;

#define SDS_MCU						0x02
#define SDS_FIRMWARE				0x03
#define SDS_FLUSH_TX_WHEN_MAX_RT	0x04	// H2D, D2H: <PID_XXX(1), pd(1,[0,1]), pcl(1), TYPE_SDS(1), SDS_FLUSH_TX_WHEN_MAX_RT(1), SDS_FLUSH_TX_WHEN_MAX_RT_t(1, [ON/OFF])>
// SDS_TRANSMIT_BY_CE_HIGH
#define SDS_TRANSMIT_BY_CE_HIGH		0x05	// H2D, D2H: <PID_XXX(1), pd(1,[0,1]), pcl(1), TYPE_SDS(1), SDS_TRANSMIT_BY_CE_HIGH(1), SDS_TRANSMIT_BY_CE_HIGH_t(1, [ON/OFF])>

// SDS_NOP_AFTER_W_TX_PAYLOAD
#define SDS_NOP_AFTER_W_TX_PAYLOAD	0x06	// H2D, D2H: <PID_XXX(1), pd(1,[0,1]), pcl(1), TYPE_SDS(1), SDS_NOP_AFTER_W_TX_PAYLOAD(1), SDS_NOP_AFTER_W_TX_PAYLOAD_t(1, [ON/OFF])>

/*
SDS_R_RX_PL_WID
Used for device to return data when the host initates a SPI R_RX_PL_WID instruction.
*/
#define SDS_R_RX_PL_WID				0x07	// D2H only: <PID_ACK(1), pd(1,[0,1]), pcl(1), TYPE_SDS(1), SDS_R_RX_PL_WID(1), SDS_R_RX_PL_WID_t(1, [0-32])>

/*
SDS_DIAGNOSTIC
Used for device to return helpful diagnostic information of the RF device.
Now, 2013.6.27, it is returned only when the rf received pld.
Returned:
1) on dedicated read-command;
2) after written tx or ack payload into tx fifo
3) after received rx payload
*/
#define SDS_DIAGNOSTIC				0x08	// D2H only: <PID_ACK(1), pd(1,[0,1]), pcl(1), TYPE_SDS(1), SDS_DIAGNOSTIC(1), SDS_DIAGNOSTIC_t(5)>


// TYPE_CMD
#define CMD_MANUAL_RETR			  0x01	// <PID_SET(1), pd(1,[0,1]), pcl(1), TYPE_CMD(1), CMD_MANUAL_RETR(1)>
#define CMD_RF_RESET			  0x02	// <PID_SET(1), pd(1,[0,1]), pcl(1), TYPE_CMD(1), CMD_RESET_RF(1)>


/*
SDS_R_RX_PL_WID
Used for device to return data when the host initates a SPI R_RX_PL_WID instruction.
*/

/*
SDS_DIAGNOSTIC
Used for device to return helpful diagnostic information of the RF device.
*/
typedef struct
{
	unsigned char status;
	unsigned char fifo_status;
	unsigned char observe_tx;
	unsigned char rpd;
	unsigned char rx_payload_width;
} SDS_DIAGNOSTIC_t;


//
// 包结构
//
#define MAX_OUT_PACKET_LENGTH   64
#define MAX_IN_PACKET_LENGTH    64*4     /*最大可取4096*/   
#define MAX_DATA_LENGTH         32

// 通信包结构：{PID_COMM, PD, PCL, PC} 
typedef struct
{
    unsigned char pid;                  // packet id
    unsigned char pd;                   // packet device
    unsigned char pcl;                  // packet content lenth
    unsigned char cmd;                  // command
    unsigned char pipe;                 // RF pipe
    unsigned char dat[MAX_DATA_LENGTH]; // data to be transmitted
} pkt_comm_t;

// 设置包结构：{PID_SETUP, PD, PCL, PC}
typedef struct
{
    unsigned char pid;
    unsigned char pd;
    unsigned char pcl;
    unsigned char cmd;
    unsigned char dat[MAX_DATA_LENGTH];
} pkt_setup_t;

// SPI 包结构：{PID_SPI, PD, PCL, PC}
typedef struct
{
    unsigned char pid;
    unsigned char pd;
    unsigned char pcl;
    unsigned char cmd;
    unsigned char dat[MAX_DATA_LENGTH];
} pkt_spi_t;

// 特定命令包结构：{PID_SPI, PD, PCL, PC}
typedef struct
{
    unsigned char pid;
    unsigned char pd;
    unsigned char pcl;
    unsigned char cmd;
    unsigned char dat[MAX_DATA_LENGTH];
} pkt_spec_t;




#endif        //  #ifndef HS6200TEST_APPLICATION_PROTOCOL_H


 /*------------------------------------End Of File--------------------------*/

