#ifndef __NRF24L01_X_H__
#define __NRF24L01_X_H__
/**************************************************/

/**************************************************
nRF24L01_X.h;
说明：
nRF24L01芯片应用程序接口，包含寄存器地址、SPI通信函数、
芯片命令函数。
须先行定义nRF24L01接口引脚 CE、CSN、CSK、MOSI、MISO、IRQ，
和nRF24L01相关变量。
一般放置在nRF24L01相关定义的最后，或宏及全局变量定义之后，
其他函数定义之前即可。
**************************************************/

#include "HS6200_types.h"
#include "nRF24L01_common.h"
#include "hal.h"

/**************************************************
              MCU 中有关 nRF24L01 的接口
**************************************************/
/**************************************************/
//// nRF24L01_0 端口引脚定义
//sbit nRF24L01_0_CE   = P0^0;  // Chip Enable pin signal (output)
//sbit nRF24L01_0_CSN  = P0^3;  // Slave Select pin, (output to nRF24L01_1_CSN, nRF24L01)
//sbit nRF24L01_0_SCK  = P0^1;  // Master Out, Slave In pin (output)
//sbit nRF24L01_0_MOSI = P1^0;  // Serial Clock pin, (output)
//sbit nRF24L01_0_MISO = P0^2;  // Master In, Slave Out pin (input)
//sbit nRF24L01_0_IRQ  = P0^6;  // Interrupt signal, from nRF24L01 (input)
//#define nRF24L01_0_EI  EX0
//
//// nRF24L01_1 端口引脚定义
//sbit nRF24L01_1_CE   = P1^1;  // Chip Enable pin signal (output)
//sbit nRF24L01_1_CSN  = P1^4;  // Slave Select pin, (output to nRF24L01_1_CSN, nRF24L01)
//sbit nRF24L01_1_SCK  = P1^2;  // Master Out, Slave In pin (output)
//sbit nRF24L01_1_MOSI = P1^5;  // Serial Clock pin, (output)
//sbit nRF24L01_1_MISO = P1^3;  // Master In, Slave Out pin (input)
//sbit nRF24L01_1_IRQ  = P0^7;  // Interrupt signal, from nRF24L01 (input)
//#define nRF24L01_1_EI  EX1


extern U8 ce_low_befor_write;
extern U8 ce_state_before_flush;

#define nRF24L01_0       0x00
#define nRF24L01_1       0x01


#define DEV_0   nRF24L01_0            //DEV_0  correspond  nRF24L01_0
#define DEV_1   nRF24L01_1            //DEV_1  correspond  nRF24L01_1



typedef union 
{
    U8 value;
    
    struct
    {
    U8 RSVD      :   1; // BIT_0  
    U8 MR        :   1; // BIT_1
    U8 AR        :   1; // BIT_2
    U8 ACKPAY    :   1; // BIT_3
    U8 EN_ACK    :   1; // BIT_4
    U8 SA_DA     :   1; // BIT_5
    U8 SPLDPL    :   1; // BIT_6
    U8 PTXPRX    :   1; // BIT_7
    } bf;
} rf_config_t;



// 通信模式
// x_x_x_x_x_xxx
// 0: PTX                         1:  PRX
// 0: SPL:static packet length    1:  DPL:dynamic packet length
// 0: SA:static ACK               1:  DA: dynamic ACK
// 0: NA: No ACK                  1:  AK: ACK
// 0: AE: ACK empty               1:  AP: ACK payload
// AR: auto retransmit
// MR: manual tx               


#define PTX_SPL_SA_NA_AE_AR_NMR_X        0x04 // 0_000Z_Z0_0 // SB_MODE
#define PRX_SPL_SA_NA_AE_AR_NMR_X        0x84 // 1_000Z_Z0_0 // SB_MODE

#define PTX_SPL_SA_AK_AE_AR_NMR_X        0x14 // 0_0010_10_0
#define PRX_SPL_SA_AK_AE_AR_NMR_X        0x94 // 1_0010_10_0

#define PTX_SPL_DA_NA_AE_AR_NMR_X        0x24 // 0_010Z_Z0_0
#define PRX_SPL_DA_NA_AE_AR_NMR_X        0xa4 // 1_010Z_Z0_0

#define PTX_SPL_DA_AK_AE_AR_NMR_X        0x34 // 0_0110_10_0
#define PRX_SPL_DA_AK_AE_AR_NMR_X        0xb4 // 1_0110_10_0

#define PTX_DPL_SA_AK_AE_AR_NMR_X        0x54 // 0_1010_10_0
#define PRX_DPL_SA_AK_AE_AR_NMR_X        0xd4 // 1_1010_10_0

