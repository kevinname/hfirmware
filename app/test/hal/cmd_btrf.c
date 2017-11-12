#include "lib.h"


#if (HAL_USE_BTRF == TRUE)

bool m_rxena_spi_mode = true; /* false is gio mode */

#if defined(TEST_HAL_ENABLE)
typedef enum
{
       DUT_DISABLED            = 0,     /* Testmode Disabled [Default]      */
       DUT_ENABLED             = 1,     /* HCI_ENABLE_DEVICE_UNDER_TEST_MODE*/
       DUT_ACTIVE              = 2,     /* Testmode Activated               */
       DUT_ACTIVE_LOOPBACK     = 3,     /* Testmode Loopback Test           */
       DUT_ACTIVE_TXTEST       = 4,     /* Testmode Transmitter Test        */
       DUT_STANDALONE_TXTEST   = 5,     /* Parthus Tx Radio Test Mode       */
       DUT_STANDALONE_RXTEST   = 6,     /* Parthus Rx Radio Test Mode       */
       DUT_TESTMODE_TESTER     = 7      /* Parthus Testmode Tester          */
} t_dut_mode;
extern t_dut_mode BTtst_dut_mode;
typedef enum t_scanEnable
{
    NO_SCANS_ENABLED            = 0x00, /* No scanning enabled */
    INQUIRY_SCAN_ONLY_ENABLED   = 0x01, /* Only inquiry scan enabled */
    PAGE_SCAN_ONLY_ENABLED      = 0x02, /* Only page scan enabled */
    BOTH_SCANS_ENABLED          = 0x03  /* Both page scan and inquiry scan enabled */
} t_scanEnable;
typedef uint8_t t_error;
extern t_error LMscan_Write_Scan_Enable(t_scanEnable scanEnable);
typedef uint32_t t_classDevice;          /* class of device */
typedef struct
{
   /*
    * u_int8 lap_byte0, lap_byte1, lap_byte2, uap, nap_byte0, nap_byte 1
    */
	uint8_t bytes[6];
} t_bd_addr;
typedef struct
{
    t_classDevice class_of_device;
    t_classDevice class_of_device_mask;
    t_bd_addr bd_addr;
    uint8_t filter_condition_type;
    uint8_t auto_accept;

} t_filter;
extern t_error LMfltr_LM_Set_Filter(uint8_t filter_type, t_filter* p_filter);
#endif /* #if defined(TEST_HAL_ENABLE) */

//port
typedef unsigned char u_int8;
typedef signed char s_int8 ;
typedef unsigned short int u_int16;
typedef signed short int s_int16;
typedef unsigned int u_int32;
typedef signed int s_int32;

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

#define GIO_HIGH_TXENA_PAGE                    0x4001803C
#define GIO_LOW_TXENA_PAGE                     0x40018054
#define GIO_OVERRIDE_MASK_TXENA_PAGE           0x00008000


#define GIO_HIGH_RXENA_PAGE                    0x4001803C
#define GIO_LOW_RXENA_PAGE                     0x40018054
#define GIO_OVERRIDE_MASK_RXENA_PAGE           0x80000000

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

#define READ_REG_HS6601(n)        ( 0x08000000 | (0<<28) | ((n)<<19) )
#define WRITE_REG_HS6601(n,val)   ( 0x88000000 | (1<<28) | ((n)<<19) | ((val)<<0) )

struct CfgBtHcRfStru
{
    uint8_t phy_type;
    uint8_t debug_mode;
#define RADIO_DEBUG_RXENA_GIO  (0 << 0)
#define RADIO_DEBUG_RXENA_SPI  (1 << 0)
#define RADIO_DEBUG_INTRA_SLOT (1 << 1)
//#define RADIO_DEBUG_CPM_RST_EN (1 << 2)   /*reset CPM when host send HCI_Reset command*/
#define RADIO_DEBUG_TX_CARRIER (1 << 2)
#define RADIO_DEBUG_CALI_RC    (1 << 3)
#define RADIO_DEBUG_CALI_RX    (1 << 4)
#define RADIO_DEBUG_CALI_TX    (1 << 5)
#define RADIO_DEBUG_CALI_AFC   (1 << 6)
#define RADIO_DEBUG_CALI_VCO   (1 << 7)
    uint16_t debug_mon_id;
    uint32_t hw_hab_phy_cfg_word;
    uint32_t ser_eser_cfg_word;

    uint16_t jal_le_ser_eser_tx_time;
    uint16_t jal_le_ser_eser_rx_time;

    /* initialize PHY delays in ms */
    uint8_t osc_startup_time;
    uint8_t phy_startup_delay1;
    uint8_t phy_startup_delay2;
    uint8_t phy_startup_delay3;

    /* radio timing in us */
    uint8_t hw_radio_pll_time;  //T1: PLL for TX and RX
    uint8_t hw_radio_ldo_time;  //T2: LDO, DAC, Up-Mixer for TX; LDO, LNA, Filter, CTSDM for RX
    uint8_t hw_radio_pa_time;   //T3: PA for TX
    uint8_t hw_radio_ramp_time; //T4, T5: Ramp up or Ramp down for TX (1~2us)

    /* input 0, calculate the times above */
    uint8_t hw_radio_tx_setup_time;     //T1+T2+T3+T4
    uint8_t hw_radio_tx_hold_time;      //T5
    uint8_t hw_radio_rx_setup_time;     //T1+T2
    uint8_t hw_radio_rx_hold_time;      //0

    /* input for le timing */
    uint8_t hw_radio_le_tx_setup_time;
    uint8_t hw_radio_le_tx_hold_time;
    uint8_t hw_radio_le_rx_setup_time;
    uint8_t hw_radio_le_rx_hold_time;

    uint8_t hw_radio_tx_tab_delay;
    uint8_t hw_radio_rx_tab_delay;      //including huntersun's digital delay
    uint8_t hw_radio_tx_phy_delay;
    uint8_t hw_radio_rx_phy_delay;

    uint8_t edr_tx_edr_delay;
    uint8_t edr_rx_edr_delay;
    uint8_t le_tifs_delay;
    uint8_t le_search_win_delay;

    uint8_t cor;
    uint8_t le_cor;
    uint8_t win_ext;                    //static win_ext for classic
    uint8_t le_win_ext;

    int8_t  rx_golden_rssi_min;
    int8_t  rx_golden_rssi_max;
    int8_t  rssi_adjust;
    uint8_t rssi_mode; //bit7: 0-late  1-early

    uint8_t low_power;

#define DEEP_SLEEP_STOP_BT_24M_CLK        (1<<0)    /*24MHz clock*/
#define DEEP_SLEEP_STOP_PLL               (1<<1)    /*stop pll*/
#define DEEP_SLEEP_DOWN_CPU_CORE_VOLATGE  (1<<2)    /*down CPU core voltage*/
#define DEEP_SLEEP_GATE_BTPHY_APB_CLOCK   (1<<3)    /*gate btphy apb clock*/
#define DEEP_SLEEP_POWER_DOWN_ADC         (1<<4)    /*power down adc*/

    uint8_t rsv4;
    uint8_t epc_max_tx_power_difference;
    uint8_t epc_max_tx_power_threshold;

    int8_t  tx_power_level_min;
    int8_t  tx_power_level_max;
    uint8_t tx_power_level_units; //Class 1: 25 levels, Class 2: 17 levels, Class 3: 15 levels
    uint8_t tx_power_level_step;  //min: 2dB, max: 8dB

    uint8_t power_ctrl_tab[32];
    int8_t  power_ctrl_tab_dBm[32];

#if 1
    /* AFH */
    uint8_t afh_channel_bit_vector[10];
    uint8_t rsv5;
    uint8_t afh_n_min_used_channels;

    /* scan */
    uint16_t inq_scan_interval;  /* scan interval */
    uint16_t inq_scan_window;    /* scan activity window */
    uint16_t page_scan_interval; /* scan interval */
    uint16_t page_scan_window;   /* scan activity window */

    /*RF calibration parameter*/
    uint16_t tx_loft_sample_times;
    uint16_t tx_gain_sample_times;
    uint16_t tx_phase_sample_times;
    uint16_t rx_phase_sample_times;
    uint16_t agc_calibration_wait_time;

    /*low power - halt system parameter*/
    uint8_t cpu_low_power_divider;
    uint8_t apb_low_power_divider;
    uint16_t cpu_core_vol_reg;

#define HALT_SYSTEM_CPM_USB      (1<<0)
#define HALT_SYSTEM_CPM_SDHC     (1<<1)
#define HALT_SYSTEM_CPM_TIM0     (1<<2)
#define HALT_SYSTEM_CPM_TIM1     (1<<3)
#define HALT_SYSTEM_CPM_TIM2     (1<<4)
#define HALT_SYSTEM_CPM_UART0    (1<<5)
#define HALT_SYSTEM_CPM_UART1    (1<<6)
#define HALT_SYSTEM_CPM_SPI0     (1<<7)
#define HALT_SYSTEM_CPM_SPI1     (1<<8)
#define HALT_SYSTEM_CPM_I2S      (1<<9)
#define HALT_SYSTEM_CPM_DMA      (1<<10)

    uint16_t halt_system_cpm_dis_xxx;    //halt system gate xxx clock.
    uint16_t halt_system_cpm_en_xxx;

    uint16_t rsv6[79*4 - 12/2 - 4 - 9];
#else
    uint16_t tx_freq_int_reg[79];
    uint16_t tx_freq_fra_reg[79];
    uint16_t rx_freq_int_reg[79];
    uint16_t rx_freq_fra_reg[79];
#endif

    //RC calibration parameter
    uint16_t rc_counter;     //total RC calibration counters
    uint16_t rx_adc_t1_t2[16];    // rc value of rxadc. from max to min,and the corresponding tune from min->max
    uint16_t tx_dac_t1_t2[16];    // rc value of txdac
    uint16_t rx_filter_t1_t2[8];  // rc value of rx filter
};

struct CfgBtHcRfStru g_sys_rf_config=
{
#if (HW_RADIO==HWradio_MAX2829)

    /* common settings for max2829 & hs6600 */
    .debug_mode                   =RADIO_DEBUG_CALI_VCO | RADIO_DEBUG_CALI_AFC | RADIO_DEBUG_CALI_RX | RADIO_DEBUG_CALI_TX | RADIO_DEBUG_CALI_RC,
    .debug_mon_id                 =0x135, //DEBUG_MBUS_BTPHY | BB_5

    /*
     * same time for Configure_LE_Spi_For_TxRx_Times()
     *   time = roundup((setup - 75) / 4) = ((87+16)-75)/4 = 7
     */
    .jal_le_ser_eser_tx_time      =0xA980,
    .jal_le_ser_eser_rx_time      =0xA980,

    .osc_startup_time             =10,
    .phy_startup_delay1           =30,
    .phy_startup_delay2           =30,
    .phy_startup_delay3           =250,

    .hw_radio_pll_time            =80,
    .hw_radio_ldo_time            =8,
    .hw_radio_pa_time             =8,
    .hw_radio_ramp_time           =2,

    /* 10us time slipping for rx window */
    .hw_radio_tx_setup_time       =0,
    .hw_radio_tx_hold_time        =0,
    .hw_radio_rx_setup_time       =0,
    .hw_radio_rx_hold_time        =0,

    /* individual LE timing */
    .hw_radio_le_tx_setup_time    =26, //tifs_delay?
    .hw_radio_le_tx_hold_time     =20,
    .hw_radio_le_rx_setup_time    =40, //search_win?
    .hw_radio_le_rx_hold_time     =20,

    .hw_radio_tx_tab_delay        =3,
    .hw_radio_tx_phy_delay        =0,
    .hw_radio_rx_tab_delay        =14,
    .hw_radio_rx_phy_delay        =2,

