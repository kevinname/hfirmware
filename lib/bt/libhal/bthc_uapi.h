/**
***********************************************************
* Copyright (C) 2014 All rights reserved.
*
* @file    : bthc_uapi.h
* @brief   : BT Host Controller user API by Huntersun
* @version : 1.0
* @date    : 2015/9/15 19:33:07
* @author  : chenzhiyuan
*
* @note    :
*
* @history :
*
***********************************************************
*/

#ifndef __BTHC_UAPI_H
#define __BTHC_UAPI_H

#include <stdint.h>

#if defined __cplusplus
extern "C" {
#endif
	
struct bufferSize_s {
  uint16_t aclDataPacketLength;
  uint16_t scoDataPacketLength;
  uint16_t numAclDataPackets;
  uint16_t numScoDataPackets;
};

typedef enum
{
    BT_LOW_POWER_ENTER_SNIFF_MODE                      =0x00,       /*exit sniff transition mode*/
    BT_LOW_POWER_EXIT_SNIFF_MODE                            ,       /*enter sniff transition mode*/
}bt_low_power_t;

__PACKED struct bt_hc_sys_config_s {
  uint8_t  bd_addr[6];              //@0x00
  uint16_t HCI_revision;            //@0x06
  uint16_t comp_id;
  uint16_t lmp_subversion;
  uint8_t  lmp_version;             //@0x0c
  uint8_t  HCI_version;

  /* static buffers or allocated buffers in OS */
  uint8_t max_active_devices;
  uint8_t max_active_devices_in_piconet;
  struct bufferSize_s hc_buffer_size;      //@0x10
  struct bufferSize_s hc_buffer_size_in;   //@0x18

  /* debug */
  uint64_t local_device_syncword;   //@0x20
  uint32_t rand_seed;               //@0x28
  uint32_t win_ext;                 //@0x2C
  uint8_t  erroneous_data_reporting;//@0x30
  uint8_t  hs_cap;
#define HS_CAP_DIS_BQB_WA  (1 << 2)
  uint8_t  plc;
  uint8_t  xtal_cap;

  /* get/set via tc interface for CBT */
  uint8_t  clock_jitter_us;         //@0x34
  uint8_t  clock_drift_ppm;         
  uint8_t  data_whitening_enable;
  uint8_t  hopping_mode;
  uint32_t tx_freq;                 //@0x38
  uint32_t rx_freq;                 //@0x3C

  /* pre-compile time settings. */
  uint8_t feature_set[16];          //@0x40
  uint8_t hci_command_set[64];      //@0x50
} __PACKED_GCC;

__PACKED struct bt_hc_phy_config_s {
  uint8_t phy_type;
  uint8_t debug_mode;
#define RADIO_DEBUG_RXENA_GIO  (0 << 0)
#define RADIO_DEBUG_RXENA_SPI  (1 << 0)
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
  uint8_t win_ext;
  uint8_t le_win_ext;

  int8_t  rx_golden_rssi_min;
  int8_t  rx_golden_rssi_max;
  int8_t  rssi_adjust;
  uint8_t rssi_mode;

  uint8_t low_power;
  uint8_t rsv4;
  uint8_t epc_max_tx_power_difference;
  uint8_t epc_max_tx_power_threshold;

  int8_t  tx_power_level_min;
  int8_t  tx_power_level_max;
  uint8_t tx_power_level_units; //Class 1: 25 levels, Class 2: 17 levels, Class 3: 15 levels
  uint8_t tx_power_level_step;  //min: 2dB, max: 8dB

  uint8_t power_ctrl_tab[32];
  int8_t  power_ctrl_tab_dBm[32];

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

  /* RC calibration parameter */
  uint16_t rc_counter;     //total RC calibration counters
  uint16_t rx_adc_t1_t2[16];    // rc value of rxadc. from max to min,and the corresponding tune from min->max    
  uint16_t tx_dac_t1_t2[16];    // rc value of txdac
  uint16_t rx_filter_t1_t2[8];  // rc value of rx filter 
};

typedef struct bt_hc_sys_config_s bt_hc_sys_config_t;
typedef struct bt_hc_phy_config_s bt_hc_phy_config_t;


/*
 * @brief  Get major system and RF configuration structure in the Host Controller Stack.
 *
 * @param[out] pp_sys_cfg: pointer to the pointer of g_sys_config
 * @param[out] pp_phy_cfg: pointer to the pointer of g_sys_rf_cfg
 * 
 * @return void
 */
void HCI_Generic_Config(bt_hc_sys_config_t **pp_sys_cfg, bt_hc_phy_config_t **pp_phy_cfg);

/*
 * @brief  Used to initialise the Transport in the Host Controller Stack.
 * @note   This function is not re-entrant.
 *
 * @param[in] num_in_acl_packets:  queue depth for incoming ACL data (packets), obsoleted
 * @param[in] num_out_acl_packets: queue depth for outgoing ACL data (packets), obsoleted
 * @param[in] num_in_sco_packets:  queue depth for incoming SCO data (packets), obsoleted
 * @param[in] num_out_sco_packets: queue depth for outgoing SCO data (packets), obsoleted
 * @param[in] hc_malloc:           external memory allocator, void *malloc(size_t size)
 * @param[in] hc_free:             external memory allocator, void free(void *blk)
 *
 * @return 0   initialisation completed successfully
 *         1   transport system already active
 *        -1   initialisation failed on memory allocation
 *        -2   hardware(RF) self diagnosis failure
 */
int HCI_Generic_Initialise( uint16_t num_in_acl_packets,
                            uint16_t num_out_acl_packets,
                            uint16_t num_in_sco_packets,
                            uint16_t num_out_sco_packets,
                            void    *hc_malloc,
                            void    *hc_free);

/*
 * @brief  Used to shutdown the Transport in the Host Controller Stack.
 * @note   This function is not re-entrant.
 *
 * @return 0   if shutdown completed successfully
 *         1   if transport system not active
 */
int HCI_Generic_Shutdown(void);

/*
 * @brief  Generic HCI registration of HC pend/post event handlers.
 * @note   Used to register the OS wrappers and enable event driven Host Controller.
 *
 * @param[in] hc_post: set event flags to wakeup hc thread, void HC_Event_Post(uint32_t mask)
 * @param[in] hc_pend: sleep on and get event flags,        uint32_t HC_Event_Pend(uint32_t mask, uint16_t timout)
 * @return void
 */
#define HCI_Generic_Register_HC_Event_Handlers() \
        HCI_Generic_Register_HC_Event_Handlers_Ex(HC_Event_Post,HC_Event_Pend,10)
        
void HCI_Generic_Register_HC_Event_Handlers_Ex(void (*hc_post)(uint32_t),
                  uint32_t (*hc_pend)(uint32_t, uint16_t), uint16_t hc_timeout);

/*
 * @brief  Registers the function to be called back to dispatch Events or Incoming data to the Host.
 * @note   hc2host
 *
 * @param[in] cb: callback function of hc2host to dispatch Events, incomming ACL/SCO data
 *
 * @return void
 */
void HCI_Generic_Register_Tx_Callback(void (*cb)(uint8_t *data, uint32_t pdu_len, uint8_t *header, uint8_t head_len));

/*
 * @brief  Acknowledge the completion of tranmission.
 * @note   hc2host
 * @note   This function should be called once a PDU has been successfully delivered to
 *         the Host. Always occurs as a result of the invokation of the host callback function
 *         registered in "HCI_Generic_Register_Tx_Callback"
 *
 * @param[in] buf_type: Identifies the type of the packet being acknowledged.
 *                      HCI_EVENT | HCI_ACLDATA | HCI_SCODATA
 * @param[in] buf_len:  The length of the buffer being acknowledged
 *
 * @return void
 */
void HCI_Generic_Acknowledge_Complete_Tx(uint8_t buf_type, uint32_t buf_len);

/*
 * @brief  Host gets a buffer from the queuing system in the Host Controller.
 * @note   host2hc
 * @note   Once the host has written the pdu into this buffer it should commit it using "HCI_Generic_Commit_Rx_Buf"
 *
 * @param[in] buf_type: indicates if the data contains a HCI Command, HCI ACL Data or HCI SCO Data
 *                      HCI_COMMAND | HCI_ACLDATA | HCI_SCODATA
 * @param[in] size:     Length of overall Command INCLUDING the bytes for the header, or 
 *                      Length of the data section of ACL/SCO packet.
 * @param[in] hci_header: Header of HCI packet
 *
 * @return the buffer pointer into which the HCI Command parameters or ACL/SCO Data bytes are to be written
 */
uint8_t* HCI_Generic_Get_Rx_Buf(uint8_t buf_type, int size, uint8_t *hci_header);

/*
 * @brief  Host commits a buffer allocated by HC_Generic_Get_Rx_Buf to the queueing system.
 * @note   host2hc
 *
 * @param[in] buf_type: HCI_COMMAND | HCI_ACLDATA | HCI_SCODATA
 * @return void
 */
void HCI_Generic_Commit_Rx_Buf(uint8_t buf_type);

/*
 * @brief  Host checks whether the CPU sleep is available or not in the HC.
 * @brief  Host can make the processor sleep if no event/activities in the HC.
 *
 * @return 0   if there are events/activities in the Host Controller.
 *         1   if there is no event/activity in the Host Controller.
 */
uint8_t HCI_Generic_HC_Monitor(void);

/*
 * @brief  Host checks whether the CPU halt is available or not in the HC.
 * @brief  Host can make the processor standby/halt if sniff/hold/park mode.
 *
 * @return 0   if the Host Controller does not allow to halt system.
 *         1   if system halt is avalable in the Host Controller.
 */
uint8_t HCI_Generic_LC_Monitor(void);

#if defined __cplusplus
}
#endif

#endif