#define PTX_DPL_SA_AK_AP_AR_NMR_X        0x5c // 0_1011_10_0
#define PRX_DPL_SA_AK_AP_AR_NMR_X        0xdc // 1_1011_10_0

#define PTX_DPL_DA_NA_AE_AR_NMR_X        0x64 // 0_110Z_Z0_0
#define PRX_DPL_DA_NA_AE_AR_NMR_X        0xe4 // 1_110Z_Z0_0

#define PTX_DPL_DA_AK_AE_AR_NMR_X        0x74 // 0_1110_10_0
#define PRX_DPL_DA_AK_AE_AR_NMR_X        0xf4 // 1_1110_10_0

#define PTX_DPL_DA_AK_AP_AR_NMR_X        0x7c // 0_1111_10_0
#define PRX_DPL_DA_AK_AP_AR_NMR_X        0xfc // 1_1111_10_0


extern rf_config_t nRF24L01_X_CurrentConfig[2];


// nRF24L01 相关设置
#define nRF24L01_X_ADR_WIDTH   5  // 发送/接收地址宽度，只能选择 3、4、5 字节，一般选择最大宽度 5 字节

// *********!!!!!务必注意：左边为低字节，右边为高字节。!!!!!**********
extern U8  nRF24L01_X_TX_ADDRESS[2][5];       // 定义发送通道地址
                                                 

extern U8  nRF24L01_X_RX_ADDRESS_P[2][6][5];      //define RX pipe address   

#define RF_CH_TABLE_LENGTH      16
extern U8  nRF24L01_X_RF_CH[RF_CH_TABLE_LENGTH];                                                
                                              
#define nRF24L01_X_MAX_PL_WIDTH   32   

extern U8  nRF24L01_X_PLOAD_WIDTH[6];

extern U8  nRF24L01_X_Tx_Payload_Width[2] ;  // 发送通道有效数据宽度：无。发送端有效数据宽度由写入 TX_FIFO 中的字节数决定，故写入时需注意使之与接收端协定的数据宽度相等（静态数据宽度模式下）。
extern U8  nRF24L01_X_Rx_Payload_Width[2][6];  // 接收通道 0 有效数据宽度：0~32字节

extern U8  nRF24L01_X_Tx_Buf[2][nRF24L01_X_MAX_PL_WIDTH]; // 发送通道数据缓冲区，用于存放向 nRF24L01 发送 FIFO 写入的数据，宽度应与协定好的接收方数据宽度相等。最多 32 字节。
extern U8  nRF24L01_X_Rx_Buf[2][6][nRF24L01_X_MAX_PL_WIDTH+0x01]; // 接收通道 P0-P5 数据缓冲区，用于存放从 nRF24L01 接收 FIFO 读出的数据，原则上应取各接收通道有效数据宽度的最大值的 3 倍，最多 32 字节的 3 倍，因为 nRF24L01 的 FIFO 为 3 级。

extern U8  nRF24L01_X_Common_BUF[2][32];
extern U8  nRF24L01_X_Busy[2];

extern U8  nRF24L01_X_Tx_ACK_Payload_Buf[2][nRF24L01_X_MAX_PL_WIDTH];    //waitting for transmit ACK payload buffer
extern U8  nRF24L01_X_Rx_ACK_Payload_Buf[2][nRF24L01_X_MAX_PL_WIDTH];    //received ACK payload buffer
extern U8  nRF24L01_X_Tx_ACK_Payload_Width[2];     //width of Tx ACK payload buffer 
extern U8  nRF24L01_X_Rx_ACK_Payload_Width[2];     //width of Rx ACK payload buffer

extern U8  nRF24L01_X_PipeNum[2];

extern nRF24L01_PipeAddr_tag nRF24L01_X_RX_ADDR_P0_RESET_VALUE_struct;  //pipe0 address 0xE7E7E7E7E7
extern nRF24L01_PipeAddr_tag nRF24L01_X_RX_ADDR_P1_RESET_VALUE_struct;  //pipe1 address 0xC2C2C2C2C2
extern nRF24L01_PipeAddr_tag nRF24L01_X_TX_ADDR_RESET_VALUE_struct;  

/*----------------------------------function prototype----------------------------------------*/

extern void nRF24L01_X_ce_high(U8 DevNum);
extern void nRF24L01_X_ce_low(U8 DevNum);
extern bool_t nRF24L01_X_is_ce_high(U8 DevNum);
extern void nRF24L01_X_csn_high(U8 DevNum);
extern void nRF24L01_X_csn_low(U8 DevNum);
extern void nRF24L01_X_csn_high_restore_int(U8 DevNum);
extern void nRF24L01_X_csn_low_disable_int(U8 DevNum);
extern void nRF24L01_X_ce_high_pulse(U8 DevNum);