    .edr_tx_edr_delay             =2,
    .edr_rx_edr_delay             =14, //14=0xe=-2
    .le_tifs_delay                =31,
    .le_search_win_delay          =50,

    .cor                          =7, //HWhab_Set_Sync_Error(7) for Classic
    .le_cor                       =3, //0: BQB
    .le_win_ext                   =0,

    .rssi_mode                    =0x80, //bit7: 1-early 0-late, bit6:rssi_adjust_en, bit[5:4]: save_mode  bit[3:0]: timeout
    .low_power                    =0,  //SYS_LF_OSCILLATOR_PRESENT
    .rsv4                         =0,
    .epc_max_tx_power_difference  =10, //EPC_MAX_TX_POWER_DIFFERENC
    .epc_max_tx_power_threshold   =8,  //EPC_REQ_MAX_TX_POWER_THRESHOLD

    //RC calibration
    //please refer to HS6600_RF shu_mo_jie_kou_v1.1_20150115.pdf
    .rc_counter                   =5,
#if defined (HS66XX_FPGA)
    .rx_adc_t1_t2                 ={356, 346, 335, 324, 313, 302, 292, 281, 270, 259, 248, 238, 227, 216, 205, 194},
    .tx_dac_t1_t2                 ={378, 360, 342, 324, 306, 288, 270, 252, 234, 216, 198, 180, 162, 144, 126, 108},
    .rx_filter_t1_t2              ={360, 330, 300, 270, 240, 210, 180, 150},
#else
    .rx_adc_t1_t2                 ={712, 692, 670, 648, 626, 604, 584, 582, 540, 518, 496, 476, 454, 432, 410, 388},
    .tx_dac_t1_t2                 ={756, 720, 684, 648, 612, 576, 540, 504, 468, 432, 396, 360, 324, 288, 252, 216},
    .rx_filter_t1_t2              ={720, 660, 600, 540, 480, 420, 360, 300},
#endif
#if defined(HS66XX_FPGA)
    .phy_type                     =PHY_TYPE_MAX2829,
    .hw_hab_phy_cfg_word          =PHY_CFG_MAX2829,
    .ser_eser_cfg_word            =SER_CFG_HS6600,//MAX2829,

    .rx_golden_rssi_min           =-56, //0: return rawRSSI
    .rx_golden_rssi_max           =-30,
    .rssi_adjust                  =-20,

    .tx_power_level_min           =-30, //TX_POWER_LEVEL_Pmin
    .tx_power_level_max           =0,   //TX_POWER_LEVEL_Pmax
    .tx_power_level_units         =6,   //HW_RADIO_MAX_TX_POWER_LEVEL 7 levels
    .tx_power_level_step          =5,   //HW_RADIO_TX_POWER_STEP_SIZE 5dBm

    .power_ctrl_tab               ={0x00,0x0C,0x16,0x20,0x2A,0x35,0x3F},
    .power_ctrl_tab_dBm           ={MAX2829_TX_VGA_GAIN_MAX-30, MAX2829_TX_VGA_GAIN_MAX-25, MAX2829_TX_VGA_GAIN_MAX-20,
                                    MAX2829_TX_VGA_GAIN_MAX-15, MAX2829_TX_VGA_GAIN_MAX-10, MAX2829_TX_VGA_GAIN_MAX-5,
                                    MAX2829_TX_VGA_GAIN_MAX-0},

    //.tx_freq_int_reg              ={FREQ_INT_IF0},
    //.tx_freq_fra_reg              ={FREQ_FRA_IF0},
    //.rx_freq_int_reg              ={FREQ_INT_IF750k},
    //.rx_freq_fra_reg              ={FREQ_FRA_IF750k},
#else /* #if defined(HS66XX_FPGA) */
//    .phy_type                     =PHY_TYPE_HS6600,
//    .hw_hab_phy_cfg_word          =PHY_CFG_HS6600,
//    .ser_eser_cfg_word            =SER_CFG_HS6600,
    .rx_golden_rssi_min           =-56,
    .rx_golden_rssi_max           =-30,
    .rssi_adjust                  =0,

    .tx_power_level_min           =-28, //TX_POWER_LEVEL_Pmin
    .tx_power_level_max           =0,   //TX_POWER_LEVEL_Pmax
    .tx_power_level_units         =7,   //HW_RADIO_MAX_TX_POWER_LEVEL 11 levels
    .tx_power_level_step          =4,   //HW_RADIO_TX_POWER_STEP_SIZE 4dBm

    .power_ctrl_tab               ={0x0E,0x0C,0x0A,0x08,0x06,0x04,0x02,0x00,0x11,0x22,0x20 },    // (PA config << 4) + PGA config.
    .power_ctrl_tab_dBm           ={-28, -24, -20, -16, -12, -8, -4, 0, 4, 8, 12},
#endif /* #if defined(HS66XX_FPGA) */
    .afh_channel_bit_vector       ={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F},
    .afh_n_min_used_channels      =19,

    .inq_scan_interval            =0x0800, /* even, 1.28 Seconds */
    .inq_scan_window              =0x0020, /* 0x12 -> 0x20 */
    .page_scan_interval           =0x0800, /* even, 1.28 Seconds */
    .page_scan_window             =0x0020, /* 0x12 -> 0x20 */

    /*RF calibration parameters*/
    .tx_loft_sample_times         = 3000,
    .tx_gain_sample_times         = 1000,
    .tx_phase_sample_times        = 1000,
    .rx_phase_sample_times        = 3000,

    /*low power - halt system parameter*/
    .cpu_low_power_divider        = 8,
    .apb_low_power_divider        = 8,
    .cpu_core_vol_reg             = 0xad54,     //cpu core @1.1v
    .halt_system_cpm_dis_xxx      = 0x7FF,
    .halt_system_cpm_en_xxx       = 0x7FC,

#elif ((HW_RADIO==HWradio_BU9467MUV_DS2) || (HW_RADIO==HWradio_BU9467MUV) || (HW_RADIO==HWradio_BU9468MUV))
    .hw_hab_phy_cfg_word          =HWhab_PHY_CFG,

    .le_tifs_delay                =30,
    .le_search_win_delay          =50,

    .cor                          =7, //HWhab_Set_Sync_Error(7) for Classic

    .rx_golden_rssi_min           =-56,
    .rx_golden_rssi_max           =-30,

    .tx_power_level_min           =-30, //TX_POWER_LEVEL_Pmin
    .tx_power_level_max           =0,   //TX_POWER_LEVEL_Pmax

#if (ROHM_RADIO_VER==9468)
    .tx_power_level_units         =4,
    .tx_power_level_step          =3,
#else
    .tx_power_level_units         =2,
    .tx_power_level_step          =5,
#endif
#endif
};

#define HWdelay_Wait_For_us(n)  chSysPolledDelayX(CPU_DEFAULT_CLOCK/1000000 * n)

//for test
static void _HS6601_MOD_TX_EN(void)
{
  HS_BTPHY->TX_RX_EN = 0x80;    //rx_en set 0, and bandband(CEVA or MAC6200) controls tx
  HS_MAC6200->SPIRCON = 0;
}
static void _HS6601_MOD_RX_EN(void)
{
  HS_BTPHY->TX_RX_EN = 0x82;    //rx en
  //HS_ANA->MAIN_ST_CFG[1] = (HS_ANA->MAIN_ST_CFG[1] & 0xFFFF0FFF) | 0x3000;    //agc_sync_timeout = 3
  //HS_MAC6200->SPIRCON |= RF_MAC_SELECT;
}

static void _HS6601_CEVA_TXENA(bool txena_on)
{
#if 0 //avoid rx short when test tx
  HS_ANA->PD_CFG[2] |= (1 << 8); //[8]pd_rxlna_reg
  HS_ANA->COMMON_CFG[0] &= ~(1 << 29); //[29]cali_rxatten
  HS_ANA->PD_CFG[0] |= (1 << 26); //[26]pd_pkdect_reg
  HS_ANA->RX_AGC_CFG[4] &= ~(1 << 16); //[16]rx_gain_flag
  HS_ANA->RX_AGC_CFG[4] &= ~(0x7 << 11); //[13:11]rx_gain_reg
#endif
  if (txena_on)
    HWradio_SetOverrideHigh(TXENA);
  else
    HWradio_SetOverrideLow(TXENA);
}
static void _HS6601_CEVA_RXENA(bool rxena_on)
{
  if (rxena_on) {
    if (m_rxena_spi_mode)
      HS_BTPHY->ANALOGUE[0x30] = (1<<7) + (1<<4); //[7]rx_start: w1 trigger to start rx fsm
    else
      HWradio_SetOverrideHigh(RXENA);
  }
  else {
    if (m_rxena_spi_mode)
      HS_BTPHY->ANALOGUE[0x30] = (1<<6) + (1<<4); //[6]rx_end: w1 trigger to end rx fsm when rxen is spi mode
                                                  //[4]rxen_sel: 0=gio; 1=spi
    else
      HWradio_SetOverrideLow(RXENA);
  }
}

static void _HS6601_MAC6200_CE(bool ce_on)
{
  if (ce_on)
    HS_MAC6200->RFCON |= RF_CE;
  else
    HS_MAC6200->RFCON &= ~RF_CE;    
}

void HS6601_BT_ON(uint8_t tx)
{
  if ((0 == tx) || (2 == tx)) {
    m_rxena_spi_mode = false; /* test RX w/ GIO RXENA */
    HS_BTPHY->ANALOGUE[0x30] = (1<<6) + (0<<4); //[6]rx_end: w1 trigger to end rx fsm when rxen is spi mode
                                                //[4]rxen_sel: 0=gio; 1=spi
  } else {
    m_rxena_spi_mode = true;  /* test RX w/o SPI to trigger RXENA */
    HS_BTPHY->ANALOGUE[0x30] = (1<<6) + (1<<4); //[6]rx_end: w1 trigger to end rx fsm when rxen is spi mode
                                                //[4]rxen_sel: 0=gio; 1=spi
  }

  _HS6601_CEVA_RXENA(false);
  _HS6601_CEVA_TXENA(false);
  HWdelay_Wait_For_us(500);

  if (tx==1) {
    _HS6601_MOD_TX_EN();
    _HS6601_CEVA_TXENA(true);
  }
  else {
    _HS6601_MOD_RX_EN();
    _HS6601_CEVA_RXENA(true);
  }
}

void HS6601_Set_Freq(uint16_t freq)
{
  HS_BTPHY->ANALOGUE[0x44] = freq; //frequency: 2392 ~ 2490 MHz
  HS_BTPHY->ANALOGUE[0x44] = freq+(1<<12); //[12]: w1 trigger?

  /* this step is not required if BT TRX? */
  HS_ANA->VCO_AFC_CFG[0] |= (1<<16); //[16]rf_pll_afc_tlu
}