extern U8 nRF24L01_X_spi_rw(U8 DevNum, U8 byte);

extern U8 nRF24L01_X_read_reg(U8 DevNum, U8 reg);
extern U8 nRF24L01_X_write_reg(U8 DevNum, U8 reg, U8 value);
extern U8 nRF24L01_X_read_reg_multibytes(U8 DevNum, U8 Reg, U8 *pBuf, U8 bytes);
extern U8 nRF24L01_X_write_reg_multibytes(U8 DevNum, U8 Reg, U8 *pBuf, U8 bytes);
extern U8 nRF24L01_X_write_reg_multibytes_no_ce(U8 DevNum, U8 Reg, U8 *pBuf, U8 bytes);

extern U8 nRF24L01_X_read_pipe_addr(U8 DevNum, U8 Px_Addr_Reg, U8 *pPipeAddr, U8 Addr_Width);
extern U8 nRF24L01_X_write_pipe_addr(U8 DevNum, U8 Px_Addr_Reg, U8 *pPipeAddr, U8 Addr_Width);

extern U8 nRF24L01_X_read_rx_payload(U8 DevNum, U8 *pBuf, U8 bytes);
extern U8 nRF24L01_X_write_tx_payload(U8 DevNum, U8 *pBuf, U8 bytes);

extern U8 nRF24L01_X_flush_rx(U8 DevNum);
extern U8 nRF24L01_X_flush_tx(U8 DevNum);

extern U8 nRF24L01_X_reuse_tx_payload(U8 DevNum);
extern U8 nRF24L01_X_activate(U8 DevNum);
extern U8 nRF24L01_X_read_rx_payload_width(U8 DevNum);

extern U8 nRF24L01_X_write_ack_payload(U8 DevNum, U8 PipeNum, U8 *pBuf, U8 bytes);  //write ACK payload
extern U8 nRF24L01_X_write_tx_payload_noack(U8 DevNum, U8 *pBuf, U8 bytes);

extern U8 nRF24L01_X_nop(U8 DevNum);


/**************************************************
                 nRF24L01 应用函数
**************************************************/

extern void nRF24L01_X_rx_fifo_block_flush(U8 DevNum);

/*-------------------nRF24L01 模式函数-----------------------*/
extern void nRF24L01_X_set_mask_rx_dr(U8 DevNum, U8 mask_rx_dr);
extern void nRF24L01_X_set_mask_tx_ds(U8 DevNum, U8 mask_tx_ds);
extern void nRF24L01_X_set_mask_max_rt(U8 DevNum, U8 mask_max_rt);
extern void nRF24L01_X_set_crc(U8 DevNum, U8 crc);
extern U8 nRF24L01_X_get_crc(U8 DevNum);
extern void nRF24L01_X_set_powerup(U8 DevNum);
extern void nRF24L01_X_set_powerdown(U8 DevNum);
extern void nRF24L01_X_set_prx(U8 DevNum);
extern void nRF24L01_X_set_ptx(U8 DevNum);
extern bool_t nRF24L01_X_is_prx(U8 DevNum);
extern bool_t nRF24L01_X_is_ptx(U8 DevNum);
extern void nRF24L01_X_set_en_auto_ack_all(U8 DevNum, U8 en);
extern void nRF24L01_X_set_en_auto_ack_pipe(U8 DevNum, U8 pipe, U8 en);
extern U8 nRF24L01_X_is_auto_ack_px(U8 DevNum, U8 pipe);
extern void nRF24L01_X_set_en_rx_pipe_all(U8 DevNum, U8 en);
extern void nRF24L01_X_set_en_rx_pipe(U8 DevNum, U8 pipe, U8 en);
extern void nRF24L01_X_set_rx_pipe_addr(U8 DevNum, U8 pipe, U8 *pPipeAddr, U8 addr_width);
extern void nRF24L01_X_set_rx_pipe_static_pay_load_width(U8 DevNum, U8 pipe, U8 static_plw);
extern U8 nRF24L01_X_read_rx_pipe_static_pay_load_width(U8 DevNum,U8 pipe);
extern void nRF24L01_X_set_address_width(U8 DevNum, U8 AW);
extern U8 nRF24L01_X_get_aw(U8 DevNum);
extern void nRF24L01_X_set_auto_retr_delay(U8 DevNum, U8 ARD);
extern U8 nRF24L01_X_get_ard(U8 DevNum);       //auto retransmit delay
extern void nRF24L01_X_set_auto_retr_count(U8 DevNum, U8 ARC);  //auto retransmit count
extern U8 nRF24L01_X_get_arc(U8 DevNum);    //get auto retransmit count
//extern void nRF24L01_X_set_auto_retr_disable(DevNum);  //disable ARC(auto retransmit function)