/* 1. VCO amplitude and AFC calibration */
static void _HS6601_VCO_AFC_Calibration(void)
{
  /* fixed agc dead when calibration after disconnecting the connection. */
  //SPI mode:
  //HS_BTPHY->ANALOGUE[0x30] |= (1<<6);    //[6]rx_end: w1 trigger to end rx fsm when rxen is spi mode
  //GIO mode: TXENA=0 RXENA=0
  HWradio_SetGIOLowReg(TXENA,0x8000);
  HWradio_SetGIOLowReg(RXENA,0x8000);
  HWradio_SetGIOLowReg(TXENA_PAGE,0x8000);
  HWradio_SetGIOLowReg(RXENA_PAGE,0x8000);
  HWradio_SetGIOHighReg(TXENA,0x0000);
  HWradio_SetGIOHighReg(RXENA,0x0000);
  HWradio_SetGIOHighReg(TXENA_PAGE,0x0000);
  HWradio_SetGIOHighReg(RXENA_PAGE,0x0000);

#if defined(__nds32__)
  /* test PLL feedback output */
  //HS_ANA->REGS.RF_PLL_TEST = 0;
  //HS_ANA->COMMON_CFG[1] = (HS_ANA->COMMON_CFG[1] & ~(0x7 << 20)) | (2 << 20); //[22:20]tst_digi_ctrl: 2=rf_pll; 4=sys_pll; 1=fm_pll

  //set RF DCOC lut to 0, not random value.
  memset((void *)&HS_ANA->DCOC_LUT_REG[0], 0x00, 51*4);
  HWdelay_Wait_For_us(500);

  //HS_BTPHY->ANALOGUE[0x6E] &= ~((1<<7)+(1<<8));    //reg<232:231> vref_vc_det: b'00=, cp randge?
  //HS_BTPHY->ANALOGUE[0x6F] = (HS_BTPHY->ANALOGUE[0x6F] & 0x9FFF) | (0<<13) | (1<<14);    //reg<254:253> con_ldo_cp: b'10=
//HS_ANA->REGS.VREF_VC_DET = 0x0;
//HS_ANA->REGS.CON_LDO_CP  = 0x1;

  /* wangxin's AGC settings on 2016.09.23 */
  HS_ANA->FILT_AGC_LUT_REG[0] = 0x00000000;
  HS_ANA->FILT_AGC_LUT_REG[1] = 0x00000000;
  HS_ANA->IF_AGC_LUT_REG[0]   = 0x00000000; 
  HS_ANA->IF_AGC_LUT_REG[1]   = 0x00000000; 
  HS_ANA->IF_AGC_LUT_REG[2]   = 0x0114010D;
  HS_ANA->IF_AGC_LUT_REG[3]   = 0x01320120;
  HS_ANA->IF_AGC_LUT_REG[4]   = 0x01A0015D;
  HS_ANA->FILT_GAINC_LUT_REG  = 0x00000000; //filt_gainc_lut: 0x012->0x000, referenced by dcoc calibration
  HS_ANA->RX_AGC_CFG[0]       = (HS_ANA->RX_AGC_CFG[0] & ~((0x7 << 20) | (0x3 << 16))) | (0x1 << 20) | (0x1 << 16); //agc_settle_time1,2: 0x22->0x11 (1.5us->1us)
  HS_ANA->RX_AGC_CFG[2]       = 0x814f01f0; //[24:16]agc_pfs2: 0x14f, [8:0]agc_pfs: 0x1fc->0x1f0

  //HS_BTPHY->ANALOGUE[0x0A] = 0x203;    //agc_pth1
//HS_ANA->RX_AGC_CFG[1] = (HS_ANA->RX_AGC_CFG[1] & 0xfe00ffff) | (0x203 << 16);              //[27:19]agc_pth1 | [18:10]agc_pth

  //HS_BTPHY->ANALOGUE[0x67] = 0x5886;     //reg<112+15:112>rxlna_xxx, rxtia: removed in hs6601?
     /* move the following lines into init.c  --luwei 2015.09.30*/
#if (BUILD_TYPE == FLASH_BUILD)
  //HS_BTPHY->ANALOGUE[0x73] = 0x30;   //reg<304+15:304>tx_cap, analog pad test mode: removed in hs6601?
  //HS_BTPHY->ANALOGUE[0x63] = 0xad54; //reg<48+15:48>ldo_dig@1.1V, charger current: 1=200mA
//HS_ANA->REGS.CHG_I_ADJ = 1; //1=100mA
  //HS_ANA->REGS.LDO_DIG_ADJ = 0x1; //1=1.2V
  //HS_BTPHY->ANALOGUE[0x62] = 0x0404; //reg<32+16:32>dc-dc duty, bt lobuf ldo?
//HS_ANA->REGS.DCDC_DUTY = 0x1;
//HS_ANA->REGS.LDO_LOBUF_CTRL = 0x3;
#endif
  //HS_BTPHY->ANALOGUE[0x70] = 0x1aa1; //reg<256+15:256> con_ldo_xtal=b'01 ccon_xtal=b'01000 rescon_xtal=b'01
//HS_ANA->REGS.CCON_XTAL = 0x8; //8=+40khz

  /* power on LDOs of RF PLL: [0]pd_bt_ldodiv, [1]pd_bt_ldommd, [2]pd_bt_synth_cp, [3]pd_bt_synth_ldocp, [4]pd_bt_synth_vc_det, [5]pd_bt_ldopfd, [6]pd_ldo_lobuf, [12]pd_bt_ldovco */
  //HS_BTPHY->ANALOGUE[0x2C] &=~ 0xFE;  //[7:1]pd_ldo_xxx_flag: 0=reg
  //HS_BTPHY->ANALOGUE[0x2B] &=~ 0xFE;  //[7:1]pd_ldo_xxx     : 0=power on
  HS_ANA->PD_CFG[1] &= ~((1 << 28) | (1 << 22) | (1 << 21) | (1 << 20) | (1 << 19) | (1 << 18) | (1 << 17) | (1 << 16)); //xxx_flag
  HS_ANA->PD_CFG[1] &= ~((1 << 12) | (1 <<  6) | (1 <<  5) | (1 <<  4) | (1 <<  3) | (1 <<  2) | (1 <<  1) | (1 <<  0)); //xxx_reg
  HWdelay_Wait_For_us(8);  //wait pd_ldo_xx setup

    /* VCO amplitude calibration, refer to HS6600_BTPHY_REG_v1.6_20141229.xlsx and RFPLL_IO_0912_v0.2.pdf */
    //HS_BTPHY->ANALOGUE[0x4A] |= 0x10; //[4]vco_amp_cal_start: set 1 to start VCO amplititude calibration
    //while( (HS_BTPHY->ANALOGUE[0x4A] & 0x10) == 0x10 ); //wait VCO amplitude calibration finished
    HS_ANA->PEAKDET_CFG |= 0x10; //[4]vco_amp_cal_start: w1
    while (HS_ANA->PEAKDET_CFG & 0x10); //wait hw self clear

    HWdelay_Wait_For_us(8);  //wait VCO stable

    /* AFC calibration, refer to HS6600_BTPHY_REG_v1.6_20141229.xlsx and RFPLL_IO_0912_v0.2.pdf */
    //HS_BTPHY->ANALOGUE[0x40] |= 0x100; //[8]afc_start: set 1 to start AFC calibration
    //while( (HS_BTPHY->ANALOGUE[0x40] & 0x100) == 0x100 ); //wait AFC calibration finished
    HS_ANA->VCO_AFC_CFG[0] |= 0x100; //[8]rf_pll_afc_start: w1
    while (HS_ANA->VCO_AFC_CFG[0] & 0x100); //wait hw self clear

    HWdelay_Wait_For_us(10);  //wait AFC stable

  //HS_BTPHY->ANALOGUE[0x70] |= (1<<13); //reg<269>pfd_rst_b: 1=cali; 0=normal, pull high according yanguang and Yangyi's mail, Feb.13 2015: removed in hs6601?

  //HS_BTPHY->ANALOGUE[0x08] = 0x402;    //[15:12]agc_sync_time b'0000=no timeout | [11:8]agc_gain1 b'0100 | [5:4]pkd_reset_time b'00 | [1:0]agc_settle_time b'10=1.5us
//HS_ANA->MAIN_ST_CFG[1] = (HS_ANA->MAIN_ST_CFG[1] & ~(0x0f << 12)) | (0 << 12); //[15:12]agc_sync_time b'0000=no timeout
  
  //HS_BTPHY->TESTCTRL0 &=~(1<<1);   //pd_core_lv: removed in hs6601
  //reg<288:287>sel_amp: b'01=x2 (in default) -> b'00=x1: removed in hs6601
  //HS_BTPHY->ANALOGUE[0x71] = (HS_BTPHY->ANALOGUE[0x71] & ~(1 << 15)) | (0 << 15);
  //HS_BTPHY->ANALOGUE[0x72] = (HS_BTPHY->ANALOGUE[0x72] & ~(1 << 0))  | (0 << 0);

  //HS6601A2 fixed high frequency bug, manually modified high freq ctuning table.
    HS_BTPHY->ANALOGUE[0x44] = 2490;
    HS_ANA->VCO_AFC_CFG[0] |=(1<<16);
    HWdelay_Wait_For_us(1);  //wait AFC stable
    HS_ANA->DBG_IDX = 4;
    HS_ANA->LNA_CLOAD_CFG = (HS_ANA->LNA_CLOAD_CFG & 0xFFFFFFF) | ((HS_ANA->DBG_RDATA & 0x0E)<<27);
#endif
}

//2.rc calibration
void _HS6601_RC_Calibration(void)
{
  pmu_cali_rc();
}

//3.ldo calibration
void _HS6601_LDO_Calibration(void)
{
  pmu_cali_ldo();
}


//4.LNA LC calibration(void)
void _HS6601_LNA_LC_Calibration()
{
}

//5.dcoc calibration
void _HS6601_DCOC_Calibration(int short_lna)
{
	uint32_t rx_gainc_regs[9];
	uint32_t i;
	uint32_t filt_gainc_reg;
	uint32_t gainc_lut[51];
	uint32_t gainc;
	uint32_t dcoc_lut_in;

	HS_PMU->SCRATCH = 0x01;     //cali path

    //set RF DCOC lut to 0, not random value.
    memset((void *)&HS_ANA->DCOC_LUT_REG[0], 0x00, 51*4);
    HWdelay_Wait_For_us(500);

	for(i=0x00;i<9;i++)
	{
		rx_gainc_regs[i] = HS_ANA->RX_GAINC_LUT_REG[i] ;
		rx_gainc_regs[i] = rx_gainc_regs[i] & 0x3FFF3FFF;
	}
	filt_gainc_reg = HS_ANA->FILT_GAINC_LUT_REG;

	//1st 17 items
	for(i=0x00;i<8;i++)
	{
		gainc_lut[i*2+0] =    ((rx_gainc_regs[i] & 0x3FFF)<<2)      | ((filt_gainc_reg & 0x03) >> 0);
		gainc_lut[2*i+1] =    ((rx_gainc_regs[i] & 0x3FFF0000)>>14) | ((filt_gainc_reg & 0x03) >> 0);
	}
	gainc_lut[16] =           ((rx_gainc_regs[8] & 0x3FFF)<<2)      | ((filt_gainc_reg & 0x03) >> 0);

	//2nd 17 items
	for(i=0x00;i<8;i++)
	{
		gainc_lut[17+i*2+0] = ((rx_gainc_regs[i] & 0x3FFF)<<2)      | ((filt_gainc_reg & 0x30) >> 4);
		gainc_lut[17+2*i+1] = ((rx_gainc_regs[i] & 0x3FFF0000)>>14) | ((filt_gainc_reg & 0x30) >> 4);
	}
	gainc_lut[33] =           ((rx_gainc_regs[8] & 0x3FFF)<<2)      | ((filt_gainc_reg & 0x30) >> 4);

	//3rd 17 items
	for(i=0x00;i<8;i++)
	{
		gainc_lut[34+i*2+0] = ((rx_gainc_regs[i] & 0x3FFF)<<2)      | ((filt_gainc_reg & 0x300) >> 8);
		gainc_lut[34+2*i+1] = ((rx_gainc_regs[i] & 0x3FFF0000)>>14) | ((filt_gainc_reg & 0x300) >> 8);
	}
	gainc_lut[50] =           ((rx_gainc_regs[8] & 0x3FFF)<<2)      | ((filt_gainc_reg & 0x300) >> 8);


	//analog initialise
//	HS_ANA->PD_CFG[0] &= 0xF33FEF07;
//	HS_ANA->PD_CFG[1] &= 0xE780E780;
//	HS_ANA->PD_CFG[2] &= 0xE000E000;

#if 1
	HS_ANA->PD_CFG[2] &= ~(1 << 23);   //[23]pd_rxagc_flag=0
	HS_ANA->PD_CFG[2] |=  (1 << 7);    //[7]pd_rxagc_reg=1
#else
	//pd_rxadc_dacbias pd_mea_ldo, pd_sar_ldo, pd_dcoc, pd_rfsctgm
	HS_ANA->PD_CFG[0] &=~((1<<12)+(1<<4)+(1<<5)+(1<<6)+(1<<7)+(1<<20)+(1<<21));
	//pd_bt_ldodiv  pd_bt_ldommd pd_bt_synth_cp pd_bt_synth_ldocp  pd_bt_synth_vc_det  pd_bt_ldopfd  pd_bt_ldovco pd_bt_vco_pkdetect
	HS_ANA->PD_CFG[1] &=~( (1<<0)+(1<<16) + (1<<1)+(1<<17) + (1<<2)+(1<<18) + (1<<3)+(1<<19)
			              + (1<<4)+(1<<20) + (1<<5)+(1<<21) + (1<<12)+(1<<28) + (1<<13)+(1<<29) );

	HWdelay_Wait_For_us(80);
	HS_ANA->PD_CFG[1] &=~((1<<6)+(1<<22));     //pd_ldo_v1p3_lobuf

	HS_ANA->PD_CFG[2] = (1<<7);    //pd_rxagc = 1
#endif

#if 0//debug pd rxlna, rxgm, rxmix
    chprintf(g_chp, "turn on rxlna, rxgm, rxmix");
    HS_ANA->PD_CFG[2] &= ~((1 << 24) | (1 << 8));
    HS_ANA->PD_CFG[2] &= ~((1 << 25) | (1 << 9));
    HS_ANA->PD_CFG[2] &= ~((1 << 27) | (1 << 11));
#endif

	HWdelay_Wait_For_us(40);

    /*
     * analog DCOC calibration
     */

	//analog calibration mode: see section 3.5.2's table 1
	HS_ANA->RX_FIL_CFG = (HS_ANA->RX_FIL_CFG & ~(3 << 12)) | (2 << 12);     //[13:12]swap_fil=2'b10
	HS_ANA->ADCOC_CNS  = (HS_ANA->ADCOC_CNS & ~0x3F) | (1 << 5) | (1 << 0); //[5]en_byp_dcoc=1 [0]sel_mod_dcoc=1 [1]sel_iq_dcoc=0 [3]sel_di_dcoc=0 [6]sel_dq_dcoc=0 [4]en_cali_dcoc=0
	HS_ANA->ADCOC_CNS  |= (1 << 21) | (1 << 13); //[21][13]do_dcoc_q/i_flag: 1=fsm
    HWdelay_Wait_For_us(2);

	for(i=0;i<51;i++)
	{
		//gainc config
      if (short_lna)
		gainc= (gainc_lut[i] & 0xCFFF) | 0x2000; //[13]shrt_lna=1 [12]sw_lna=0: don't use RF input
      else
        gainc = gainc_lut[i];//luwei: use RF input for debug
        HS_ANA->RX_AGC_CFG[4] = (HS_ANA->RX_AGC_CFG[4] & 0xFFFE0000) | gainc; //rx_gain

		//select I && start calibration
		HS_ANA->ADCOC_CNS &= ~(1 << 1); //[1]sel_iq_dcoc=0=I
		HS_ANA->ADCOC_CNS |= (1 << 3);  //[3]sel_di_dcoc
		HWdelay_Wait_For_us(20);
        HS_ANA->ADCOC_CNS |= (1 << 4); //[4]en_cali_dcoc
		while((HS_ANA->ADCOC_CNS & (1<<31)) == 0x00 ); //[31]califsh_dcoc

		//get I cali result before disable calibration
		dcoc_lut_in = (HS_ANA->ADCOC_CNS & 0x1F000000)>>8;
		HS_ANA->ADCOC_CNS &= ~((1 << 1) | (1 << 3) | (1 << 6) | (1 << 4)); //[4]en_cali_dcoc=0

		//select Q && start calibration
		HS_ANA->ADCOC_CNS |= (1 << 1); //[1]sel_iq_dcoc=1=Q
		HS_ANA->ADCOC_CNS |= (1 << 6); //[6]sel_dq_dcoc
		HWdelay_Wait_For_us(20);
        HS_ANA->ADCOC_CNS |= (1 << 4); //[4]en_cali_dcoc
		while( (HS_ANA->ADCOC_CNS & (1<<31)) == 0x00 );

		//get Q cali result before disable calibration
		dcoc_lut_in |= (HS_ANA->ADCOC_CNS & 0x1F000000);
		HS_ANA->ADCOC_CNS &= ~((1 << 1) | (1 << 3) | (1 << 6) | (1 << 4)); //[4]en_cali_dcoc=0

        //put I & Q cali value to dcoc table
		HS_ANA->DCOC_LUT_REG[i] = dcoc_lut_in;
	}

	/*
     * digital DCOC calibration
     */

	//filter path init
	HS_BTPHY->FM_CORDIC_BP=1;    //cordic bypass & filter gain
	HS_ANA->TRX_COMP_CFG[0] &=0xFFFF7F7F;    //rx i/q offset to zero

	HS_BTPHY->FM_MODE = 1;    //set fm mode
	HWdelay_Wait_For_us(10);
	HS_BTPHY->FM_LRHC_FILT = 0x1100e;    //clear FM ram

	//iq switch clr
	HS_BTPHY->IQ_IN_SWAP = 0;

	//digital calibration mode: see section 4's table
	//HS_ANA->RX_FIL_CFG = (HS_ANA->RX_FIL_CFG & ~(3 << 12)) | (0 << 12);     //[13:12]swap_fil=2'b00
//HS_ANA->RX_FIL_CFG = (HS_ANA->RX_FIL_CFG & ~(3 << 12)) | (2 << 12);     //[13:12]swap_fil=2'b10 for debug to verify analog calibration
	HS_ANA->ADCOC_CNS  = (HS_ANA->ADCOC_CNS & ~0x3F) | (1 << 5) | (1 << 0); //[5]en_byp_dcoc=1 [0]sel_mod_dcoc=1 [1]sel_iq_dcoc=0 [3]sel_di_dcoc=0 [6]sel_dq_dcoc=0 [4]en_cali_dcoc=0

	//calibr I part
	for(i=0x00;i<51;i++)
	{
      if (short_lna)
        gainc= (gainc_lut[i] & 0xCFFF) | 0x2000; //[13]shrt_lna=1 [12]sw_lna=0: don't use RF input
      else
        gainc = gainc_lut[i];//luwei: use RF input for debug
		HS_ANA->RX_AGC_CFG[4] = (HS_ANA->RX_AGC_CFG[4] & 0xFFFE0000) | gainc; //rx_gain
		HWdelay_Wait_For_us(20);

		//analog dcoc feedback to analog part
		HS_ANA->ADCOC_CNS = (HS_ANA->ADCOC_CNS & 0xFF0000FF) | ((HS_ANA->DCOC_LUT_REG[i] & 0xFFFF0000) >> 8);// | (1 << 21) | (1 << 13); //[21][13]do_dcoc_q/i_flag: 1=fsm

		HWdelay_Wait_For_us(800);    //>800us
		HS_ANA->DCOC_LUT_REG[i] = (HS_ANA->DCOC_LUT_REG[i] & 0xFFFFFF00) | (((HS_BTPHY->FM_DECIMATION & 0x00001FE0) >> 5) & 0xFF);
	}

	HS_BTPHY->IQ_IN_SWAP = 1;

	//calib Q part
	for(i=0x00;i<51;i++)
	{
		//gainc config
      if (short_lna)
		gainc= (gainc_lut[i] & 0xCFFF) | 0x2000; //[13]shrt_lna=1 [12]sw_lna=0: don't use RF input
      else
        gainc = gainc_lut[i];//luwei: use RF input for debug
		HS_ANA->RX_AGC_CFG[4] = (HS_ANA->RX_AGC_CFG[4] & 0xFFFE0000) | gainc; //rx_gain
		HWdelay_Wait_For_us(20);

		//analog dcoc feedback to analog part
		HS_ANA->ADCOC_CNS = (HS_ANA->ADCOC_CNS & 0xFF0000FF) | ((HS_ANA->DCOC_LUT_REG[i] & 0xFFFF0000) >> 8);// | (1 << 21) | (1 << 13); //[21][13]do_dcoc_q/i_flag: 1=fsm

		HWdelay_Wait_For_us(800);    //>800us
		HS_ANA->DCOC_LUT_REG[i] = (HS_ANA->DCOC_LUT_REG[i] & 0xFFFF00FF) | ((((HS_BTPHY->FM_DECIMATION & 0x00001FE0) >> 5) & 0xFF) << 8);
	}

    //normal work mode: see section 3.5.2's table 2
	HS_ANA->RX_FIL_CFG = (HS_ANA->RX_FIL_CFG & ~(3 << 12)) | (0 << 12);     //[13:12]swap_fil=2'b00
//HS_ANA->RX_FIL_CFG = (HS_ANA->RX_FIL_CFG & ~(3 << 12)) | (2 << 12);     //[13:12]swap_fil=2'b10=lpf for debug
	HS_ANA->ADCOC_CNS  = (HS_ANA->ADCOC_CNS & ~0x3F) | (0 << 5) | (1 << 0); //[5]en_byp_dcoc=0 [0]sel_mod_dcoc=1 [1]sel_iq_dcoc=0 [3]sel_di_dcoc=0 [6]sel_dq_dcoc=0 [4]en_cali_dcoc=0
	HS_ANA->ADCOC_CNS  |= (1 << 21) | (1 << 13); //[21][13]do_dcoc_q/i_flag: 1=fsm

    /* the lines as below will be moved to _HS6600_Calibration_Post() */

	//iq switch clear
	HS_BTPHY->IQ_IN_SWAP = 0;
	HS_BTPHY->FM_MODE = 0;
    HS_BTPHY->FM_CORDIC_BP = 0; //don't bypass cordic & dc removal & ... which are shared by BTRX & FM

    /* new rxfil gain for rxagc by wangxin, it is good for too strong input */
    HS_ANA->FILT_AGC_LUT_REG[0] = 0x00B50000;
    HS_ANA->FILT_AGC_LUT_REG[1] = 0x000000F0;

    /* workaround: regs set gain_idx to !0, then fsm set gain_idx followed by enable rxagc */
    HS_ANA->MAIN_ST_CFG[1] = (HS_ANA->MAIN_ST_CFG[1] & 0xFFFF0FFF) | 0x0000;    //agc_sync_timeout = 0
    HS_ANA->RX_AGC_CFG[4] = (HS_ANA->RX_AGC_CFG[4] & 0xFFFE0000) | 0x904a; //rx_gain=max: rx_gain[13:0]=52dB | fil_gain[1:0]=b'10
    //HS_ANA->RX_AGC_CFG[4] = (HS_ANA->RX_AGC_CFG[4] & 0xFFFE0000) | 0xa04a; //rx_gain=max: rx_gain[13:0]=52dB | fil_gain[1:0]=b'10, and shrt_lna for debug
    __hal_set_bitsval(HS_ANA->RX_AGC_CFG[3], 24, 31, 50/*gain_idx to change later*/);
    HWdelay_Wait_For_us(20);

	//AGC restore normal mode
    __hal_set_bitsval(HS_ANA->RX_AGC_CFG[3], 31, 31, 1); //[31]gain_idx_flag: 1=fsm
    HS_ANA->RX_AGC_CFG[4] = 0xA2610000; //[16]rx_gain_flag ...: 1=fsm to enable AGC

#if 0
	//pd_xxx signal comes from FSM.
	HS_ANA->PD_CFG[0] = (1<<1)+(1<<2)+(1<<4)+(1<<6)+(1<<23)+(1<<25)+(1<<27);   //pd_dcoc=0
	HS_ANA->PD_CFG[1] = 0xFFFF0000;
	HS_ANA->PD_CFG[2] = 0xFFFF0000;
#endif
}