extern void nRF24L01_X_set_rf_ch(U8 DevNum, U8 RF_CH);  //set RF channel
extern U8 nRF24L01_X_get_rf_ch(U8 DevNum);    //get RF channel
extern void nRF24L01_X_set_cont_wave(U8 DevNum, U8 CONT_WAVE); //contious carrier wave transmit
extern U8 nRF24L01_X_is_cont_wave(U8 DevNum); 
extern void nRF24L01_X_set_pll_lock(U8 DevNum, U8 PLL_LOCK);   //force PLL lock  used in test
extern U8 nRF24L01_X_is_pll_lock(U8 DevNum);

extern void nRF24L01_X_set_rf_datarate(U8 DevNum, U8 RF_DR);  //set RF datarate
extern U8 nRF24L01_X_get_rf_datarate(U8 DevNum);
extern void nRF24L01_X_set_rf_output_power(U8 DevNum, U8 RF_PWR);  //set RF out power in TE mode
extern void nRF24L01_X_set_lna_gain(U8 DevNum, U8 LNA_HCURR); //set LNA(Low Noise Amplifer ) gain
extern U8 nRF24L01_X_read_rx_p_no(U8 DevNum); //RX FIFO status (which pipe is avilable)
extern U8 nRF24L01_X_is_rx_empty(U8 DevNum);  //RX FIFO is empty? 
extern U8 nRF24L01_X_is_tx_full(U8 DevNum);   //TX FIFO is full?
extern U8 nRF24L01_X_is_rx_fifo_full(U8 DevNum);  //RX FIFO is full
extern U8 nRF24L01_X_is_rx_fifo_empty(U8 DevNum);  //RX FIFO is empty?
extern U8 nRF24L01_X_is_tx_reuse(U8 DevNum);       //retransmit TX
extern U8 nRF24L01_X_is_tx_fifo_full(U8 DevNum);   //TX FIFO is FIFO
extern U8 nRF24L01_X_is_tx_fifo_empty(U8 DevNum);  //TX FIFO empty
extern U8 nRF24L01_X_get_plos_cnt(U8 DevNum);   //get PLOS(Packet lost) count
extern U8 nRF24L01_X_get_arc_cnt(U8 DevNum);   //get counters retransmit packets
extern U8 nRF24L01_X_is_rpd(U8 DevNum);   //RPD is Receive power detect?
extern void nRF24L01_X_set_en_dpl_all(U8 DevNum, U8 en);   //enable all pipes to dynamic payload
extern void nRF24L01_X_set_en_dpl_px(U8 DevNum, U8 pipe, U8 en);  //enable pipes to dynamic payload
extern void nRF24L01_X_set_en_dpl_feature(U8 DevNum, U8 en);//set EN_DPL
extern U8 nRF24L01_X_is_dpl_px(U8 DevNum, U8 pipe);   //pipe is dynamic?
extern U8 nRF24L01_X_is_dpl_feature(U8 DevNum);  //is enable dynamic (EN_DPL)?
extern void nRF24L01_X_set_en_ack_pay_feature(U8 DevNum, U8 en);  //enable ack payload
extern U8 nRF24L01_X_is_ack_pay_feature(U8 DevNum);
extern void nRF24L01_X_set_en_dyn_ack_feature(U8 DevNum, U8 en);
extern U8 nRF24L01_X_is_dyn_ack_feature(U8 DevNum);
extern void nRF24L01_X_set_standby_I(U8 DevNum);
extern void nRF24L01_X_set_standby_II(U8 DevNum);
extern void nRF24L01_X_clear_int_flags(U8 DevNum); //clear int flags in status register
extern void nRF24L01_X_clear_plos_cnt(U8 DevNum);

extern void nRF24L01_X_set_pipe(U8 DevNum, U8 PipeNum, U8 en_pipe, U8 en_auto_ack, U8 *pPipeAddr, U8 addr_width, U8 static_plw, U8 en_dpl);

extern void nRF24L01_X_send_packet(U8 DevNum, U8 *pBuf, U8 plw, U8 en_ack);
extern void nRF24L01_X_send_packet_by_mode(uchar DevNum, uchar PipeNum, uchar *pBuf, uchar plw);
extern void nRF24L01_X_common_setting(U8 DevNum); // CRC, ARC, ARD, RF_CH, etc.
extern void nRF24L01_X_ptx_init(U8 DevNum);
extern void nRF24L01_X_prx_init(U8 DevNum);