void _HS6601_AGC_Calibration(void)
{
	HS_ANA->RX_AGC_CFG[0] |=(1<<8);
	while(HS_ANA->RX_AGC_CFG[0] & (1<<8));

}

static void _HS6601_Tx_Single_Tone(void)
{
  //analog part initial -- DCOC calibration
  HS_ANA->COMMON_PACK[0x06] = 0x020802BC;
  HS_ANA->COMMON_CFG[0x00]  = 0x00400200;
  HS_ANA->ADCOC_CNS = 0xC1;
  HS_ANA->PD_CFG[0x00] = 0xFFCFEFFF;
  HS_ANA->PD_CFG[0x01] = 0xFFFFFFEF;

  //tx special pattern
  //CEVA part
  HS_BTBB_RF->PHY_CFG.bits.REFCLK_DIV = 24;  //divide 24;
  HS_BTBB_RF->PHY_CFG.bits.REFCLK_DIR = 1;   //clock comes from PLL

  HS_MAC6200->SPIRCON = 0x8F; //select hs6200 MAC and mask all interrupt
  HS_MAC6200->RFCON &= 0xFD;  //csn=0
  while((HS_MAC6200->SPIRSTAT & 0x02) == 0x02 )  //2-level tx fifo empty
  {
    HS_MAC6200->SPIRDAT = 0x20 | 0x06;   //RF_SETUP addr: 0x06
    HS_MAC6200->SPIRDAT = 0xC2;
    break;
  }
  while((HS_MAC6200->SPIRSTAT & 0x08)==0x08)     //2-level rx fifo full
  {
    HS_MAC6200->SPIRDAT;
    HS_MAC6200->SPIRDAT;
  }
  while((HS_MAC6200->SPIRSTAT & 0x10)==0x10);  //wait for spi not busy
  HS_MAC6200->RFCON |= 0x02;  //csn=1
  HS_MAC6200->RFCON |=0x01;    //ce=1

  HS_BTPHY->TX_IF_REG = 0xc00;  //320kHz

  //tx init
  HS_BTPHY->TX_RX_EN |=0x05;
}

//10. restore to normal txrx state.
static void _HS6601_Calibration_Post(void)
{
  /* 1. disable fm mode */
  //HS_BTPHY->FM_MODE = 0;
  HS_ANA->REGS.RXADC_SEL_INPUT = 0; //BT RX
  HS_ANA->REGS.RF_PLL_MODE = 1; //BT

  /* 2. restore adc default value */
  //HS_BTPHY->ANALOGUE[0x2A] = 0xFFFE; //pd_rx_xxx_flag: 1=fsm; pd_rx_xxx: 1, changed in hs6601?
  //HS_BTPHY->ANALOGUE[0x2C] = 0xFF;   //pd_ldo_xxx_flag: 1=fsm
//HS_ANA->PD_CFG[1] |= (1 << 22) | (1 << 21) | (1 << 19) | (1 << 18) | (1 << 17) | (1 << 16); //pd_ldo_xxx_flag

  //HS_BTPHY->ANALOGUE[0x27] = 0x80; //[8]os_calib_en b'0 | [7:0]dpd_thes_1 0x80: change in hs6601?
//HS_ANA->RX_AGC_CFG[0] &= ~0x100; //[8]os_cali_en: 0=disable offset calibration enable
  //HS_BTPHY->ANALOGUE[0x43] &=~(1<<7); //[7]con_mmd_mn: 0=rf pll sdm out comes from FSM
//HS_ANA->SDM_CFG &= ~0x100; //[8]bt_synth_con_mmd_mn: b'0=fsm

  /* 3. rx_gainc and rxiq_offset restore to comes from FSM */
  //HS_BTPHY->ANALOGUE[0x21] |=(1<<12) + (1<<13); //[13]rxiq_offset_flag b'1 | [12]rx_gainc_flag b'1
  HS_ANA->RX_AGC_CFG[4] = 0xA2610000; //[16]rx_gain_flag ...: 1=fsm to enable AGC
  
  /* 4. pd_rxlnaldo,pd_rxldo,pd_rxlna,pd_rxmix,pd_rxtia,rxagc_sel,agc_en comes from FSM. */
  //HS_BTPHY->ANALOGUE[0x0B] = 0xFF1F; //pd_rx_xxx_flag: 1=fsm
//HS_ANA->PD_CFG[2] |= 0xFFFF0000; //pd_rx_xxx_flag: 1=fsm

  /* 5. tx related restore to default */
  //HS_BTPHY->ANALOGUE[0x00] = 0xF5F6; //pd_tx_xxx_flag: 1=fsm
  //HS_BTPHY->ANALOGUE[0x01] = 0xF1E9; //pd_tx_ldo_xxx_flag: 1=fsm
//HS_ANA->PD_CFG[1] |= 0xEFBF0000; //pd_tx_xxx_flag: 1=fsm, except for [28]pd_bt_ldovco_flag [22]pd_ldo_v1p3_lobuf_flag 

  HS_ANA->PD_CFG[0] = 0xFFCFEFFF; //[21]pd_rfcstgm [20]pd_dcoc [12]pd_rxadc_dacbias
  HS_ANA->PD_CFG[1] = 0xFFFFFFEF; //[4]pd_bt_synth_vc_det=0 [20]pd_bt_synth_vc_det_flag(removed)
  HS_ANA->PD_CFG[2] = 0xFFFFFFFF;
  HS_ANA->PD_CFG[0] &= ~(1 << 15); //[15]pd_bt_mmd_fm: hs6601A1's bug which cannot shutdown FM's MMD totally; turn it on can improve rx sensitivity in 7dB

  /* Filter: [13:12]swap_fil b'00=complex filter
             [0]modsel_fil  b'1=BT b'0=FM */
  HS_ANA->RX_FIL_CFG &= ~(0x3 << 12);
  HS_ANA->RX_FIL_CFG |=  (0x1 << 0);

  /* 6.disable tx_cali_en, rx_cali_en */
  //HS_BTPHY->ANALOGUE[0x17] &=~((1<<1)+(1<<3));
//HS_ANA->COMMON_CFG[0] &= ~((1 << 1) | (1 << 0)); //[1]rx_cali_en b'0 | [0]tx_cali_en b'0

  /* 7. restore gauss output signal select signal to come from datapath */
//HS_BTPHY->TX_IF_REG &= ~(1 << 11); //[11]tx_if_reg_sel: 0=gauss output select signal comes from datapath 

  /* 8. disable HS6200 MAC */
  //disabled at the end of _HS6600_Rx_Phase_Calibration()

  /* 9. switch Bluetooth PHY to Baseband control */
//HS_BTPHY->TX_RX_EN &= ~0x7; //reg_tx_en_sel: 0=baseband

  /* 10. tx pa gain to 0dBm */
  /* set output power to maximum, DL_Initialise_Link() function initialise all the link to MAX_POWER_LEVEL_UNITS. */
  //HS_BTPHY->ANALOGUE[0x69] = HS_BTPHY->ANALOGUE[0x69] & (0xFFFC) | ((g_sys_rf_config.power_ctrl_tab[g_sys_rf_config.tx_power_level_units] >>4) & 0x03);   //PA power.
  //HS_BTPHY->ANALOGUE[0x02] = (HS_BTPHY->ANALOGUE[0x02] & 0xFFF0) | (g_sys_rf_config.power_ctrl_tab[g_sys_rf_config.tx_power_level_units] & 0x0F);   //dpga power.
  HS_BTPHY->TX_DPGA_GC = g_sys_rf_config.power_ctrl_tab[g_sys_rf_config.tx_power_level_units] & 0x0f;
  HS_ANA->REGS.GSEL_PA = (g_sys_rf_config.power_ctrl_tab[g_sys_rf_config.tx_power_level_units] >> 4) & 0x03;

  /* turn on pd_ldo_lobuf by register, i.e. keep it always on */
  //HS_BTPHY->ANALOGUE[0x2C] &= ~(1<<1); //[1]pd_ldo_lobuf_flag: 0=reg
  //HS_BTPHY->ANALOGUE[0x2B] &= ~(1<<1); //[1]pd_ldo_lobuf: 0=power on
//HS_ANA->PD_CFG[1] &= ~((1 << 22) | (1 << 6)); //[22]pd_ldo_v1p3_lobuf_flag b'0 | [6]pd_ldo_v1p3_lobuf_reg b'0

  /* HS6600's offset flags differ from the other flags */
  //HS_BTPHY->ANALOGUE[0x12] &=~(1<<12);   //[12]prf_os_flag:  0=fsm
  //HS_BTPHY->ANALOGUE[0x13] &=~(1<<12);   //[12]pif1_os_flag: 0=fsm
  //HS_BTPHY->ANALOGUE[0x14] &=~(1<<12);   //[12]pif2_os_flag: 0=fsm
  /* HS6601's offset flags: 1=fsm */
//HS_ANA->RX_AGC_CFG[3] |= (1 << 21) | (1 << 9); //[21]pif_os_flag b'1 | [9]prf_os_flag b'1

  //HS_BTPHY->ANALOGUE[0x70] = (HS_BTPHY->ANALOGUE[0x70] & (~(0x1F<<2)) ) | (g_sys_config.xtal_cap<<2);   //reg<262:258>ccon_xtal: xtal load cap for V02
  HS_ANA->REGS.CCON_XTAL = 16;//g_sys_config.xtal_cap;
  HS_ANA->REGS.PA_LDO_BW = 1; //width bandwidth requires less setup time of LDOs

  HS_BTPHY->IQ_IN_SWAP = 0;
  HS_BTPHY->FM_MODE = 0;
  HS_BTPHY->FM_CORDIC_BP = 0; //don't bypass cordic & dc removal & ... which are shared by BTRX & FM

  /* 11. inter freq to 750KHz for bt, also refer to HS_BTPHY->RX_RVS_IF_ROT */
  HS_BTPHY->IF_REG = 0x300; //0x300=750k for bt; 0x80=125k for fm
  HS_ANA->INTER_FREQ = 0x340000; //0x340000=750k for bt on analog

  /* 12. enable DC removal */
  HS_BTPHY->EN_DC_REMOVAL = 1; //enable dc removal

  //wait 160us 
  HWdelay_Wait_For_us(160);

  /* GFSK filter sel */
  HS_BTPHY->CHF_INI_SEL = 1; //inital filter tap: 0=GFSK; 1=DPSK

  //HS_BTPHY->ANALOGUE[0x09] = 0x2fe;  //agc_pfs
  //HS_BTPHY->ANALOGUE[0x20] = 0x23e;  //agc_pfs2
  //HS_BTPHY->ANALOGUE[0x08] |= 0x03;  //[1:0]agc_settle_time: 3=1.75us
//HS_ANA->RX_AGC_CFG[0] = ; //[22:20]agc_settle_time2; [17:16]agc_settle_time1

  HS_BTPHY->TH_ANALOG = 10; //the threshold counter of over detect from analog

  //HS_BTPHY->ANALOGUE[0x0A] = 0x20b;  //agc_pth1
  //HS_BTPHY->ANALOGUE[0x08] = (HS_BTPHY->ANALOGUE[0x08] & 0xF0FF) | (0x06 << 8); //[11:8]agc_gain1
//HS_ANA->RX_AGC_CFG[1] = ; //[9:5]agc_gain3; [4:0]agc_gain1

  HS_ANA->FILT_AGC_LUT_REG[0] = 0xb30000;
  HS_ANA->FILT_AGC_LUT_REG[1] = 0x133;
}

static void _HWradio_Init_RF_PHY(void)
{
    HS_BTPHY->DEM_DLY_BKOFF_DPSK=0x0B;   //address 0x4002006C
    HS_BTPHY->SW_VLD_DLY_SEL=0x03;       //address 0x4002005C
    HS_BTPHY->TX_EDR2_DLY_SEL=21;        //address 0x40020034  EDR2 delay
    HS_BTPHY->TX_EDR3_DLY_SEL=17;        //address 0x40020164  EDR3 delay
    HS_BTPHY->GAU_DLY_CNT=32;            //address 0x40020168
    HS_BTPHY->GUARD_DLY_CNT=60;          //address 0x4002016C
    HS_BTPHY->EN_CFO_EST=1;              //address 0x40020018, 1: open CFO, 0 : close CFO
    HS_BTPHY->MAXMIN0_LIM=1;             //address 0x400200F0
    HS_BTPHY->DC_LIM=1;                  //address 0x400200F8
    HS_BTPHY->EN_FAGC=1;                  //address 0x400200B0
    HS_BTPHY->NOISE_DET_EN=0;            //address 0x40020158
    HS_BTPHY->RSSI_EST_SEL=0;            //address 0x400200D4, 1: de-rotator1 output, 0: channel filter output
    HS_BTPHY->RSSI_TIMEOUT_CNST= (g_sys_rf_config.rssi_mode >> 0) & 0x0f; /* 0: wait 2^0 cycles @24MHz since fsync_det */
    HS_BTPHY->RSSI_SAVE_MODE   = (g_sys_rf_config.rssi_mode >> 4) & 0x03; /* 0: store RSSI after sync detect and stable */

    /* settings in cycles @24MHz */
#if 0//luwei
    HS_ANA->MAIN_ST_CFG[0] = ((((g_sys_rf_config.hw_radio_pll_time * 24) & 0xffff) << 16) | //[31:16]pll_wait: in cycles @24MHz
                              (((g_sys_rf_config.hw_radio_ldo_time * 24) & 0xffff) << 0));  //[15:0]txldo_wait
    HS_ANA->MAIN_ST_CFG[1] = ((((g_sys_rf_config.hw_radio_ldo_time * 24) & 0xffff) << 16) | //[31:16]rxldo_wait
                              (( 0                                    & 0x0f)   << 12) | //[15:12]agc_sync_time: 0=no timeout; 1=10us; 2=20us; ...; 15=15us
                              (((g_sys_rf_config.hw_radio_ramp_time -1)  & 0x01)   << 8)  | //[8]ramp_1us: 0=1us; 1=2us
                              (((g_sys_rf_config.hw_radio_pa_time * 24)  & 0xff)   << 0));  //[7:0]pa_wait
#endif

    /* debug BB-PHY interface */
    HS_SYS->DEBUG_MON_ID = g_sys_rf_config.debug_mon_id;
#if defined(HS66XX_FPGA) && (BUILD_TYPE==FLASH_BUILD)
    /* LPO @32000Hz */
    HS_PMU_CPM->RTC_CFG = 0x14;   // RTCCLK source is XTAL
    HS_PSO->BTPHY_CFG = 0x2a04;   // LPO source is XTAL
    HS_PMU_CPM->CRY_CFG = (500 << 7) | 0x11; //FIXME: XTAL@16MHz
    HS_PMU_CPM->UPD = 0x01;
#endif
}
static void _HWhab_Init_RF(void)
{
  volatile uint32_t config_word = 0;

  //donot use autowake up
  *(volatile unsigned int*)0x400180A0 = 0x00000000;

  _HWradio_Init_RF_PHY();

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

int HWradio_init(void)
{
                           /* 0x2029080c, 0xb5541014 */
  uint32_t common_pack[10] = {0x2028000c, 0xb5541004, 0x530020aa, 0x535ab30a,
                /*0x72583201*/0x72580201, 0x964c6449, 0x1e0802bc, 0xaaa82a82,
                              0x80002ac1, 0xc010015a};
  int i, ret = 0;

  /* don't reset BTPHY because some calibration had run in main.c */
  //cpmResetBTPHY();
  if (HS_PSO->BTPHY_CFG & CPM_BUS_GATE) {
    cpmEnableBTPHY();
    msleep(1);
  }

  /* check default value */
  if ((HS_ANA->COMMON_PACK[0] & 0xfffe03ff) != (common_pack[0] & 0xfffe03ff)) //excluding PGA_GAIN, RCtune
    ret = -1;
  if ((HS_ANA->COMMON_PACK[1] & 0xffffffef) != (common_pack[1] & 0xffffffef)) //excluding DRV_GAIN
    ret = -2;
  if ((HS_ANA->COMMON_PACK[2] & 0xffffffff) != common_pack[2])
    ret = -3;
  if ((HS_ANA->COMMON_PACK[3] & 0xffffffff) != common_pack[3])
    ret = -4;
  if ((HS_ANA->COMMON_PACK[4] & 0xfffc87ff) != (common_pack[4] & 0xfffc87ff)) //excluding GSEL_PA, TXDAC_BW_CAL
    ret = -5;
  for (i = 5; i < 10; i++) {
    if (HS_ANA->COMMON_PACK[i] != common_pack[i]) {
      ret = 0-(i+1);
      break;
    }
  }
  /* reg<252:251>con_ldo_pfd b'01=1.3V; b'11=1.4V */
  if (HS_ANA->REGS.CON_LDO_PFD != pmu_ana_get(251, 252))
    ret = -99;

  /* max tx power */
  HS_BTPHY->TX_DPGA_GC = g_sys_rf_config.power_ctrl_tab[10] & 0x0f;
  HS_ANA->REGS.GSEL_PA = (g_sys_rf_config.power_ctrl_tab[10] >> 4) & 0x03;

  *(volatile unsigned int*)0x40018028 = 0x360062b8;    //config phy
  *(volatile unsigned int*)0x40018008 =  0x70000;      //config sym
  *(volatile unsigned int*)0x40018008 = 0x170000;      //config cor

  HS_BTPHY->SPI_APB_SWITCH=0;
  _HWhab_Init_RF();
  //_HWradio_Go_To_Idle_State();
  return ret;
}
void _btrf_help(BaseSequentialStream *chp)
{
    chprintf(chp, "Usage: btrf function params\r\n");
    chprintf(chp, "function: 0 , carrier test;        params: freq(dec in MHz) [1|0|2] [window_ms(dec)] [interval_ms(dec)] [count(dec)] [pa_gain_idx(dec)]\r\n");
    chprintf(chp, "function: 1 , single tone;         params: freq(dec in MHz)         [window_ms(dec)] [interval_ms(dec)] [count(dec)] [pa_gain_idx(dec)]\r\n");
    chprintf(chp, "function: 2 , debug_mon_id;        params: [montor_id(hex) to write]\r\n");
    chprintf(chp, "function: 3 , frequency;           params: [freq(dec in MHz)] [1(tx)|0(rx)|2(rx_gio)] [tx_gain(hex)|rx_gain(hex)]\r\n");
    chprintf(chp, "function: 4 , rw reg in word;      parmas: ana/apb/spi, offset(hex), [value(hex) to write]\r\n");
    chprintf(chp, "function: 5 , rw ana in bits;      parmas: offset(hex), end(dec), start(dec), [value(hex) to write]; \r\n");
    chprintf(chp, "function: 6 , pin mux;             parmas: pin_no, mux_mode; \r\n");
    chprintf(chp, "function: 320, rw 320 bits;        parmas: end(dec), start(dec), [value(hex) to write]; \r\n");
    chprintf(chp, "function: 7 , classic BT test mode;   \r\n");   //BT controller test mode
    chprintf(chp, "function: 8 , debug bus IQ signal;    \r\n");   //debug bus IQ signal
    chprintf(chp, "function: 9 , DCOC calibration     params: [offset_q.offset_i(hex) | cali=1F1F]\r\n");   //DCOC calibration
    chprintf(chp, "function: 10, xtal cap;            parmas: [xtal_cap(dec)] [dac_vol(dec)] [adc_vol(dec)]\r\n");
}

extern int hs_bthc_start(void);
// calibration
//
void cmd_btrf(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void) (chp);
  uint8_t error = 1;
  int index = atoi(argv[0]);
  int mhz = 2440;
  int tx = 1;

  if (argc < 1) {
    _btrf_help(chp);
    return;
  }

  argc--;
  HS_BTPHY->SPI_APB_SWITCH=0;
  switch (index) {
    //carrier test: freq | tx

  case 0:
  {
    int rc;
    if (argc >= 1)
      mhz = atoi(argv[1]);
    if (argc >= 2)
      tx = atoi(argv[2]);

    error = 0;
    if (mhz > 3490) {
      error = 1;
      break;
    }
    rc = HWradio_init();
    if (rc < 0) {
      if (rc == -99)
        chprintf(chp, " !!! Warning: software bug on HS_AYA->REGS.xxx\r\n");
      else
        chprintf(chp, " !!! Warning: regs<319:0> default values mismatch at word%d\r\n", -rc-1);
    }
    _HS6601_VCO_AFC_Calibration();
    //_HS6601_Calibration_Post();
    HS_ANA->MAIN_ST_CFG[1] = (HS_ANA->MAIN_ST_CFG[1] & 0xFFFF0FFF) | 0x3000;    //agc_sync_timeout = 3
    HS6601_Set_Freq(mhz);

    /* keep always on LDOs of RF PLL: [0]pd_bt_ldodiv, [1]pd_bt_ldommd, [3]pd_bt_synth_ldocp, [5]pd_bt_ldopfd, [6]pd_ldo_v1p3_lobuf, [12]pd_bt_ldovco */
    HS_ANA->PD_CFG[1] &= ~((1 << 28) | (1 << 22) | (1 << 21) | (1 << 19) | (1 << 17) | (1 << 16)); //xxx_flag
    HS_ANA->PD_CFG[1] &= ~((1 << 12) | (1 <<  6) | (1 <<  5) | (1 <<  3) | (1 <<  1) | (1 <<  0)); //xxx_reg

    /* keep always on BT RX blocks: [12]pd_rxfil, [11]pd_rxmixer, [10]pd_rxtia, [9]pd_rxgm, [8]pd_rxlna, [5]pd_rxadc_q, [4]pd_rxadc_i, [2]pd_rxadc_biasgen */
    HS_ANA->PD_CFG[2] &= ~((1 << 28) | (1 << 27) | (1 << 26) | (1 << 25) | (1 << 24) | (1 << 21) | (1 << 20) | (1 << 18)); //xxx_flag
    HS_ANA->PD_CFG[2] &= ~((1 << 12) | (1 << 11) | (1 << 10) | (1 <<  9) | (1 <<  8) | (1 <<  5) | (1 <<  4) | (1 <<  2)); //xxx_reg
    /* [23][22]pd_tca [21]pd_rfcstgm [12]pd_rxadc_dacbias */
    HS_ANA->PD_CFG[0] &= ~((1 << 23) | (1 << 22) | (1 << 21) | (1 << 12));

    /* keep always on BT TX blocks: ?[10]pd_txpa, [9]pd_txum, ?[7]pd_txdac */
    HS_ANA->PD_CFG[1] &= ~((1 << 25) | (0 << 23)); //xxx_flag
    HS_ANA->PD_CFG[1] &= ~((1 <<  9) | (0 <<  7)); //xxx_reg
    HWdelay_Wait_For_us(8);  //wait pd_ldo_xx setup

    HS6601_BT_ON(tx);

    if (argc >= 4) {
      int window = atoi(argv[3]);
      int interval = atoi(argv[4]);
      int ii, jj = 7, count = 10;
      if (interval < window)
        interval = window;
      if (argc >= 5)
        count = atoi(argv[5]);
      if (argc >= 6)
        jj = atoi(argv[6]);
      HS_BTPHY->TX_DPGA_GC = g_sys_rf_config.power_ctrl_tab[jj] & 0x0f;
      HS_ANA->REGS.GSEL_PA = (g_sys_rf_config.power_ctrl_tab[jj] >> 4) & 0x03;
      chprintf(chp, "power_ctrl_tab_dBm: ");
      for (ii = 0; ii < 11; ii++)
        chprintf(chp, "%d ", g_sys_rf_config.power_ctrl_tab_dBm[ii]);
      chprintf(chp, "\r\nidx=%d: PA gain %ddBm\r\n", jj, g_sys_rf_config.power_ctrl_tab_dBm[jj]);
      for (ii = 0; ii < count; ii++) {
        for (jj = 0; jj < 2; jj++) {
          if (tx)
            _HS6601_CEVA_TXENA(false);
          else
            _HS6601_CEVA_RXENA(false);
          osDelay(window);
          if (tx)
            _HS6601_CEVA_TXENA(true);
          else
            _HS6601_CEVA_RXENA(true);
          osDelay(window);
        }
        if (interval > window) {
          if (tx)
            _HS6601_CEVA_TXENA(false);
          else
            _HS6601_CEVA_RXENA(false);
          osDelay(interval - window);
        }
      }
    }
    break;
  }

    //single tone: freq
  case 1:
    if (argc >= 1)
      mhz = atoi(argv[1]);

    error = 0;
    HWradio_init();
    _HS6601_VCO_AFC_Calibration();
#if 0
    _HS6601_RC_Calibration();
    _HS6601_LDO_Calibration();
    _HS6601_LNA_LC_Calibration();
#endif
    _HS6601_DCOC_Calibration(1/*short_lna*/);
    //_HS6601_AGC_Calibration(); //9.AGC calibration
    _HS6601_Calibration_Post(); //10. restore to normal txrx state.

    HS6601_Set_Freq(mhz);
    _HS6601_Tx_Single_Tone();
    _HS6601_MAC6200_CE(true);

    if (argc >= 3) {
      int window = atoi(argv[2]);
      int interval = atoi(argv[3]);
      int ii, jj = 7, count = 10;
      if (interval < window)
        interval = window;
      if (argc >= 4)
        count = atoi(argv[4]);
      if (argc >= 5)
        jj = atoi(argv[5]);
      HS_BTPHY->TX_DPGA_GC = g_sys_rf_config.power_ctrl_tab[jj] & 0x0f;
      HS_ANA->REGS.GSEL_PA = (g_sys_rf_config.power_ctrl_tab[jj] >> 4) & 0x03;
      for (ii = 0; ii < count; ii++) {
        /* set frequency manually if MAC6200 */
        HS6601_Set_Freq(mhz);
        //_HS6601_VCO_AFC_Calibration();
        //_HS6601_Tx_Single_Tone();

        _HS6601_MAC6200_CE(true);
        osDelay(window);
        if (interval > window) {
          _HS6601_MAC6200_CE(false);
          osDelay(interval - window);
        }
      }
    }
    break;

	//debug_mon_id
  case 2:
    if (argc == 0) {
      /* read */
      error = 0;
      g_sys_rf_config.debug_mon_id = HS_SYS->DEBUG_MON_ID;
      chprintf(chp,"g_sys_rf_config.debug_mon_id=0x%x,\r\n",g_sys_rf_config.debug_mon_id);
    }
    else if (argc == 1) {
      /* write id */
      error = 0;
      g_sys_rf_config.debug_mon_id = strtol(argv[1], NULL, 16);
      HS_SYS->DEBUG_MON_ID = g_sys_rf_config.debug_mon_id;
      chprintf(chp,"g_sys_rf_config.debug_mon_id=0x%x,\r\n",g_sys_rf_config.debug_mon_id);
    }
    break;

    //rf frequency
  case 3:
    if (argc == 0) {
      /* read: gain */
      uint32_t lut;
      uint32_t gain;
      error = 0;
      chprintf(chp,"RF frequency=%d MHz\r\n", (HS_BTPHY->ANALOGUE[0x44] & 0x0FFF));
      HS_ANA->DBG_IDX = 4;
      lut = HS_ANA->DBG_RDATA;
      chprintf(chp, "current rfpll entry: 0x%08lx track.ftune.ctune=%x.%x.%x\r\n", lut, (lut >> 8) & 0x7, (lut >> 4) & 0x7, (lut >> 0) & 0xf);

      HS_ANA->DBG_IDX = 5;
      lut = HS_ANA->DBG_RDATA;
      HS_ANA->DBG_IDX = 6;
      gain = HS_ANA->DBG_RDATA;
      chprintf(chp, "current rfpll bt_synth_int_freq=0x%08lx bt_synth_freq_sdm=0x%08lx=%d.MHz\r\n", lut, gain, gain >> 20);

      /* tx */
      gain = HS_ANA->REGS.GSEL_PA;
      lut = HS_BTPHY->TX_DPGA_GC & 0x0f;
      chprintf(chp, "current tx_gain=0x%01x%01x: %ddBm (PA=%ddBm DPGA=%ddBm)\r\n", gain, lut, gain*6 - 2*(int)lut, gain*6, -2*(int)lut);

      /* rx */
      gain = HS_ANA->RX_AGC_CFG[4];
      chprintf(chp, "rx_gain regs get: RX_AGC_CFG4=0x%08lx, RX AGC %s, rx_gain=0x%04x, shrt_lna=%d, sw_lna=%d\r\n", gain,
               gain & (1 << 16) ? "enabled" : "disabled",
               gain & 0xffff,
               gain & (1 << 13) ? 1 : 0,
               gain & (1 << 12) ? 1 : 0);

      HS_ANA->DBG_IDX = 0;
      lut = HS_ANA->DBG_RDATA;
      HS_ANA->DBG_IDX = 1;
      gain = HS_ANA->DBG_RDATA;
      chprintf(chp, "current pd_xxx=0x%08lx main_fsm=0x%08lx\r\n", lut, gain);

      HS_ANA->DBG_IDX = 2;
      gain = HS_ANA->DBG_RDATA;
      chprintf(chp, "current rx_gain=0x%04x, pd_xxx=0x%04x\r\n", gain >> 16, gain & 0xffff);
    }
    else if (argc >= 1) {
      /* write: mhz tx gain */
      uint32_t gain = 0;
      mhz = atoi(argv[1]);
      if (argc >= 2)
        tx = atoi(argv[2]);
      if (argc >= 3)
        gain = strtol(argv[3], NULL, 16);

      error = 0;
      if (mhz > 3490) {
        error = 1;
        break;
      }

      if (tx == 1) {
        /* tx */
        if (argc >= 3) {
          HS_BTPHY->TX_DPGA_GC = gain & 0x0f;
          HS_ANA->REGS.GSEL_PA = (gain >> 4) & 0x03;
        }
      }
      if ((tx == 0/*rxena_spi*/) || (tx == 2/*rxena_gio*/)) {
        /* rx */
        if (argc >= 3) {
          if (gain == 0x1FFFF) {
            HS_ANA->RX_AGC_CFG[4] |= (1 << 16); //[16]rx_gain_flag: 1=fsm
            chprintf(chp, "RX AGC enabled: RX_AGC_CFG4=0x%08lx\r\n", HS_ANA->RX_AGC_CFG[4]);
          } else {
            HS_ANA->RX_AGC_CFG[4] = (HS_ANA->RX_AGC_CFG[4] & ~0x1FFFF) | (gain & 0x1FFFF);
            chprintf(chp, "rx_gain regs set: RX_AGC_CFG4=0x%08lx\r\n", HS_ANA->RX_AGC_CFG[4]);
          }
        }
      }

      /* the following operations don't hurt the existing classic bt connection */
      HS6601_Set_Freq(mhz);
      HS6601_BT_ON(tx);
    }
    break;

	//rw reg
  case 4:
    if (argc >= 2) {
      /* read ana|apb|spi offset */
      uint32_t offset = strtol(argv[2], NULL, 16);

      error = 0;
      /* write ana|apb|spi offset value */
      if (argc >= 3) {
        uint32_t value = strtol(argv[3], NULL, 16);

        if(strcmp("apb",argv[1])==0x00) {    //apb
          offset &=~0x03;
          *((uint32_t*)(0x40020000+offset)) = value;    //HS_BTPHY base addr 0x40020000
          chprintf(chp,"readback BTPHY APB REG[0x%x] = 0x%08lx\r\n", offset, *((volatile uint32_t*)(0x40020000+offset)) );
        }
        else if(strcmp("spi",argv[1])==0x00) {   //spi
          HS_BTPHY->ANALOGUE[offset] = value;
          chprintf(chp,"readback BTPHY SPI REG[0x%x] = 0x%08lx\r\n", offset, HS_BTPHY->ANALOGUE[offset]);
        }
        else if(strcmp("ana",argv[1])==0x00) {    //ana
          *((uint32_t*)(0x4000F000+(offset<<2))) = value;    //HS_BTPHY base addr 0x4000F000
          chprintf(chp,"readback ANA REG[0x%x] = 0x%08lx\r\n", offset, *((volatile uint32_t*)(0x4000F000+(offset<<2) ))); //HS_ANA base addr 0x4000F000
        }
        else {
          error = 1;
        }
      } else {
        if(strcmp("apb",argv[1])==0x00) {   //apb
          offset &= ~0x03;
          chprintf(chp,"BTPHY APB REG[0x%x] = 0x%08lx\r\n", offset, *((volatile uint32_t*)(0x40020000+offset))); //HS_BTPHY base addr 0x40020000
        }
        else if(strcmp("spi",argv[1])==0x00) {   //spi
          chprintf(chp,"BTPHY SPI REG[0x%x] = 0x%08lx\r\n", offset, HS_BTPHY->ANALOGUE[offset]);
        }
        else if(strcmp("ana",argv[1])==0x00) {   //ana
          chprintf(chp,"ANA REG[[0x%x] = 0x%08lx\r\n", offset, *((volatile uint32_t*)(0x4000F000+(offset<<2)))); //HS_ANA base addr 0x4000F000
        }
        else {
          error = 1;
        }
      }
    }
    break;

    //rw ana bits
  case 5:
    if (argc >= 1) {
      uint32_t offset, value;
      int start=0, end=31;
      unsigned int mask = 0x00;
      /* read offset end start */
      offset = strtol(argv[1], NULL, 16);
      if (argc >= 2)
        end = atoi(argv[2]);
      if (argc >= 3)
        start = atoi(argv[3]);

      error = 0;
      if (start > end) {
        error = 1;
        break;
      }

      if((end-start)==31){     //GCC compiler compile '<<' as rotate shift right not logical shift right.
        mask = 0xFFFFFFFF;
      }
      else{
        mask=(1 << (end - start + 1)) - 1;
      }

      if (argc >= 4) {
        /* write offset end start value */
        uint32_t valbits = strtol(argv[4], NULL, 16);

        __hal_set_bitsval(*((volatile uint32_t*)(0x4000F000+(offset<<2))), start, end, valbits);
        value = *((volatile uint32_t*)(0x4000F000+(offset<<2)));
        if (((value >> start) & mask) != valbits)
          chprintf(chp, "mismatched: ");
        chprintf(chp, "readback ANA REG[0x%x] = 0x%08lx, bits = 0x%x\r\n",
                 offset, value, (value >> start) & mask);
      } else {
        value = *((volatile uint32_t*)(0x4000F000+(offset<<2)));
        chprintf(chp, "ANA REG[0x%x] = 0x%08lx, bits = 0x%x\r\n",
                 offset, value, (value >> start) & mask);
      }
    }
    break;

    //pin mux
  case 6:
    if (argc >= 1) {
      int pin, mux = 1/*debug*/;
      /* pin_no mux_mode */
      pin = atoi(argv[1]);
      if (argc >= 2)
        mux = atoi(argv[2]);

      if ((pin >= 32) || (mux >= 64)) {
        break;
      }

      error = 0;
      if (pin < 16)
        palSetPadMode(IOPORT0, pin,    PAL_MODE_INPUT|PAL_MODE_ALTERNATE(mux)|PAL_MODE_DRIVE_CAP(3));
      else
        palSetPadMode(IOPORT1, pin-16, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(mux)|PAL_MODE_DRIVE_CAP(3));
    }
    break;

    //rw 320 bits
  case 320:
    if (argc >= 2) {
      /* read end start */
      int start, end;
      end = atoi(argv[1]);
      start = atoi(argv[2]);

      error = 0;
      if (start > end) {
        error = 1;
        break;
      }

      if (argc >= 3) {
        /* write end start value */
        uint32_t valbits = strtol(argv[3], NULL, 16);
        pmu_ana_set(start, end, valbits);
        if (pmu_ana_get(start, end) != valbits)
          chprintf(chp, "mismatched: ");
        chprintf(chp, "readback ANA reg<%d:%d> = 0x%x\r\n", end, start, pmu_ana_get(start, end));
      } else {
        chprintf(chp, "ANA reg<%d:%d> = 0x%x\r\n", end, start, pmu_ana_get(start, end));
      }
    } else {
      /* dump all 320-bit registers */
      int i;
      error = 0;
      for (i = 0; i < 10; i++)
        chprintf(chp, "0x%08lx ", HS_ANA->COMMON_PACK[i]);
      chprintf(chp, "\r\n");
    }
    break;

  case 7:    //BT controller test mode
    #if HS_USE_BT
    hs_bthc_start();

    //add test mode
  	BTtst_dut_mode = DUT_ENABLED;
  	LMscan_Write_Scan_Enable(BOTH_SCANS_ENABLED);

  	t_filter _filter;    //auto connection
  	_filter.class_of_device = 0x258a1;
  	_filter.class_of_device_mask = 0x3;
  	_filter.filter_condition_type = 0x0;
  	_filter.auto_accept = 0x2;
  	LMfltr_LM_Set_Filter(2, &_filter);

    error = 0;
    #endif
    break;

  case 8:    //IQ signal
	  palSetPadMode(IOPORT0, 10 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 0
	  palSetPadMode(IOPORT0, 12 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 1
	  palSetPadMode(IOPORT0, 14 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 2
	  palSetPadMode(IOPORT1, 0 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 3
	  palSetPadMode(IOPORT1, 1 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 4
	  palSetPadMode(IOPORT1, 3 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 6
	  palSetPadMode(IOPORT1, 4 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 7
      #if defined(RUN_IN_FLASH) //PA2/JTAG_TMS
	  palSetPadMode(IOPORT0, 2 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 8
      #endif
	  palSetPadMode(IOPORT0, 7 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 9
	  palSetPadMode(IOPORT0, 9 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 10
	  palSetPadMode(IOPORT0, 11 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 11
	  palSetPadMode(IOPORT0, 13 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 12
	  palSetPadMode(IOPORT0, 15 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 13
	  palSetPadMode(IOPORT1, 2 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 14
	  palSetPadMode(IOPORT1, 6 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 15

	  HS_SYS->DEBUG_MON_ID = g_sys_rf_config.debug_mon_id = 0x12C;    //bt phy mbus
	  error = 0;
	  break;

  case 9:   //DCOC calibration
  {
    uint32_t lut;

    if (argc >= 1) {
      lut = strtol(argv[1], NULL, 16);
      if ((lut == 0x1F1F) || (lut == 0x1F3F)) {
        int i;
        int short_lna = 1;
        if (lut == 0x1F3F)
          short_lna = 0;
        /* don't do VCO calibraton again; DCOC calibration requires a stable RF input */
        chprintf(chp, "Warning: such as btrf 0 2427 0 is required\r\n");
        _HS6601_DCOC_Calibration(short_lna);
        for (i = 0; i < 51; i++)
		   chprintf(chp, "  HS_ANA->DCOC_LUT_REG[%2d]=%08lx \r\n", i,HS_ANA->DCOC_LUT_REG[i]);
        chprintf(chp, "ADCOC_CNS=0x%08lx RX_AGC_CFG3=0x%08lx RX_AGC_CFG4=0x%08lx\r\n", HS_ANA->ADCOC_CNS, HS_ANA->RX_AGC_CFG[3], HS_ANA->RX_AGC_CFG[4]);
        chprintf(chp, "DCOC calibration done: ");
        //_HS6601_Calibration_Post();
      }
      else if (lut != 0xFFFF) {
        __hal_set_bitsval(HS_ANA->ADCOC_CNS, 16, 20, (lut >> 8) & 0x1f);
        __hal_set_bitsval(HS_ANA->ADCOC_CNS,  8, 12, (lut >> 0) & 0x1f);
        HS_ANA->ADCOC_CNS &= ~((1 << 21) | (1 << 13)); //[21][13]do_dcoc_q/i_flag: 0=reg
        chprintf(chp, "DCOC regs set: q.i=%02x.%02x\r\n", (lut >> 8) & 0x1f, (lut >> 0) & 0x1f);
      }

      if (argc >= 2) {
        /* set fix rx_gain and toggle gain_idx to watch current DCOC LUT entry */
        int gain_idx = atoi(argv[2]);
        HS_ANA->RX_AGC_CFG[4] = (HS_ANA->RX_AGC_CFG[4] & 0xFFFE0000) | 0x904a; //rx_gain=max: rx_gain[13:0]=52dB | fil_gain[1:0]=b'10
        __hal_set_bitsval(HS_ANA->RX_AGC_CFG[3], 24, 31, 50/*gain_idx to change later*/);
        HWdelay_Wait_For_us(20);
        __hal_set_bitsval(HS_ANA->RX_AGC_CFG[3], 24, 31, ((gain_idx % 17) << 2) + ((gain_idx / 17) & 0x3) /* idx=rx_gain_idx[4:0] | fil_gain_idx[1:0]*/);
        chprintf(chp, "idx %d to dcoc lut, RX_AGC_CFG3=0x%08lx\r\n", gain_idx, HS_ANA->RX_AGC_CFG[3]);
      }
    }
    HS_ANA->DBG_IDX = 8;
    lut = HS_ANA->DBG_RDATA;
    chprintf(chp, "current dcoc offset = ana.q.i=%02x.%02x.%02x\r\n", (lut >> 16) & 0x1f, (lut >>  8) & 0x1f, (lut >>  0) & 0x1f);

    error = 0;
    break;
  }

  case 10:    //xtal_cap
    error = 0;
    if (argc == 0) {
      /* read: xtal_cap dac_vol adc_vol */
      chprintf(chp,"xtal_cap=%d dac_vol=%d@0x%08lx adc_vol=%d@0x%08lx\r\n", HS_ANA->REGS.CCON_XTAL,
               (int)((HS_CODEC->DAC_VOL_CTRL >> 16) & 0xff), &HS_CODEC->DAC_VOL_CTRL, 
               (int)((HS_CODEC->ADC_VOL_CTRL >> 16) & 0xff), &HS_CODEC->ADC_VOL_CTRL);
      break;
    }
    if (argc >= 1) {
      HS_ANA->REGS.CCON_XTAL = atoi(argv[1]);
      chprintf(chp, "HS_ANA->REGS.CCON_XTAL=0x%x\r\n", HS_ANA->REGS.CCON_XTAL);
    }
    if (argc >= 2) {
      int vol = atoi(argv[2]);
      HS_CODEC->DAC_VOL_CTRL = (1 << 24u/*update*/) | (vol << 16u) | (vol << 8u) | (HS_CODEC->DAC_VOL_CTRL & 0xff);
      chprintf(chp, "DAC_VOL_CTRL=0x%08lx\r\n", HS_CODEC->DAC_VOL_CTRL);
    }
    if (argc >= 3) {
      int vol = atoi(argv[3]);
      HS_CODEC->ADC_VOL_CTRL = (1 << 24u/*update*/) | (vol << 16u) | (vol << 8u) | (HS_CODEC->ADC_VOL_CTRL & 0xff);
      chprintf(chp, "ADC_VOL_CTRL=0x%08lx\r\n", HS_CODEC->ADC_VOL_CTRL);
    }
    break;

  case 11:  //flash disturb
    if (argc >= 2) {
      int window = atoi(argv[1]);
      int interval = atoi(argv[2]);
      int ii, count = 10;
      if (interval < window)
        interval = window;
      if (argc >= 3)
        count = atoi(argv[3]);
      chprintf(chp, "win=%d int=%d cnt=%d\r\n", window, interval, count);
      for (ii = 0; ii < count; ii++) {
        nds32_dcache_flush();
        memcpy((void *)0x4/*rom*/, (void *)0x80000000/*sflash*/, window*8000/48);
        osDelay(interval - window);
      }
    }
    break;

  default:
    error = 1;
    break;
  }

  HS_BTPHY->SPI_APB_SWITCH=1;
  if (error)
    _btrf_help(chp);
}

void HS6601_mac6200_init(void)
{
  HS_BTPHY->SPI_APB_SWITCH=0;
  HWradio_init();

#if 0
  /* turn off parts of BT RX manually because workaround BT's RXENA fsm will turn on these */
  /* [27][11]pd_rxmix: 1 */
  /* [25][9]pd_rxgm:   1 */
  /* [24][8]pd_rxlna:  1 */
  /* [23][7]pd_rxagc:  1 */
  HS_ANA->PD_CFG[2] &= ~((1 << 27) | (1 << 25) | (1 << 24) | (1 << 23));
  HS_ANA->PD_CFG[2] |=  ((1 << 11) | (1 <<  9) | (1 <<  8) | (1 << 7));
  /* turn off BT's rx agc of digital */
  /* rx_gain: [15:14]lna_gain [13]shrt_lna [12]sw_lna [11]en_att [10:9]att_ctrl [8:6]tca_ctrl [5]en_gm [4:2]tia_ctrl(by fm agc) [1:0]filt_gain(0 for fm) */
  HS_ANA->RX_AGC_CFG[4] = 0xA2600000; //[16]rx_gain_flag ...: 0=reg
#endif

  _HS6601_VCO_AFC_Calibration();

#if 0
  /* workaround: keep BT's RXENA high to un-reset SDM */
  HS_BTPHY->ANALOGUE[0x30] = (1<<7) + (1<<4); //[7]rx_start: w1 trigger to start rx fsm
#endif

  HS_BTPHY->IF_REG = 0x300; //0x300=750k for bt & rf62; 0x80=125k for fm
  HS_ANA->INTER_FREQ = 0x340000; //0x340000=750k for bt & rf62 on analog

  HS_BTPHY->SPI_APB_SWITCH=1;
}

#endif  /*HAL_USE_BTRF*/