extern void nRF24L01_X_write_reg_bits(U8 DevNum, U8 reg, U8 bit_mask, U8 bit_);
extern void nRF24L01_X_prx_mode(U8 DevNum);
extern void nRF24L01_X_ptx_mode(U8 DevNum);
extern void nRF24L01_X_powerdown(U8 DevNum);
extern void nRF24L01_X_powerup(U8 DevNum);
extern void nRF24L01_X_standby_I(U8 DevNum);
extern U8 nRF24L01_X_standby_II(U8 DevNum);

extern void nRF24L01_X_spl_mode(U8 DevNum);  //静长
extern void nRF24L01_X_dpl_mode(U8 DevNum);  //动长
extern void nRF24L01_X_sa_mode(U8 DevNum);   //静答
extern void nRF24L01_X_da_mode(U8 DevNum);    //动答
extern void nRF24L01_X_ack_mode(U8 DevNum);   //有答
extern void nRF24L01_X_noack_mode(U8 DevNum);  //无答
extern void nRF24L01_X_ack_empty_mode(U8 DevNum);  //空答
extern void nRF24L01_X_ack_pay_mode(U8 DevNum);  //载答
extern void nRF24L01_X_noretr_mode(U8 DevNum);  //无重发模式
extern void nRF24L01_X_ar_mode(U8 DevNum, U8 arc);  //自动重发模式
extern void nRF24L01_X_mr_mode(U8 DevNum, U8 en);   //手动重发模式



extern U8 nRF24L01_X_get_rf_ch_idx(const U8 rf_ch, U8 *rf_ch_table, const U8 table_length);  //get RF channel
extern U8 nRF24L01_X_get_next_rf_ch(U8 current_rf_ch, U8 *rf_ch_table, U8 table_length);  //get next RF channel
extern bool_t nRF24L01_X_is_rf_ch_clean(U8 DevNum, U8 rf_ch, U8 gate);    //rf_ch is clean?
extern bool_t nRF24L01_X_is_rf_ch_dirty(U8 DevNum, U8 rf_ch, U8 gate);   //rf_ch is dirty
extern U8 nRF24L01_X_search_next_clean_rf_ch(U8 DevNum, U8 current_rf_ch, U8 *rf_ch_table, U8 table_length, U8 gate);  //search next clean channel
extern U8 nRF24L01_X_search_next_dirty_rf_ch(U8 DevNum, U8 current_rf_ch, U8 *rf_ch_table, U8 table_length, U8 gate);  //search next dirty channel
extern U8 nRF24L01_X_search_best_rf_ch(U8 DevNum, U8 current_rf_ch, U8 *rf_ch_table, U8 table_length, U8 gate_max);   //search best RF channel
extern U8 nRF24L01_X_search_worst_rf_ch(U8 DevNum, U8 current_rf_ch, U8 *rf_ch_table, U8 table_length, U8 gate_min);   //get worst RF channel




extern U8 nRF24L01_X_ptx_fh(U8 DevNum, U8 *rf_ch_table, U8 table_length);
extern U8 nRF24L01_X_prx_fh(U8 DevNum, U8 *rf_ch_table, U8 table_length);
extern void nRF24L01_X_DisplayRegValue(U8 DevNum, U8 reg);  //display regval
extern U8 nRF24L01_X_test_Reg_ReadVSValue(U8 DevNum, U8 reg, U8 value); //test register value
extern U8 nRF24L01_X_test_Reg_PipeAddr_ReadVSValue(U8 DevNum, U8 Px_Addr_Reg, nRF24L01_PipeAddr_tag PipeAddr, U8 Addr_Width);  //test pipe addr
extern U8 nRF24L01_X_test_Reg_ReadVSWrite(U8 DevNum, U8 reg, U8 value); //test write register value
extern U8 nRF24L01_X_test_Reg_PipeAddr_ReadVSWrite(U8 DevNum, U8 Px_Addr_Reg, nRF24L01_PipeAddr_tag PipeAddr, U8 Addr_Width); //test write pipe address value 

extern void nRF24L01_X_test_Reg_ResetValue_all(U8 DevNum);   
extern void nRF24L01_X_test_Reg_ReadVSWrite_all(U8 DevNum);

extern U8 nRF24L01_X_read_fifo_status(U8 DevNum);
extern U8 nRF24L01_X_read_cfg_word(U8 DevNum);


/**************************************************/

#endif /*__NRF24L01_X_H__*/


/*--------------------------------------End Of File---------------------------------------------*/
