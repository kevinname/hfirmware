/**************************************************
nRF24L01_X.c;
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
#include "nRF24L01_X.h"
#include "stdlib.h"
#include "HS6200_Reg.h"
#include "HS6200_Analog_Test.h"
#include "HS6200_test_sys.h"
#include "hal.h"

/**************************************************/
U8 ce_low_befor_write = 0; 
U8 ce_state_before_flush=0x00;

rf_config_t nRF24L01_X_CurrentConfig[2];

// 定义发送通道地址
U8  nRF24L01_X_TX_ADDRESS[2][5]      =  { /* LSByte 在前, MSByte在后 */
    {0x10,0x43,0x10,0x11,0x34},  
    {0x00,0x43,0x10,0x11,0x34}
}; 

U8  nRF24L01_X_RX_ADDRESS_P[2][6][5] = {
   {
      {0x00,0x43,0x10,0x11,0x34},  // 定义接收通道 0 地址，
      {0x01,0x43,0x10,0x11,0x34},  // 定义接收通道 1 地址，
      {0x02,0x43,0x10,0x11,0x34},  // 定义接收通道 2 地址，仅最低字节，高字节与通道 1 相同。
      {0x03,0x43,0x10,0x11,0x34},  // 定义接收通道 3 地址，仅最低字节，高字节与通道 1 相同。
      {0x04,0x43,0x10,0x11,0x34},  // 定义接收通道 4 地址，仅最低字节，高字节与通道 1 相同。
      {0x05,0x43,0x10,0x11,0x34}   // 定义接收通道 5 地址，仅最低字节，高字节与通道 1 相同。
   }, 
   {
      {0x10,0x43,0x10,0x11,0x34}, 
      {0x11,0x43,0x10,0x11,0x34}, 
      {0x12,0x43,0x10,0x11,0x34}, 
      {0x13,0x43,0x10,0x11,0x34}, 
      {0x14,0x43,0x10,0x11,0x34}, 
      {0x15,0x43,0x10,0x11,0x34}
   }
};

U8  nRF24L01_X_RF_CH [RF_CH_TABLE_LENGTH] = { 7,  112,   56,   91,   42,   70,  105,   14,   63,   98,   35,   77,   21,   84,   28,   49};                                                
                                               // {0x07, 0x70, 0x38, 0x5B, 0x2A, 0x46, 0x69, 0x0E, 0x3F, 0x62, 0x23, 0x4D, 0x15, 0x54, 0x1C, 0x31}

// 接收通道 0 有效数据宽度：0~32字节
U8  nRF24L01_X_PLOAD_WIDTH[6]={5, 5, 5, 5, 5, 5}; 
                                           
U8  nRF24L01_X_Tx_Payload_Width[2] ;  // 发送通道有效数据宽度：无。发送端有效数据宽度由写入 TX_FIFO 中的字节数决定，故写入时需注意使之与接收端协定的数据宽度相等（静态数据宽度模式下）。
U8  nRF24L01_X_Rx_Payload_Width[2][6] = {      // 接收通道 0 有效数据宽度：0~32字节
  {5, 5, 5, 5, 5, 5},
  {5, 5, 5, 5, 5, 5}
};

U8  nRF24L01_X_Tx_Buf[2][nRF24L01_X_MAX_PL_WIDTH]; // 发送通道数据缓冲区，用于存放向 nRF24L01 发送 FIFO 写入的数据，宽度应与协定好的接收方数据宽度相等。最多 32 字节。
U8  nRF24L01_X_Rx_Buf[2][6][nRF24L01_X_MAX_PL_WIDTH+0x01]; // 接收通道 P0-P5 数据缓冲区，用于存放从 nRF24L01 接收 FIFO 读出的数据，原则上应取各接收通道有效数据宽度的最大值的 3 倍，最多 32 字节的 3 倍，因为 nRF24L01 的 FIFO 为 3 级。
                                                                //第一字节存放pipe num

U8  nRF24L01_X_Tx_ACK_Payload_Buf[2][nRF24L01_X_MAX_PL_WIDTH];    //waitting for transmit ACK payload buffer
U8  nRF24L01_X_Tx_ACK_Payload_Width[2];     //width of Tx ACK payload buffer 

U8  nRF24L01_X_PipeNum[2];

U8  nRF24L01_X_Common_BUF[2][32];
U8  nRF24L01_X_Busy[2] = {0, 0};

/**************************************************
                                        端口函数
**************************************************/
void nRF24L01_X_ce_high(U8 DevNum)
{
    switch (DevNum)
    {
        case DEV_0:
            MAC6200_CE_High();
            break;
        case DEV_1:

            break;
    }
}

void nRF24L01_X_ce_low(U8 DevNum)
{
    switch (DevNum)
    {
        case DEV_0:
        	MAC6200_CE_Low();
            break;
        case DEV_1:

            break;
    }
}

bool_t nRF24L01_X_is_ce_high(U8 DevNum)
{
    bool_t inbit = 0;
    
    switch (DevNum)
    {
        case DEV_0:
        	inbit = (HS_MAC6200->RFCON & RF_CE);
            break;
        case DEV_1:

            break;
    }
    return (inbit);
}

void nRF24L01_X_csn_high(U8 DevNum)
{
    switch (DevNum)
    {
        case DEV_0:
        	MAC6200_CSN_High();
            break;
        case DEV_1:

            break;
    }
}

void nRF24L01_X_csn_low(U8 DevNum)
{
    switch (DevNum)
    {
        case DEV_0:
        	 MAC6200_CSN_Low();
            break;
        case DEV_1:

            break;
    }
}

void nRF24L01_X_csn_with_int_change(U8 DevNum, bool_t highlow)
{
//    static bool_t pre_ei0 = 0, pre_ei1 = 0;
    
    switch (DevNum)
    {
        case DEV_0:
            if (highlow)
            {
                nRF24L01_X_csn_high(DEV_0);
            }
            else
            {
                nRF24L01_X_csn_low(DEV_0);
            }
            break;
        case DEV_1:

            break;
    }
}

void nRF24L01_X_csn_high_restore_int(U8 DevNum)
{
    nRF24L01_X_csn_with_int_change(DevNum, 1);
}

void nRF24L01_X_csn_low_disable_int(U8 DevNum)
{
    nRF24L01_X_csn_with_int_change(DevNum, 0);
}

/**************************************************
函数：nRF24L01_X_ce_high_pulse(DevNum);

说明：
    产生 CE 高脉冲，保持 10us 以上。
**************************************************/
void nRF24L01_X_ce_high_pulse(U8 DevNum)
{
    nRF24L01_X_ce_high(DevNum);   // 置高
//    chThdSleepMicroseconds(20);          
//    nRF24L01_X_ce_low(DevNum);    // 置低
}
/**************************************************/

/**************************************************
函数：nRF24L01_X_init_port(DevNum);

说明：
    初始化 nRF24L01 的端口控制引脚。
**************************************************/
void nRF24L01_X_init_port(U8 DevNum)
{
    nRF24L01_X_ce_low(DevNum);   // RF disable
    nRF24L01_X_csn_high(DevNum); // Spi disable
//    nRF24L01_X_sck_low(DevNum);  // Spi clock line init low
}
/**************************************************/

/**************************************************
                                        SPI 通信函数
**************************************************/
/**************************************************
函数：nRF24L01_X_spi_rw(DevNum);

说明：
    按照 SPI 协议，向 nRF24L01 写入一个字节，
    并返回在写入的同时从其读出的一个字节。
**************************************************/
U8 nRF24L01_X_spi_rw(U8 DevNum, U8 byte)
{
  U8 rxbyte;
    
  switch (DevNum)
  {
      case DEV_0:
          while(!(HS_MAC6200->SPIRSTAT & RF_TXFIFO_READY));      //Tx Fifo not ready.
          HS_MAC6200->SPIRDAT=byte;
          while(!(HS_MAC6200->SPIRSTAT & RF_RXDATA_READY));      //Rx Fifo has data.
          rxbyte = HS_MAC6200->SPIRDAT;  
          break;
      case DEV_1:
    	  break;
  } 
  
  return rxbyte;
}
/**************************************************/

/**************************************************
                                 nRF24L01 指令函数
**************************************************/
/**************************************************
函数：nRF24L01_X_read_reg(DevNum);

说明：
    从 nRF24L01 的寄存器 'reg' 读出一个字节。
**************************************************/
U8 nRF24L01_X_read_reg(U8 DevNum, U8 reg)
{
    U8 reg_val;

    nRF24L01_X_csn_low_disable_int(DevNum);                    // CSN low, init SPI transaction
    nRF24L01_X_spi_rw(DevNum, nRF24L01_R_REGISTER + reg);  // select register
    reg_val = nRF24L01_X_spi_rw(DevNum, 0);                // ..then read registervalue
    nRF24L01_X_csn_high_restore_int(DevNum);                   // CSN high again

    return (reg_val);                                          // return nRF24L01 status byte
}
/**************************************************/

#if 1
U8 nRF24L01_X_write_reg(U8 DevNum, U8 reg, U8 value)
{
  U8 status;
  bool_t ceb;
  
  if (ce_low_befor_write)
  {	
    ceb = nRF24L01_X_is_ce_high(DevNum);
    nRF24L01_X_ce_low(DevNum);
  }
  
  nRF24L01_X_csn_low_disable_int(DevNum);                               // CSN low, init SPI transaction
  status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_REGISTER + reg);    // select register
  nRF24L01_X_spi_rw(DevNum, value);                                 // ..and write value to it..
  nRF24L01_X_csn_high_restore_int(DevNum);                              // CSN high again
  
  if (ce_low_befor_write) 
  {
    ceb ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
  }
  return (status);                                                      // return nRF24L01 status byte
}
/**************************************************/

/**************************************************
函数：nRF24L01_X_read_reg_multibytes(DevNum);

说明：
    Writes value 'value' to register 'reg'
    Function other than nRF24L01's commands
**************************************************/
U8 nRF24L01_X_read_reg_multibytes(U8 DevNum, U8 Reg, U8 *pBuf, U8 bytes)
{
    U8 status, byte_ctr;

    nRF24L01_X_csn_low_disable_int(DevNum);                               // CSN low, init SPI transaction
    status = nRF24L01_X_spi_rw(DevNum, nRF24L01_R_REGISTER + Reg);    // Select register to write to and read status byte
    for(byte_ctr=0; byte_ctr<bytes; byte_ctr++)
    pBuf[byte_ctr] = nRF24L01_X_spi_rw(DevNum, 0);                    // Perform nRF24L01_X_spi_rw to read byte from nRF24L01
    nRF24L01_X_csn_high_restore_int(DevNum);                              // CSN high again

    return (status);                                                      // return nRF24L01 status byte
}
/**************************************************/

/**************************************************
函数：nRF24L01_X_write_reg_multibytes(DevNum);

说明：
    Writes value 'value' to register 'reg'
    Function other than nRF24L01's commands.
**************************************************/
//U8 nRF24L01_X_write_reg_multibytes(U8 DevNum, U8 Reg, U8 *pBuf, U8 bytes)
//{
//    U8 status, byte_ctr;
//    bool_t ceb;
//    
//    ceb = nRF24L01_X_is_ce_high(DevNum);
//    nRF24L01_X_ce_low(DevNum);
//    nRF24L01_X_csn_low_disable_int(DevNum);                               // CSN low, init SPI transaction
//        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_REGISTER + Reg);    // Select register to write to and read status byte
//        for(byte_ctr=0; byte_ctr<bytes; byte_ctr++)                       // then write all byte in buffer(*pBuf)
//        nRF24L01_X_spi_rw(DevNum, *pBuf++);
//    nRF24L01_X_csn_high_restore_int(DevNum);                              // CSN high again
//    ceb ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
//
//    return (status);                                                      // return nRF24L01 status byte
//}

U8 nRF24L01_X_write_reg_multibytes(U8 DevNum, U8 Reg, U8 *pBuf, U8 bytes)
{
    U8 status, byte_ctr;
    bool_t ceb;
    
	if (ce_low_befor_write)
	{	
    	ceb = nRF24L01_X_is_ce_high(DevNum);
    	nRF24L01_X_ce_low(DevNum);
    }

    nRF24L01_X_csn_low_disable_int(DevNum);                               // CSN low, init SPI transaction
    status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_REGISTER + Reg);    // Select register to write to and read status byte
    for(byte_ctr=0; byte_ctr<bytes; byte_ctr++)                       // then write all byte in buffer(*pBuf)
     nRF24L01_X_spi_rw(DevNum, *pBuf++);
    nRF24L01_X_csn_high_restore_int(DevNum);                              // CSN high again
    
	if (ce_low_befor_write) 
    {
		ceb ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
    }
    return (status);                                                      // return nRF24L01 status byte
}

/*
 * 在Analog Test 对TestPattern测试时，不能对CE操作.
 *
 */
U8 nRF24L01_X_write_reg_multibytes_no_ce(U8 DevNum, U8 Reg, U8 *pBuf, U8 bytes)
{
	U8 status, byte_ctr;
    nRF24L01_X_csn_low_disable_int(DevNum);                               // CSN low, init SPI transaction
    status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_REGISTER + Reg);    // Select register to write to and read status byte
    for(byte_ctr=0; byte_ctr<bytes; byte_ctr++)                       // then write all byte in buffer(*pBuf)
        nRF24L01_X_spi_rw(DevNum, *pBuf++);
    nRF24L01_X_csn_high_restore_int(DevNum);                              // CSN high again
    return (status);                                                      // return nRF24L01 status byte
}

/**************************************************/

/**************************************************
函数：nRF24L01_X_read_pipe_addr(DevNum);

说明：
    Writes value 'value' to register 'reg'
    Function other than nRF24L01's commands
**************************************************/
U8 nRF24L01_X_read_pipe_addr(U8 DevNum, U8 Px_Addr_Reg, U8 *pPipeAddr, U8 Addr_Width)
{
    U8 status, byte_ctr;

    nRF24L01_X_csn_low_disable_int(DevNum);                                       // CSN low, init SPI transaction
        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_R_REGISTER + Px_Addr_Reg);    // Select register to write to and read status byte
        for(byte_ctr=0; byte_ctr<Addr_Width; byte_ctr++)
        pPipeAddr[byte_ctr] = nRF24L01_X_spi_rw(DevNum, 0);                       // Perform nRF24L01_X_spi_rw to read byte from nRF24L01
    nRF24L01_X_csn_high_restore_int(DevNum);                                      // CSN high again

    return (status);                                                              // return nRF24L01 status byte
}
/**************************************************/

/**************************************************
函数：nRF24L01_X_write_pipe_addr(DevNum);

说明：
    Writes value 'value' to register 'reg'
    Function other than nRF24L01's commands.
**************************************************/
//U8 nRF24L01_X_write_pipe_addr(U8 DevNum, U8 Px_Addr_Reg, U8 *pPipeAddr, U8 Addr_Width)
//{
//    U8 status, byte_ctr;
//    bool_t ceb;
//    
//    ceb = nRF24L01_X_is_ce_high(DevNum);
//    nRF24L01_X_ce_low(DevNum);
//    
//    nRF24L01_X_csn_low_disable_int(DevNum);                                       // CSN low, init SPI transaction
//        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_REGISTER + Px_Addr_Reg);    // Select register to write to and read status byte
//        for(byte_ctr=0; byte_ctr<Addr_Width; byte_ctr++)                          // then write all byte in buffer(*pBuf)
//        nRF24L01_X_spi_rw(DevNum, *pPipeAddr++);
//    nRF24L01_X_csn_high_restore_int(DevNum);                                      // CSN high again
//
//    ceb ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
//
//    return (status);                                                              // return nRF24L01 status byte
//}

U8 nRF24L01_X_write_pipe_addr(U8 DevNum, U8 Px_Addr_Reg, U8 *pPipeAddr, U8 Addr_Width)
{
    U8 status, byte_ctr;
    bool_t ceb;
    
	if (ce_low_befor_write)
	{	
    	ceb = nRF24L01_X_is_ce_high(DevNum);
    	nRF24L01_X_ce_low(DevNum);
    }
    
    nRF24L01_X_csn_low_disable_int(DevNum);                                       // CSN low, init SPI transaction
        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_REGISTER + Px_Addr_Reg);    // Select register to write to and read status byte
        for(byte_ctr=0; byte_ctr<Addr_Width; byte_ctr++)                          // then write all byte in buffer(*pBuf)
        nRF24L01_X_spi_rw(DevNum, *pPipeAddr++);
    nRF24L01_X_csn_high_restore_int(DevNum);                                      // CSN high again
    
	if (ce_low_befor_write) 
    {
		ceb ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
    }
    return (status);                                                              // return nRF24L01 status byte
}

/**************************************************/

/**************************************************
函数：nRF24L01_X_read_rx_payload(DevNum);

说明：
    Reads 'bytes' #of bytes from register 'reg'
    Typically used to read RX payload, Rx/Tx address
**************************************************/
U8 nRF24L01_X_read_rx_payload(U8 DevNum, U8 *pBuf, U8 bytes)
{
    U8 status, byte_ctr;

    nRF24L01_X_csn_low_disable_int(DevNum);                           // CSN low, init SPI transaction
        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_R_RX_PAYLOAD);    // Select register to write to and read status byte
        for(byte_ctr=0;byte_ctr<bytes;byte_ctr++)
        pBuf[byte_ctr] = nRF24L01_X_spi_rw(DevNum, 0);                // Perform nRF24L01_X_spi_rw to read byte from nRF24L01
    nRF24L01_X_csn_high_restore_int(DevNum);                          // CSN high again

    return (status);                                                  // return nRF24L01 status byte
}
/**************************************************/

/**************************************************
函数：nRF24L01_X_write_tx_payload(DevNum);

说明：
    Writes contents of buffer '*pBuf' to nRF24L01
    Typically used to write TX payload, Rx/Tx address
**************************************************/
//U8 nRF24L01_X_write_tx_payload(U8 DevNum, U8 *pBuf, U8 bytes)
//{
//    U8 status, byte_ctr;
//    bool_t ceb;
//    
//    ceb = nRF24L01_X_is_ce_high(DevNum);
//    nRF24L01_X_ce_low(DevNum);
//    
//    nRF24L01_X_csn_low_disable_int(DevNum);                           // CSN low, init SPI transaction
//        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_TX_PAYLOAD);    // Select register to write to and read status byte
//        for(byte_ctr=0; byte_ctr<bytes; byte_ctr++)                   // then write all byte in buffer(*pBuf)
//            nRF24L01_X_spi_rw(DevNum, *pBuf++);
//    nRF24L01_X_csn_high_restore_int(DevNum);                          // CSN high again
//
//    ceb ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
//
//    return (status);                                                  // return nRF24L01 status byte
//}

U8 nRF24L01_X_write_tx_payload(U8 DevNum, U8 *pBuf, U8 bytes)
{
    U8 status, byte_ctr;
    bool_t ceb;
    
	if (ce_low_befor_write)
	{	
    	ceb = nRF24L01_X_is_ce_high(DevNum);
    	nRF24L01_X_ce_low(DevNum);
    }
    
    nRF24L01_X_csn_low_disable_int(DevNum);                           // CSN low, init SPI transaction
        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_TX_PAYLOAD);    // Select register to write to and read status byte
        for(byte_ctr=0; byte_ctr<bytes; byte_ctr++)                   // then write all byte in buffer(*pBuf)
            nRF24L01_X_spi_rw(DevNum, *pBuf++);
    nRF24L01_X_csn_high_restore_int(DevNum);                          // CSN high again
    
	if (ce_low_befor_write) 
    {
		ceb ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
    }
    return (status);                                                  // return nRF24L01 status byte
}

/**************************************************/

/**************************************************
函数：nRF24L01_X_flush_rx(DevNum);

说明：
    Writes value 'value' to register 'reg'
**************************************************/
U8 nRF24L01_X_flush_rx(U8 DevNum)
{
    U8 status;	
    nRF24L01_X_csn_low_disable_int(DevNum);                       // CSN low, init SPI transaction
    status = nRF24L01_X_spi_rw(DevNum, nRF24L01_FLUSH_RX);    // send command
    nRF24L01_X_csn_high_restore_int(DevNum);                      // CSN high again
    return (status);                                              // return nRF24L01 status byte
}
/**************************************************/

/**************************************************
函数：nRF24L01_X_flush_tx(DevNum);

说明：
    Writes value 'value' to register 'reg'
**************************************************/
U8 nRF24L01_X_flush_tx(U8 DevNum)
{
    U8 status;
    nRF24L01_X_csn_low_disable_int(DevNum);                       // CSN low, init SPI transaction
    status = nRF24L01_X_spi_rw(DevNum, nRF24L01_FLUSH_TX);    // send command
    nRF24L01_X_csn_high_restore_int(DevNum);                      // CSN high again
    return (status);                                              // return nRF24L01 status byte
}
/**************************************************/

/**************************************************
函数：nRF24L01_X_reuse_tx_payload(DevNum);

说明：
    manually to retransmit
**************************************************/
U8 nRF24L01_X_reuse_tx_payload(U8 DevNum)
{
    U8 status;

    nRF24L01_X_csn_low_disable_int(DevNum);                          // CSN low, init SPI transaction
        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_REUSE_TX_PL);    // send command
    nRF24L01_X_csn_high_restore_int(DevNum);                         // CSN high again

    return (status);                                                 // return nRF24L01 status byte
}
/**************************************************/

/**************************************************
函数：nRF24L01_X_activate(DevNum);

说明：
    Writes value 'value' to register 'reg'
**************************************************/
U8 nRF24L01_X_activate(U8 DevNum)
{
    U8 status;

    nRF24L01_X_csn_low_disable_int(DevNum);                       // CSN low, init SPI transaction
        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_ACTIVATE);    // send command
        nRF24L01_X_spi_rw(DevNum, 0x73);
    nRF24L01_X_csn_high_restore_int(DevNum);                      // CSN high again

    return (status);                                              // return nRF24L01 status byte
}
/**************************************************/

/**************************************************
函数：nRF24L01_X_read_rx_payload_width(DevNum);

说明：
    Writes value 'value' to register 'reg'
**************************************************/
U8 nRF24L01_X_read_rx_payload_width(U8 DevNum)
{
    U8 plw;

    nRF24L01_X_csn_low_disable_int(DevNum);                 // CSN low, init SPI transaction
        nRF24L01_X_spi_rw(DevNum, nRF24L01_R_RX_PL_WID);    // send command
        plw = nRF24L01_X_spi_rw(DevNum, 0);
    nRF24L01_X_csn_high_restore_int(DevNum);                // CSN high again

    return (plw);                                           // return value
}
/**************************************************/

/**************************************************
函数：nRF24L01_X_write_ack_payload(DevNum);

说明：
    Writes ACK payload 
**************************************************/
//U8 nRF24L01_X_write_ack_payload(U8 DevNum, U8 PipeNum, U8 *pBuf, U8 bytes)  //write ACK payload
//{
//    U8 status, byte_ctr;
//    bool_t ceb;
//    
//    ceb = nRF24L01_X_is_ce_high(DevNum);
//    nRF24L01_X_ce_low(DevNum);
//    
//    nRF24L01_X_csn_low_disable_int(DevNum);                                     // CSN low, init SPI transaction
//        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_ACK_PAYLOAD | PipeNum);   // Select register to write to and read status byte
//        for(byte_ctr=0; byte_ctr<bytes; byte_ctr++) {                           // then write all byte in buffer(*pBuf)
//            nRF24L01_X_spi_rw(DevNum, *pBuf++);
//        }
//    nRF24L01_X_csn_high_restore_int(DevNum);                                    // CSN high again
//
//    ceb ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
//
//    return (status);                                                            // return nRF24L01 status byte
//}

U8 nRF24L01_X_write_ack_payload(U8 DevNum, U8 PipeNum, U8 *pBuf, U8 bytes)  //write ACK payload
{
    U8 status, byte_ctr;
    bool_t ceb;
    
	if (ce_low_befor_write)
	{	
    	ceb = nRF24L01_X_is_ce_high(DevNum);
    	nRF24L01_X_ce_low(DevNum);
    }
  
    nRF24L01_X_csn_low_disable_int(DevNum);                                     // CSN low, init SPI transaction
        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_ACK_PAYLOAD | PipeNum);   // Select register to write to and read status byte
        for(byte_ctr=0; byte_ctr<bytes; byte_ctr++) {                           // then write all byte in buffer(*pBuf)
            nRF24L01_X_spi_rw(DevNum, *pBuf++);
        }
    nRF24L01_X_csn_high_restore_int(DevNum);                                    // CSN high again
    
	if (ce_low_befor_write)
    {        
		ceb ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
    }
    return (status);                                                            // return nRF24L01 status byte
}

/**************************************************/

/**************************************************
函数：nRF24L01_X_write_tx_payload_noack(DevNum);

说明：
    Writes value 'value' to register 'reg'
**************************************************/
//U8 nRF24L01_X_write_tx_payload_noack(U8 DevNum, U8 *pBuf, U8 bytes)
//{
//    U8 status, byte_ctr;
//    bool_t ceb;
//    
//    ceb = nRF24L01_X_is_ce_high(DevNum);
//    nRF24L01_X_ce_low(DevNum);
//    
//    nRF24L01_X_csn_low_disable_int(DevNum);                                 // CSN low, init SPI transaction
//        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_TX_PAYLOAD_NOACK);    // Select register to write to and read status byte
//        for(byte_ctr=0; byte_ctr<bytes; byte_ctr++)                         // then write all byte in buffer(*pBuf)
//        nRF24L01_X_spi_rw(DevNum, *pBuf++);
//    nRF24L01_X_csn_high_restore_int(DevNum);                                // CSN high again
//
//    ceb ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
//
//    return (status); 
//}

U8 nRF24L01_X_write_tx_payload_noack(U8 DevNum, U8 *pBuf, U8 bytes)
{
    U8 status, byte_ctr;
    bool_t ceb;
    
	if (ce_low_befor_write)
	{	
    	ceb = nRF24L01_X_is_ce_high(DevNum);
    	nRF24L01_X_ce_low(DevNum);
    }
    
    nRF24L01_X_csn_low_disable_int(DevNum);                                 // CSN low, init SPI transaction
        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_W_TX_PAYLOAD_NOACK);    // Select register to write to and read status byte
        for(byte_ctr=0; byte_ctr<bytes; byte_ctr++)                         // then write all byte in buffer(*pBuf)
        nRF24L01_X_spi_rw(DevNum, *pBuf++);
    nRF24L01_X_csn_high_restore_int(DevNum);                                // CSN high again

	if (ce_low_befor_write)
    {        
		ceb ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
    }
    return (status); 
}

/**************************************************/

/**************************************************
函数：nRF24L01_X_nop(DevNum);

说明：
    Writes value 'value' to register 'reg'
**************************************************/
U8 nRF24L01_X_nop(U8 DevNum)
{
    U8 status;

    nRF24L01_X_csn_low_disable_int(DevNum);
    status = nRF24L01_X_spi_rw(DevNum, nRF24L01_NOP);
    nRF24L01_X_csn_high_restore_int(DevNum);

    return (status); 
}
/**************************************************/

/*-------------------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------------------------------------*/

/**************************************************
                                 nRF24L01 应用函数
**************************************************
**************************************************
函数：nRF24L01_X_rx_fifo_block_flush(DevNum);

说明：
检查 RX_FIFO 是否满，是否中断，若满而未中断，
则堵塞，清空之。
**************************************************/
void nRF24L01_X_rx_fifo_block_flush(U8 DevNum)
{
    if (nRF24L01_X_read_reg(DevNum, nRF24L01_FIFO_STATUS) & 0x20) // RX_FIFO 满否？
    if (!(nRF24L01_X_nop(DevNum) & 0x40))                        // RX_DR 中断否？
    {
        if (nRF24L01_X_is_ce_high(DevNum))
        {
            nRF24L01_X_ce_low(DevNum);
            nRF24L01_X_flush_rx(DevNum);                                     // 若满而未中断，则 RX_FIFO 堵塞，清空之
            nRF24L01_X_ce_high(DevNum);
        }
        else nRF24L01_X_flush_rx(DevNum);                                       // 若满而未中断，则 RX_FIFO 堵塞，清空之
    }
}
/**************************************************/

/*--------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------*/
/**************************************************
                nRF24L01 模式函数
**************************************************/
void nRF24L01_X_set_mask_rx_dr(U8 DevNum, U8 mask_rx_dr)
{
    if(mask_rx_dr)
        nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_MASK_RX_DR) | nRF24L01_MASK_RX_DR);
    else
        nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_MASK_RX_DR));
}

void nRF24L01_X_set_mask_tx_ds(U8 DevNum, U8 mask_tx_ds)
{
    if(mask_tx_ds)
        nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_MASK_TX_DS) | nRF24L01_MASK_TX_DS);
    else
        nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_MASK_TX_DS));
}

void nRF24L01_X_set_mask_max_rt(U8 DevNum, U8 mask_max_rt)
{
    if(mask_max_rt)
        nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_MASK_MAX_RT) | nRF24L01_MASK_MAX_RT);
    else
        nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_MASK_MAX_RT));
}

//1=1byte CRC;2=2Byte CRC,0=disable CRC,
void nRF24L01_X_set_crc(U8 DevNum, U8 crc)
{
    if (crc)
    {
        nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) | nRF24L01_EN_CRC);
        if (crc == 1)
            nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_CRCO) | nRF24L01_CRCO_1_BYTES);
        else if (crc == 2)
            nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_CRCO) | nRF24L01_CRCO_2_BYTES);
    }
    else nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_EN_CRC));
}

U8 nRF24L01_X_get_crc(U8 DevNum)
{
	U8 Temp;
    if (nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & nRF24L01_EN_CRC)
    {
        if (nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & nRF24L01_CRCO)
            Temp=0x02;
        else
            Temp=0x01;
    }
    else Temp=0x00;
	return Temp;
}

void nRF24L01_X_set_powerup(U8 DevNum)
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_PWR_UP) | nRF24L01_PWR_UP);
}

void nRF24L01_X_set_powerdown(U8 DevNum)
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_PWR_UP) | nRF24L01_PWR_DWN);
}

void nRF24L01_X_set_prx(U8 DevNum)
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_PRIM_RX) | nRF24L01_PRIM_RX);
}

void nRF24L01_X_set_ptx(U8 DevNum)
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & (~nRF24L01_PRIM_RX));
}

bool_t nRF24L01_X_is_prx(U8 DevNum)
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & nRF24L01_PRIM_RX);
}

bool_t nRF24L01_X_is_ptx(U8 DevNum)

{
    return !(nRF24L01_X_read_reg(DevNum, nRF24L01_CONFIG) & nRF24L01_PRIM_RX);
}

void nRF24L01_X_set_en_auto_ack_all(U8 DevNum, U8 en)
{
    if(en)
        nRF24L01_X_write_reg(DevNum, nRF24L01_EN_AA, 0x3f);
    else
        nRF24L01_X_write_reg(DevNum, nRF24L01_EN_AA, 0x00);
}

void nRF24L01_X_set_en_auto_ack_pipe(U8 DevNum, U8 pipe, U8 en)
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_EN_AA, nRF24L01_X_read_reg(DevNum, nRF24L01_EN_AA) & (~(1<<pipe)) | (en<<pipe));
}


U8 nRF24L01_X_is_auto_ack_px(U8 DevNum, U8 pipe)
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_EN_AA) & (1<<pipe));
}

void nRF24L01_X_set_en_rx_pipe_all(U8 DevNum, U8 en)
{
    if(en)
        nRF24L01_X_write_reg(DevNum, nRF24L01_EN_RXADDR, 0x3f);
    else
        nRF24L01_X_write_reg(DevNum, nRF24L01_EN_RXADDR, 0x00);
}

void nRF24L01_X_set_en_rx_pipe(U8 DevNum, U8 pipe, U8 en)
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_EN_RXADDR, nRF24L01_X_read_reg(DevNum, nRF24L01_EN_RXADDR) & (~(1<<pipe)) | (en<<pipe));
}

void nRF24L01_X_set_rx_pipe_addr(U8 DevNum, U8 pipe, U8 *pPipeAddr, U8 addr_width)
{
    nRF24L01_X_write_reg_multibytes(DevNum, nRF24L01_RX_ADDR_P0 + pipe, pPipeAddr, addr_width);
}

void nRF24L01_X_set_rx_pipe_static_pay_load_width(U8 DevNum, U8 pipe, U8 static_plw)
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_RX_PW_P0 + pipe, static_plw); // 通道静态载荷长度
}
U8 nRF24L01_X_read_rx_pipe_static_pay_load_width(U8 DevNum,U8 pipe)
{
    U8 static_plw;
	static_plw=nRF24L01_X_read_reg(DevNum, nRF24L01_RX_PW_P0 + pipe);
	return 	static_plw;
}

void nRF24L01_X_set_address_width(U8 DevNum, U8 AW)
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_SETUP_AW, ((AW == 3) ? nRF24L01_AW_3_BYTES : ((AW == 4) ? nRF24L01_AW_4_BYTES : nRF24L01_AW_5_BYTES)));
}

U8 nRF24L01_X_get_aw(U8 DevNum)
{
	U8 Temp;
    switch (nRF24L01_X_read_reg(DevNum, nRF24L01_SETUP_AW) & nRF24L01_AW_BITS)
    {
    case nRF24L01_AW_3_BYTES:
        Temp=0x03;
	break;
    case nRF24L01_AW_4_BYTES:
        Temp=0x04;
	break;
    case nRF24L01_AW_5_BYTES:
        Temp=0x05;
	break;
    default:
        Temp=0x00;
	break;
    }
	return Temp;
}

void nRF24L01_X_set_auto_retr_delay(U8 DevNum, U8 ARD)
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_SETUP_RETR, (nRF24L01_X_read_reg(DevNum, nRF24L01_SETUP_RETR) & (~nRF24L01_ARD_BITS) | ((ARD & 0x0f)<<4)));
}

U8 nRF24L01_X_get_ard(U8 DevNum)       //auto retransmit delay
{
    return ((nRF24L01_X_read_reg(DevNum, nRF24L01_SETUP_RETR) & nRF24L01_ARD_BITS)>>4);
}

void nRF24L01_X_set_auto_retr_count(U8 DevNum, U8 ARC)  //auto retransmit count
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_SETUP_RETR, (nRF24L01_X_read_reg(DevNum, nRF24L01_SETUP_RETR) & (~nRF24L01_ARC_BITS) | (ARC & 0x0f)));
}

U8 nRF24L01_X_get_arc(U8 DevNum)    //get auto retransmit count
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_SETUP_RETR) & nRF24L01_ARC_BITS);
}

void nRF24L01_X_set_auto_retr_disable(U8 DevNum)  //disable ARC(auto retransmit function)
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_SETUP_RETR, (nRF24L01_X_read_reg(DevNum, nRF24L01_SETUP_RETR) & (~nRF24L01_ARC_BITS) | 0));
}

void nRF24L01_X_set_rf_ch(U8 DevNum, U8 RF_CH)  //set RF channel
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_RF_CH, RF_CH);
    if((Dev_Flag[DevNum]==MAC6200_DEV))
    {
       MAC6200_set_freq(2400+RF_CH); 
       chThdSleepMilliseconds(5);
       MAC6200_set_freq(2400+RF_CH); 
    }
}

U8 nRF24L01_X_get_rf_ch(U8 DevNum)    //get RF channel
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_CH) & nRF24L01_RF_CH_BITS);
}

void nRF24L01_X_set_cont_wave(U8 DevNum, U8 CONT_WAVE) //contious carrier wave transmit
{
    if(CONT_WAVE)
        nRF24L01_X_write_reg(DevNum, nRF24L01_RF_SETUP, (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_SETUP) & (~nRF24L01_CONT_WAVE) | nRF24L01_CONT_WAVE));
    else
        nRF24L01_X_write_reg(DevNum, nRF24L01_RF_SETUP, (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_SETUP) & (~nRF24L01_CONT_WAVE)));
}

U8 nRF24L01_X_is_cont_wave(U8 DevNum)  
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_SETUP) & nRF24L01_CONT_WAVE);
}

void nRF24L01_X_set_pll_lock(U8 DevNum, U8 PLL_LOCK)   //force PLL lock  used in test
{
    if(PLL_LOCK)
        nRF24L01_X_write_reg(DevNum, nRF24L01_RF_SETUP, (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_SETUP) & (~nRF24L01_PLL_LOCK) | nRF24L01_PLL_LOCK));
    else
        nRF24L01_X_write_reg(DevNum, nRF24L01_RF_SETUP, (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_SETUP) & (~nRF24L01_PLL_LOCK)));
}

U8 nRF24L01_X_is_pll_lock(U8 DevNum)   
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_SETUP) & nRF24L01_PLL_LOCK);
}

void nRF24L01_X_set_rf_datarate(U8 DevNum, U8 RF_DR)  //set RF datarate
{
    switch (RF_DR)
    {
    case 0:
        RF_DR = nRF24L01_RF_DR_250_Kbps;
        break;
    case 1:
        RF_DR = nRF24L01_RF_DR_1_Mbps;
        break;
    case 2:
        RF_DR = nRF24L01_RF_DR_2_Mbps;
        break;
    default:
        break;
    }
    nRF24L01_X_write_reg(DevNum, nRF24L01_RF_SETUP, (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_SETUP) & (~(nRF24L01_RF_DR_LOW | nRF24L01_RF_DR_HIGH)) | RF_DR));
}

U8 nRF24L01_X_get_rf_datarate(U8 DevNum)
{
    U8 Temp;
    switch (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_SETUP) & (nRF24L01_RF_DR_LOW | nRF24L01_RF_DR_HIGH))
    {
    case nRF24L01_RF_DR_250_Kbps:
        Temp=0x00;
    break;
    case nRF24L01_RF_DR_1_Mbps:
        Temp=0x01;
    break;
    case nRF24L01_RF_DR_2_Mbps:
        Temp=0x02;
    break;
    default:
        Temp=0xff;
    break;
    }
	return Temp;
}

void nRF24L01_X_set_rf_output_power(U8 DevNum, U8 RF_PWR)  //set RF out power in TE mode
{
    switch (RF_PWR) {
    case 0x00:
        RF_PWR = nRF24L01_RF_PWR_n18_dBm;
        break;
    case 0x01:
        RF_PWR = nRF24L01_RF_PWR_n12_dBm;
        break;
    case 0x02:
        RF_PWR = nRF24L01_RF_PWR_n6_dBm;
        break;
    case 0x03:
        RF_PWR = nRF24L01_RF_PWR_0_dBm;
        break;
    default:
        break;
    }
    
    nRF24L01_X_write_reg(DevNum, nRF24L01_RF_SETUP, (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_SETUP) & (~nRF24L01_RF_PWR_0_dBm) | RF_PWR));
}

void nRF24L01_X_set_lna_gain(U8 DevNum, U8 LNA_HCURR) //set LNA(Low Noise Amplifer ) gain
{
    if(LNA_HCURR)
        nRF24L01_X_write_reg(DevNum, nRF24L01_RF_SETUP, (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_SETUP) & (~nRF24L01_LNA_HCURR) | nRF24L01_LNA_HCURR));
    else
        nRF24L01_X_write_reg(DevNum, nRF24L01_RF_SETUP, (nRF24L01_X_read_reg(DevNum, nRF24L01_RF_SETUP) & (~nRF24L01_LNA_HCURR)));
}

U8 nRF24L01_X_read_rx_p_no(U8 DevNum) //RX FIFO status (which pipe is avilable)
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_STATUS) & nRF24L01_RX_FIFO_EMPTY)>>1;
}

U8 nRF24L01_X_is_rx_empty(U8 DevNum) //RX FIFO is empty? 
{
    return ((nRF24L01_X_read_reg(DevNum, nRF24L01_STATUS) & nRF24L01_RX_FIFO_EMPTY) == nRF24L01_RX_FIFO_EMPTY);
}

U8 nRF24L01_X_is_tx_full(U8 DevNum)  //TX FIFO is full?
{
    return ((nRF24L01_X_read_reg(DevNum, nRF24L01_STATUS) & nRF24L01_TX_FIFO_FULL) == nRF24L01_TX_FIFO_FULL);
}

U8 nRF24L01_X_is_rx_fifo_full(U8 DevNum) //RX FIFO is full
{
    return ((nRF24L01_X_read_reg(DevNum, nRF24L01_FIFO_STATUS) & nRF24L01_FIFO_STATUS_RX_FULL) == nRF24L01_FIFO_STATUS_RX_FULL);
}

U8 nRF24L01_X_is_rx_fifo_empty(U8 DevNum) //RX FIFO is empty?
{
    return ((nRF24L01_X_read_reg(DevNum, nRF24L01_FIFO_STATUS) & nRF24L01_FIFO_STATUS_RX_EMPTY) == nRF24L01_FIFO_STATUS_RX_EMPTY);
}

U8 nRF24L01_X_is_tx_reuse(U8 DevNum)  //retransmit TX
{
    return ((nRF24L01_X_read_reg(DevNum, nRF24L01_FIFO_STATUS) & nRF24L01_FIFO_STATUS_TX_REUSE) == nRF24L01_FIFO_STATUS_TX_REUSE);
}

U8 nRF24L01_X_is_tx_fifo_full(U8 DevNum) //TX FIFO is FIFO
{
    return ((nRF24L01_X_read_reg(DevNum, nRF24L01_FIFO_STATUS) & nRF24L01_FIFO_STATUS_TX_FULL) == nRF24L01_FIFO_STATUS_TX_FULL);
}

U8 nRF24L01_X_is_tx_fifo_empty(U8 DevNum) //TX FIFO empty
{
    return ((nRF24L01_X_read_reg(DevNum, nRF24L01_FIFO_STATUS) & nRF24L01_FIFO_STATUS_TX_EMPTY) == nRF24L01_FIFO_STATUS_TX_EMPTY);
}

U8 nRF24L01_X_get_plos_cnt(U8 DevNum)  //get PLOS(Packet lost) count
{
    return ((nRF24L01_X_read_reg(DevNum, nRF24L01_OBSERVE_TX) & nRF24L01_PLOS_CNT)>>4);
}

U8 nRF24L01_X_get_arc_cnt(U8 DevNum)  //get counters retransmit packets
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_OBSERVE_TX) & nRF24L01_ARC_CNT);
}

U8 nRF24L01_X_is_rpd(U8 DevNum)  //RPD is Receive power detect?
{
    return ((nRF24L01_X_read_reg(DevNum, nRF24L01_RPD) & nRF24L01_RPD_BITS) == nRF24L01_RPD_BITS);
}

void nRF24L01_X_set_en_dpl_all(U8 DevNum, U8 en)  //enable all pipes to dynamic payload
{
    if(en)
        nRF24L01_X_write_reg(DevNum, nRF24L01_DYNPD, 0x3f);
    else
        nRF24L01_X_write_reg(DevNum, nRF24L01_DYNPD, 0x00);
}

void nRF24L01_X_set_en_dpl_px(U8 DevNum, U8 pipe, U8 en)  //enable pipes to dynamic payload
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_DYNPD, (nRF24L01_X_read_reg(DevNum, nRF24L01_DYNPD) & (~(1<<pipe))) | (en<<pipe));
}

void nRF24L01_X_set_en_dpl_feature(U8 DevNum, U8 en)//set EN_DPL
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_FEATURE, (nRF24L01_X_read_reg(DevNum, nRF24L01_FEATURE) & (~nRF24L01_EN_DPL) | (en ? nRF24L01_EN_DPL : 0)));
}

U8 nRF24L01_X_is_dpl_px(U8 DevNum, U8 pipe) //pipe is dynamic?
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_DYNPD) & (1<<pipe));
}

U8 nRF24L01_X_is_dpl_feature(U8 DevNum)  //is enable dynamic (EN_DPL)?
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_FEATURE) & nRF24L01_EN_DPL);
}

void nRF24L01_X_set_en_ack_pay_feature(U8 DevNum, U8 en)  //enable ack payload
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_FEATURE, (nRF24L01_X_read_reg(DevNum, nRF24L01_FEATURE) & (~nRF24L01_EN_ACK_PAY) | (en ? nRF24L01_EN_ACK_PAY : 0)));
}

U8 nRF24L01_X_is_ack_pay_feature(U8 DevNum)
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_FEATURE) & nRF24L01_EN_ACK_PAY);
}

void nRF24L01_X_set_en_dyn_ack_feature(U8 DevNum, U8 en)
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_FEATURE, (nRF24L01_X_read_reg(DevNum, nRF24L01_FEATURE) & (~nRF24L01_EN_DYN_ACK) | (en ? nRF24L01_EN_DYN_ACK : 0)));
}

U8 nRF24L01_X_is_dyn_ack_feature(U8 DevNum)
{
    return (nRF24L01_X_read_reg(DevNum, nRF24L01_FEATURE) & nRF24L01_EN_DYN_ACK);
}

void nRF24L01_X_set_standby_I(U8 DevNum)
{
    nRF24L01_X_set_powerup(DevNum);
    nRF24L01_X_ce_low(DevNum);
}

void nRF24L01_X_set_standby_II(U8 DevNum)
{
    nRF24L01_X_set_powerup(DevNum);
    nRF24L01_X_flush_tx(DevNum);
    nRF24L01_X_set_ptx(DevNum);
    nRF24L01_X_ce_high(DevNum);
}


void nRF24L01_X_clear_int_flags(U8 DevNum)  //clear int flags in status register
{
    nRF24L01_X_write_reg(DevNum, nRF24L01_STATUS, 0x70); 
}

void nRF24L01_X_clear_plos_cnt(U8 DevNum)
{
    nRF24L01_X_set_rf_ch(DevNum, nRF24L01_X_get_rf_ch(DevNum)); 
}

void nRF24L01_X_set_pipe(U8 DevNum, U8 PipeNum, U8 en_pipe, U8 en_auto_ack, U8 *pPipeAddr, U8 addr_width, U8 static_plw, U8 en_dpl)
{
    nRF24L01_X_set_en_rx_pipe(DevNum, PipeNum, en_pipe);                        // 通道使能
    nRF24L01_X_set_en_auto_ack_pipe(DevNum, PipeNum, en_auto_ack);              // 通道自动应答使能
    nRF24L01_X_set_rx_pipe_addr(DevNum, PipeNum, pPipeAddr, addr_width);        // 通道地址
    nRF24L01_X_set_rx_pipe_static_pay_load_width(DevNum, PipeNum, static_plw);  // 通道静态载长
    nRF24L01_X_set_en_dpl_px(DevNum, PipeNum, en_dpl);                          // 通道动态载长使能
}

/**************************************************
函数：nRF24L01_X_send_packet(DevNum);

说明：
使用 nRF24L01 发送一个包。
**************************************************/
void nRF24L01_X_send_packet(uchar DevNum, uchar *pBuf, uchar plw, uchar en_ack)
{
    nRF24L01_X_ce_low(DevNum); //转入 Standby-I 模式
    
    if (nRF24L01_X_is_tx_fifo_full(DevNum)) return; // 若 TX_FIFO 满则不发送，直接返回。

    if (!(nRF24L01_X_read_reg(DevNum, nRF24L01_EN_AA) & nRF24L01_EN_AA_P0)) // SB：静无答
    {
        nRF24L01_X_write_tx_payload(DevNum, pBuf, plw);
    }
    else // ESB
    {
        if (en_ack)
        {
            nRF24L01_X_write_tx_payload(DevNum, pBuf, plw);
        }
        else
        {
            nRF24L01_X_write_tx_payload_noack(DevNum, pBuf, plw); // 向 TX FIFO 写入数据
        }
    }

    if (!nRF24L01_X_Busy[DevNum])
    {
        nRF24L01_X_Busy[DevNum] = 1; 
        nRF24L01_X_ce_high_pulse(DevNum); 
    }
}

void nRF24L01_X_send_packet_by_mode(uchar DevNum, uchar PipeNum, uchar *pBuf, uchar plw)
{
	rf_config_t Config;
    Config=nRF24L01_X_CurrentConfig[DevNum];
    if (Config.bf.PTXPRX) // PRX
    {
        if (Config.bf.ACKPAY)
        {
            if (!nRF24L01_X_is_tx_fifo_full(DevNum))
            {
                nRF24L01_X_write_ack_payload(DevNum, PipeNum, pBuf, plw); // 向 TX FIFO 写入通道 PipeNum 应答数据，等待应答
            }
        }
    }
    else // PTX
    {
        // change tx_addr to PipeNum addr
//        nRF24L01_X_write_pipe_addr(g_Input_Focus, nRF24L01_TX_ADDR,    nRF24L01_X_RX_ADDRESS_P[g_Input_Focus][PipeNum], nRF24L01_X_get_aw(g_Input_Focus));
//        nRF24L01_X_write_pipe_addr(g_Input_Focus, nRF24L01_RX_ADDR_P0, nRF24L01_X_RX_ADDRESS_P[g_Input_Focus][PipeNum], nRF24L01_X_get_aw(g_Input_Focus));

//        EA = 0;
        // 是否手重传，是否动静长，是否动无答
        if (Config.bf.MR == 1) // 手重传
        {
            nRF24L01_X_Busy[DevNum] = 1;
            nRF24L01_X_ce_high_pulse(DevNum);
            // while (nRF24L01_X_Busy[DevNum]);

            // nRF24L01_X_ce_high_pulse(DevNum, nRF24L01_Thce);
            //          nRF24L01_1_CE = 1;
            //          nRF24L01_X_Busy = 1; // 暂不置忙，以免有答而未收到应答时死掉，待弄清手动重传的机理后再定。
        }
        else
        {
            nRF24L01_X_send_packet(DevNum, pBuf, plw, Config.bf.EN_ACK);
        }
//       EA = 1;
    }
}


/**************************************************/


void nRF24L01_X_common_setting(U8 DevNum) // CRC, ARC, ARD, RF_CH, etc.
{
    nRF24L01_X_init_port(DevNum);
    
    //--------------------------------------------------/
    //                  共同设置
    //--------------------------------------------------/
    // 共同设置：CRC 使能、CRC 编码方案、接收地址宽度、频道、空中数据速率、LNA 增益、RF 输出功率；
    // 数据接收中断、数据发送中断、最大重发中断；上电、收发控制；自动重发延时、自动重发次数。
    // {通道使能（EN_RXADDR），自动应答使能（EN_AA）}
    // CONFIG, EN_AA, EN_RXADDR, SETUP_AW, SETUP_RETR, RF_CH, RF_SETUP
    nRF24L01_X_set_mask_rx_dr(DevNum, 0);                             // 启用接收中断（RX_DR）
    nRF24L01_X_set_mask_tx_ds(DevNum, 0);                             // 启用发送中断（TX_DS）
    nRF24L01_X_set_mask_max_rt(DevNum, 0);                            // 启用最大重传中断（MAX_RT）
    nRF24L01_X_set_crc(DevNum, 2);                                    // 使用 2 字节 CRC 编码机制
    nRF24L01_X_set_powerup(DevNum);                                   // 上电
    nRF24L01_X_set_prx(DevNum);                                       // 将设备置于 PRX 模式
    
    nRF24L01_X_set_en_auto_ack_all(DevNum, 0);                        // 禁能所有接收通道自动应答，后面接收通道设置中单独使能。
    nRF24L01_X_set_en_rx_pipe_all(DevNum, 0);                         // 禁能所有接收通道，后面接收通道设置中单独使能。
    nRF24L01_X_set_address_width(DevNum, nRF24L01_X_ADR_WIDTH);       // 设置通道地址宽度:0x03:5 字节，此处必须注意，不能填 5。
                                                                        
    nRF24L01_X_set_auto_retr_delay(DevNum, nRF24L01_ARD_500_US);      // 设置重发延时：500us + 130us, refer to PDF page 46.
    nRF24L01_X_set_auto_retr_count(DevNum, nRF24L01_ARC_10);          // 重发次数：10
    
    nRF24L01_X_set_rf_ch(DevNum, 105);                                 // 选择 RF 信道号：105
    
    nRF24L01_X_set_cont_wave(DevNum, 0);                              // 禁用恒定载波输出
    nRF24L01_X_set_pll_lock(DevNum, 0);                               // 禁用强制 PLL 锁定信号
    nRF24L01_X_set_rf_datarate(DevNum, 1);                            // 数据速率：1Mbps
    nRF24L01_X_set_rf_output_power(DevNum, 0);                        // RF 功率：0dBm
    nRF24L01_X_set_lna_gain(DevNum, nRF24L01_LNA_HCURR);              // LNA 增益：HCURR
        
    // nRF24L01 未提供软复位功能，只能逐个将寄存器设置为复位状态。
    nRF24L01_X_clear_int_flags(DevNum);                               // 清零状态寄存器，尤其是 MAX_RT 位，If MAX_RT    is asserted it must be cleared to enable further communication.
    nRF24L01_X_clear_plos_cnt(DevNum);
    nRF24L01_X_set_en_dpl_all(DevNum, 0);
    nRF24L01_X_set_en_dpl_feature(DevNum, 0);
    nRF24L01_X_set_en_dyn_ack_feature(DevNum, 0);
    nRF24L01_X_set_en_ack_pay_feature(DevNum, 0);
    nRF24L01_X_flush_rx(DevNum);                                      // 清空 RX_FIFO
    nRF24L01_X_flush_tx(DevNum);                                      // 清空 TX_FIFO
    // 将各通道地址设置至上电复位地址
    
/*     
    //--------------------------------------------------/
    //                  共同设置
    //--------------------------------------------------/
    // 共同设置：CRC 使能、CRC 编码方案、接收地址宽度、频道、空中数据速率、LNA 增益、RF 输出功率；
    // 数据接收中断、数据发送中断、最大重发中断；上电、收发控制；自动重发延时、自动重发次数。
    // {通道使能（EN_RXADDR），自动应答使能（EN_AA）}
    // CONFIG, EN_AA, EN_RXADDR, SETUP_AW, SETUP_RETR, RF_CH, RF_SETUP
    nRF24L01_X_write_reg(DevNum, nRF24L01_CONFIG, 0x0f);            // 允许发送中断（MASK_TX_DS=0），使用 2 字节 CRC（EN_CRC=1，CRCO=1)，上电（PWR_UP=1)，设置为接收（Prim_RX=1）。
    nRF24L01_X_write_reg(DevNum, nRF24L01_EN_AA, 0x00);             // 禁能所有接收通道自动应答，后面接收通道设置中单独使能。
    nRF24L01_X_write_reg(DevNum, nRF24L01_EN_RXADDR, 0x00);         // 禁能所有接收通道，后面接收通道设置中单独使能。
    nRF24L01_X_write_reg(DevNum, nRF24L01_SETUP_AW, 0x03);          // 设置通道地址宽度:0x03:5 字节，此处必须注意，不能填 5。
    nRF24L01_X_write_reg(DevNum, nRF24L01_SETUP_RETR, 0x1a);        // 设置重发延时：500us + 86us，重发次数：10
    nRF24L01_X_write_reg(DevNum, nRF24L01_RF_CH, 40);               // 选择 RF 信道号：40
    nRF24L01_X_write_reg(DevNum, nRF24L01_RF_SETUP, 0x0f);          // 数据速率：2Mbps，RF 功率：0dBm，LNA 增益：HCURR
    nRF24L01_X_set_rf_datarate(DevNum, 1);
    
    // nRF24L01 未提供软复位功能，只能逐个将寄存器设置为复位状态。
    nRF24L01_X_write_reg(DevNum, nRF24L01_STATUS, 0x70);            // 清零状态寄存器，尤其是 MAX_RT 位，If MAX_RT  is asserted it must be cleared to enable further communication.
    nRF24L01_X_write_reg(DevNum, nRF24L01_DYNPD, 0x00);
    nRF24L01_X_write_reg(DevNum, nRF24L01_FEATURE, 0x00);
    nRF24L01_X_flush_rx(DevNum);                                    // 清空 RX_FIFO
    nRF24L01_X_flush_tx(DevNum);                                    // 清空 TX_FIFO
    // 将各通道地址设置至上电复位地址
 */
}

/**************************************************
函数：nRF24L01_X_ptx_init(DevNum);

说明：
    本函数用于将 nRF24L01 初始化为 PTX 设备，包括共同
    设置和各通道单独设置。
    初始化后，设备进入 Standby-I 模式，等待发送数据。
**************************************************/
void nRF24L01_X_ptx_init(U8 DevNum)
{
	U8 i;
	U8 aw;

    nRF24L01_X_common_setting(DevNum);
    nRF24L01_X_set_ptx(DevNum);
    
    /**************************************************/
    //                                   通道设置
    /**************************************************/
    // 说明：
    // PTX 和 PRX 通信双方必须约定有效数据宽度，但 PTX
    // 没有发送通道有效数据宽度参数设置，PTX 依据 MCU
    // 每次写入 TX FIFO 的字节数确定发送的有效数据宽度，
    // 最多 32 字节；与此不同的是，接收方 PRX 必须设置
    // 对应接收通道的有效数据宽度参数。
    
    /**************************************************/
    //                               发送通道设置
    /**************************************************/
    // 本机发送通道地址，实际若不主动发送，无必要设，仅作为应答通道。
    // 发送通道设置：{发送通道地址}。
    nRF24L01_X_write_pipe_addr(DevNum, nRF24L01_TX_ADDR, nRF24L01_X_TX_ADDRESS[DevNum], nRF24L01_X_ADR_WIDTH); // 通道地址

    /**************************************************/
    //                               接收通道设置
    /**************************************************/
    /**************************************************/
    // 接收通道 0 设置：
    // 说明：
    // {RX_ADDR_PX: 接收通道地址; RX_PW_PX: 接收通道有效数据宽度}。
    // 作为发送方 PTX，RX_P0 必须使用且使能自动应答（作为自动应答机制启用开关），作为 TX 的自动应答临时接收通道使用。
    // 作为接收方 PRX，RX_P0 可以但不必使用，作为独立接收通道使用。
    // 作为双向通信方，即既作为 PTX 又作为 PRX，则 RX_P0 必须使用；当为 PTX 时，作为自动应答临时接收通道使用；当为 PRX 时，可以作为接收通道，也可以不使用，而使用其他通道作为接收通道。
    /**************************************************/
    // 接收通道 0 设置：{通道使能，通道自动应答使能，通道地址，通道有效数据宽度}。
    // 本机工作在 PTX 模式下，接收通道 0 用于接收本机发送通道的应答。
	if(Dev_Flag[DevNum]==nRF24L01_DEV)			   //nRF24L01 device
	{
		for (i = 0; i <= 5; i++)
	    {
	        aw = (i < 2) ? nRF24L01_X_ADR_WIDTH : 1;
	        nRF24L01_X_set_pipe(
	            DevNum,                                     // DevNum
	            i,                                          // PipeNum     
	            1,                                          // en_pipe  
	            0,                                          // en_auto_ack 
	            nRF24L01_X_RX_ADDRESS_P[DevNum][i],         // *pPipeAddr 
	            aw,                                         // addr_width 
	            nRF24L01_X_PLOAD_WIDTH[i],     // static_plw 
	            0                                           // en_dpl 
	            );
	    }
	}
	else					 //HS6200 device 
	{
		for (i = 0; i <= 5; i++)
	    {
	        aw = (i < 1) ? nRF24L01_X_ADR_WIDTH : 1;
	        nRF24L01_X_set_pipe(
	            DevNum,                                     // DevNum
	            i,                                          // PipeNum     
	            1,                                          // en_pipe  
	            0,                                          // en_auto_ack 
	            nRF24L01_X_RX_ADDRESS_P[DevNum][i],         // *pPipeAddr 
	            aw,                                         // addr_width 
	            nRF24L01_X_PLOAD_WIDTH[i],     // static_plw 
	            0                                           // en_dpl 
	            );
	    }
	}


    //nRF24L01_X_write_reg(DevNum, nRF24L01_RX_PW_P0, nRF24L01_X_RX_PLOAD_WIDTH_P0); // 通道有效数据宽度，因本通道作为应答接收通道，故可设可不设有效数据宽度。
    
    //nRF24L01_X_ce_high(DevNum); // 无数据发送时不必使能 RF，使处于 Standby-I 模式；即使使能也只是转入 Standby-II 模式。
    
    /***********************************************************/
    // 说明：
    // 该设备已设置为发送设备 PTX：
    // 有效数据长度：1 字节，
    // 自动应答，数据速率：2M，RF 信道：40，RF 电平：0dBm，
    // LNA 增益：HCURR，重发延时：500us + 86us，重发次数：10。
    /**********************************************************/
}
/**************************************************/

/**************************************************
函数：nRF24L01_X_prx_init(DevNum);

说明：
    本函数用于将 nRF24L01 初始化为 PRX 设备，包括共同
    设置和各通道单独设置。
    初始化后，CE 置高使能，设备进入接收模式，准备好
    接收数据。
**************************************************/
void nRF24L01_X_prx_init(U8 DevNum)
{
    U8 i, aw;
    
    nRF24L01_X_common_setting(DevNum);
    nRF24L01_X_set_prx(DevNum);
    
    /**************************************************/
    //                                   通道设置
    /**************************************************/
    // 说明：
    // PTX 和 PRX 通信双方必须约定有效数据宽度，但 PTX
    // 没有发送通道有效数据宽度参数设置，PTX 依据 MCU
    // 每次写入 TX FIFO 的字节数确定发送的有效数据宽度，
    // 最多 32 字节；与此不同的是，接收方 PRX 必须设置
    // 对应接收通道的有效数据宽度参数。
    
    /**************************************************/
    //                               发送通道设置
    /**************************************************/
    // 本机发送通道地址，实际若不主动发送，无必要设，仅作为应答通道。
    nRF24L01_X_write_pipe_addr(DevNum, nRF24L01_TX_ADDR, nRF24L01_X_TX_ADDRESS[DevNum], nRF24L01_X_ADR_WIDTH);
    /**************************************************/
    
    /**************************************************/
    //                               接收通道设置
    /**************************************************/
    /**************************************************/
    // 接收通道 0 设置：
    // 说明：
    // {RX_ADDR_PX: 接收通道地址; RX_PW_PX: 接收通道有效数据宽度}。
    // 作为发送方 PTX，RX_P0 必须使用且使能自动应答（作为自动应答机制启用开关），作为 TX 的自动应答临时接收通道使用。
    // 作为接收方 PRX，RX_P0 可以但不必使用，作为独立接收通道使用。
    // 作为双向通信方，即既作为 PTX 又作为 PRX，则 RX_P0 必须使用；当为 PTX 时，作为自动应答临时接收通道使用；当为 PRX 时，可以作为接收通道，也可以不使用，而使用其他通道作为接收通道。
    //---------------------------------------------------------------------------/
    // 接收通道 0 设置：{通道使能，通道自动应答使能，通道地址，通道有效数据宽度，通道动态数据长度使能}。
    
	if(Dev_Flag[DevNum]==nRF24L01_DEV)			   //nRF24L01 device
	{
		for (i = 0; i <= 5; i++)
	    {
	        aw = (i < 2) ? nRF24L01_X_ADR_WIDTH : 1;
	        nRF24L01_X_set_pipe(
	            DevNum,                                     // DevNum
	            i,                                          // PipeNum     
	            1,                                          // en_pipe  
	            0,                                          // en_auto_ack 
	            nRF24L01_X_RX_ADDRESS_P[DevNum][i],         // *pPipeAddr 
	            aw,                                         // addr_width 
	            nRF24L01_X_PLOAD_WIDTH[i],     // static_plw 
	            0                                           // en_dpl 
	            );
	    }
	}
	else					 //HS6200 device 
	{
		for (i = 0; i <= 5; i++)
	    {
	        aw = (i < 1) ? nRF24L01_X_ADR_WIDTH : 1;
	        nRF24L01_X_set_pipe(
	            DevNum,                                     // DevNum
	            i,                                          // PipeNum     
	            1,                                          // en_pipe  
	            0,                                          // en_auto_ack 
	            nRF24L01_X_RX_ADDRESS_P[DevNum][i],         // *pPipeAddr 
	            aw,                                         // addr_width 
	            nRF24L01_X_PLOAD_WIDTH[i],     // static_plw 
	            0                                           // en_dpl 
	            );
	    }
	}

    
  nRF24L01_X_ce_high(DevNum); // Set CE pin high to enable RX device
    
    /***********************************************************/
    // 说明：
    // 该设备已设置为接收设备 PRX，可接收 6 个 TX 设备的数据：
    // 通道 0、1、2、3、4、5，有效数据长度：1 字节，
    // 自动应答，数据速率：2M，RF 信道：40，RF 电平：0dBm，
    // LNA 增益：HCURR，重发延时：500us + 86us，重发次数：10。
    /**********************************************************/
}
/**************************************************/
void nRF24L01_X_write_reg_bits(U8 DevNum, U8 reg, U8 bit_mask, U8 bit_)
{
    nRF24L01_X_write_reg(DevNum, reg, nRF24L01_X_read_reg(DevNum, reg) & (~bit_mask) | bit_);
}

void nRF24L01_X_prx_mode(U8 DevNum)
{
    nRF24L01_X_prx_init(DevNum);
}

void nRF24L01_X_ptx_mode(U8 DevNum)
{
    nRF24L01_X_ptx_init(DevNum);
}

void nRF24L01_X_powerdown(U8 DevNum)
{
    nRF24L01_X_write_reg_bits(DevNum, nRF24L01_CONFIG, nRF24L01_PWR_UP_MASK, nRF24L01_PWR_DWN_CODE);
}

void nRF24L01_X_powerup(U8 DevNum)
{
    nRF24L01_X_write_reg_bits(DevNum, nRF24L01_CONFIG, nRF24L01_PWR_UP_MASK, nRF24L01_PWR_UP_CODE);
}

void nRF24L01_X_standby_I(U8 DevNum)
{
    nRF24L01_X_powerup(DevNum);
    nRF24L01_X_ce_low(DevNum);
}

U8 nRF24L01_X_standby_II(U8 DevNum)
{
    if (nRF24L01_X_is_prx(DevNum)) return (0);
    
    nRF24L01_X_powerup(DevNum);
    nRF24L01_X_flush_tx(DevNum);
//    nRF24L01_X_ptx_mode(DevNum);
    nRF24L01_X_ce_high(DevNum);
    return (1);
}



void nRF24L01_X_spl_mode(U8 DevNum)  //静长
{
    nRF24L01_X_write_reg_bits(DevNum, nRF24L01_FEATURE, nRF24L01_EN_DPL_MASK, ~nRF24L01_EN_DPL_CODE); // close general dpl
    nRF24L01_X_write_reg_bits(DevNum, nRF24L01_DYNPD, 0xff, 0x00); // close each pipe dpl
}

void nRF24L01_X_dpl_mode(U8 DevNum)  //动长
{
    nRF24L01_X_write_reg_bits(DevNum, nRF24L01_FEATURE, nRF24L01_EN_DPL_MASK, nRF24L01_EN_DPL_CODE); // open general
    nRF24L01_X_write_reg_bits(DevNum, nRF24L01_DYNPD, 0xff, 0x3f); // open each pipe
}


void nRF24L01_X_sa_mode(U8 DevNum)   //静答
{ 
    nRF24L01_X_set_en_dyn_ack_feature(DevNum, 0);
}

void nRF24L01_X_da_mode(U8 DevNum)    //动答
{
    nRF24L01_X_set_en_dyn_ack_feature(DevNum, 1);
}

void nRF24L01_X_ack_mode(U8 DevNum)   //有答
{
    nRF24L01_X_set_en_auto_ack_all(DevNum, 0);
    
    if (nRF24L01_X_is_prx(DevNum)) 
    {
        nRF24L01_X_set_en_auto_ack_pipe(DevNum, 0, 1);
        nRF24L01_X_set_en_auto_ack_pipe(DevNum, 1, 1);
        nRF24L01_X_set_en_auto_ack_pipe(DevNum, 2, 1);
        nRF24L01_X_set_en_auto_ack_pipe(DevNum, 3, 1);
        nRF24L01_X_set_en_auto_ack_pipe(DevNum, 4, 1);
        nRF24L01_X_set_en_auto_ack_pipe(DevNum, 5, 1);
    }
    else
    {
        nRF24L01_X_set_en_auto_ack_pipe(DevNum, 0, 1);
    }
}

void nRF24L01_X_noack_mode(U8 DevNum)  //无答
{
    nRF24L01_X_set_en_auto_ack_all(DevNum, 0);
    
    if (nRF24L01_X_is_dyn_ack_feature(DevNum)) // 动无答，发送方在发送前执行 NOACK 指令
    {
        if (nRF24L01_X_is_prx(DevNum)) 
        {
            nRF24L01_X_set_en_auto_ack_pipe(DevNum, 0, 1);
            nRF24L01_X_set_en_auto_ack_pipe(DevNum, 1, 1);
            nRF24L01_X_set_en_auto_ack_pipe(DevNum, 2, 1);
            nRF24L01_X_set_en_auto_ack_pipe(DevNum, 3, 1);
            nRF24L01_X_set_en_auto_ack_pipe(DevNum, 4, 1);
            nRF24L01_X_set_en_auto_ack_pipe(DevNum, 5, 1);
        }
        else
        {
            nRF24L01_X_set_en_auto_ack_pipe(DevNum, 0, 1);
        }
    }
    else // 静无答，即 SB 模式
    {
        nRF24L01_X_set_rf_datarate(DevNum, 1); // SB 模式必须使用 1M 或 250K bps，不可使用 2Mbps。
    }
}

void nRF24L01_X_ack_empty_mode(U8 DevNum)  //空答
{
    nRF24L01_X_set_en_ack_pay_feature(DevNum, 0);
}

void nRF24L01_X_ack_pay_mode(U8 DevNum)  //载答
{
    nRF24L01_X_set_en_ack_pay_feature(DevNum, 1);
}

void nRF24L01_X_noretr_mode(U8 DevNum)  //无重发模式
{
    nRF24L01_X_set_auto_retr_count(DevNum, 0);
}
void nRF24L01_X_ar_mode(U8 DevNum, U8 arc)  //自动重发模式
{
    nRF24L01_X_set_auto_retr_count(DevNum, arc);
}

void nRF24L01_X_mr_mode(U8 DevNum, U8 en)   //手动重发模式
{
    if (en)
    {
        // 预先向 TX_FIFO 写入要发送的数据并发送一次，然后执行 REUSE_TX_FIFO 指令。
        // 此后发送程序只需发出 CE 高脉冲即可重传上次发送的数据。
        nRF24L01_X_Tx_Buf[DevNum][0] = 0x01;
        // nRF24L01_X_send_packet(DevNum, nRF24L01_X_TX_BUF[DevNum], nRF24L01_X_TX_PLOAD_WIDTH, nRF24L01_X_is_auto_ack_px(DevNum, 0));
        // nRF24L01_X_write_tx_payload(DevNum, nRF24L01_X_TX_BUF[DevNum], 1);
        chThdSleepMilliseconds(10);
        // if (!nRF24L01_X_Busy[DevNum])
        nRF24L01_X_reuse_tx_payload(DevNum);
    }
    else
    {
        nRF24L01_X_flush_tx(DevNum); // 清除 REUSE_TX 标志
        // nRF24L01_X_reuse_tx_payload(DevNum);
    }
}


/*--------------------------------------------------------------------------------------------------------------------------------------*/
U8 nRF24L01_X_get_rf_ch_idx(const U8 rf_ch, U8 *rf_ch_table, const U8 table_length)  //get rf_ch index 
{
    U8 i, found = 0;
    
    for (i = 0; i < table_length; i++)
    {
        if (*(rf_ch_table + i) == rf_ch) 
        {
            found = 1;
            break;
        }
    }
    
    if (found) return i;
    else return 127;  // current_rf_ch not found
}

U8 nRF24L01_X_get_next_rf_ch(U8 current_rf_ch, U8 *rf_ch_table, U8 table_length)  //get current_rf_ch's next RF channel
{
    U8 idx;
    
    idx = (nRF24L01_X_get_rf_ch_idx(current_rf_ch, rf_ch_table, table_length) + 1);
    if (idx == table_length) idx = 0;
    if (idx < table_length) return *(rf_ch_table + idx);
    else return (idx);
}

bool_t nRF24L01_X_is_rf_ch_clean(U8 DevNum, U8 rf_ch, U8 gate)    //rf_ch is clean?
{
    U8 temp_rf_ch, i = 0, rpd_cnt = 0;
    bool_t prx, ce;
    
    ce = nRF24L01_X_is_ce_high(DevNum);
    nRF24L01_X_ce_low(DevNum);
        prx = nRF24L01_X_is_prx(DevNum);
        nRF24L01_X_set_prx(DevNum);
        temp_rf_ch = nRF24L01_X_get_rf_ch(DevNum);
        nRF24L01_X_set_rf_ch(DevNum, rf_ch);
        
        nRF24L01_X_ce_high(DevNum);
            for (rpd_cnt = 0, i = 0; i < 100; i++)    //measure 100 times
            {
                chThdSleepMicroseconds(20); // delay_us(nRF24L01_Tstby2a + nRF24L01_Tdelay_AGC + 10);
                if (nRF24L01_X_is_rpd(DevNum)) rpd_cnt += rpd_cnt==255 ? 0 : 1;  //if clean,rpd_cnt+=0;else rpd_cnt++ 
            }
        nRF24L01_X_ce_low(DevNum);
        
        nRF24L01_X_set_rf_ch(DevNum, temp_rf_ch);
        prx ? nRF24L01_X_set_prx(DevNum) : nRF24L01_X_set_ptx(DevNum);
    ce ? nRF24L01_X_ce_high(DevNum) : nRF24L01_X_ce_low(DevNum);
    
    return (rpd_cnt <= gate);      //if rpd_cnt>gata times,return 1,eles return 0  
}

bool_t nRF24L01_X_is_rf_ch_dirty(U8 DevNum, U8 rf_ch, U8 gate)   //rf_ch is dirty
{
    return !nRF24L01_X_is_rf_ch_clean(DevNum, rf_ch, gate);
}

U8 nRF24L01_X_search_next_clean_rf_ch(U8 DevNum, U8 current_rf_ch, U8 *rf_ch_table, U8 table_length, U8 gate)  //search next clean channel
{
    U8 i, found = 0;
    
    for (i = 0; i < table_length; i++)
    {
        if (*(rf_ch_table + i) == current_rf_ch) 
        {
            found = 1;
            break;
        }
    }
    
    if (found)     //current_rf_ch is in rf_ch_table
    {
        found = i;
        for (i = i + 1; i < table_length; i++) // 从当前信道向后寻找
        {
            if (nRF24L01_X_is_rf_ch_clean(DevNum, *(rf_ch_table + i), gate)) 
            {
                return *(rf_ch_table + i);
            }
        }
        
        for (i = 0; i <= found; i++) // 若上面未找到，则从第一个信道寻找到当前信道
        {
            if (nRF24L01_X_is_rf_ch_clean(DevNum, *(rf_ch_table + i), gate)) 
            {
                return *(rf_ch_table + i);
            }
        }
        
        return (126); // clean_rf_ch not found
    }
    else 
        return (127); // current_rf_ch error 
}

U8 nRF24L01_X_search_next_dirty_rf_ch(U8 DevNum, U8 current_rf_ch, U8 *rf_ch_table, U8 table_length, U8 gate)  //search next dirty channel
{
    U8 i, found = 0;
    
    for (i = 0; i < table_length; i++)
    {
        if (*(rf_ch_table + i) == current_rf_ch) 
        {
            found = 1;
            break;
        }
    }
    
    if (found)
    {
        found = i;
        for (i = i + 1; i < table_length; i++) // 从当前信道向后寻找
        {
            if (nRF24L01_X_is_rf_ch_dirty(DevNum, *(rf_ch_table + i), gate)) 
            {
                return *(rf_ch_table + i);
            }
        }
        
        for (i = 0; i <= found; i++) // 若上面未找到，则从第一个信道寻找到当前信道
        {
            if (nRF24L01_X_is_rf_ch_dirty(DevNum, *(rf_ch_table + i), gate)) 
            {
                return *(rf_ch_table + i);
            }
        }
        
        return (126); // clean_rf_ch not found
    }
    else 
        return (127); // current_rf_ch error 
}

U8 nRF24L01_X_search_best_rf_ch(U8 DevNum, U8 current_rf_ch, U8 *rf_ch_table, U8 table_length, U8 gate_max)   //search best RF channel
{
    U8 gate, rf_ch;
    
    for (gate = 0; gate <= gate_max; gate++)     //search for best RF chanel--->for(gate=gate_max;gata>0;gate--) 
    {
        if ((rf_ch = nRF24L01_X_search_next_clean_rf_ch(DevNum, current_rf_ch, rf_ch_table, table_length, gate)) < 126) return rf_ch;
    }
    
    return rf_ch;
}

U8 nRF24L01_X_search_worst_rf_ch(U8 DevNum, U8 current_rf_ch, U8 *rf_ch_table, U8 table_length, U8 gate_min)   //get worst RF channel
{
    U8 gate, rf_ch;
    
    for (gate = 100; gate >= gate_min; gate--)
    {
        if ((rf_ch = nRF24L01_X_search_next_dirty_rf_ch(DevNum, current_rf_ch, rf_ch_table, table_length, gate)) < 126) return rf_ch;
    }
    
    return rf_ch;
}

U8 nRF24L01_X_ptx_fh(U8 DevNum, U8 *rf_ch_table, U8 table_length)
{
    U8 i = 0, j = 0, rf_ch_idx = 255;
    
    if (nRF24L01_X_is_prx(DevNum)) return 252;
    if (!nRF24L01_X_get_plos_cnt(DevNum)) return 253;
    rf_ch_idx = nRF24L01_X_get_rf_ch_idx(nRF24L01_X_get_rf_ch(DevNum), rf_ch_table, table_length);
    if (rf_ch_idx >= table_length) return 254; // rf_ch not found in the table
    
    nRF24L01_X_ce_low(DevNum);
    for (i = 0; i < table_length; i++) // 跳频一周，未果，回到原点，不再跳。
    {
        for (j = 0; j < 3; j++)
        {
            nRF24L01_X_set_rf_ch(DevNum, rf_ch_table[rf_ch_idx]);
            nRF24L01_X_Busy[DevNum] = 1;
                nRF24L01_X_ce_high_pulse(DevNum);
            while (nRF24L01_X_Busy[DevNum]);
            
            if (!nRF24L01_X_get_plos_cnt(DevNum)) 
            {
//                LED_BLINK(rf_ch_table[rf_ch_idx]);
                return rf_ch_table[rf_ch_idx];
            }
        }
        
        rf_ch_idx = (rf_ch_idx == table_length-1) ? 0 : (rf_ch_idx+1);
    }
    
    nRF24L01_X_clear_plos_cnt(DevNum);
    nRF24L01_X_flush_tx(DevNum);
    
    return 255;
}

U8 nRF24L01_X_prx_fh(U8 DevNum, U8 *rf_ch_table, U8 table_length)
{
    U8 rf_ch = 255;
    
    if (nRF24L01_X_is_prx(DevNum) && nRF24L01_X_is_rf_ch_dirty(DevNum,  nRF24L01_X_get_rf_ch(DevNum), 10))   //prx and current rf channel is dirty
    {
        rf_ch = nRF24L01_X_search_best_rf_ch(DevNum, nRF24L01_X_get_rf_ch(DevNum), rf_ch_table, table_length, 3); //search best rf channel
        if (rf_ch < 126) 
        {
            nRF24L01_X_ce_low(DevNum);
                nRF24L01_X_set_rf_ch(DevNum, rf_ch);
            nRF24L01_X_ce_high(DevNum);
 //           LED_BLINK(rf_ch);
        }
//        else LED_SET(rf_ch);
    }
    return rf_ch;
}




/**************************************************/
U8 nRF24L01_X_test_Reg_ReadVSValue(U8 DevNum, U8 reg, U8 value) //test register value
{
    return (nRF24L01_X_read_reg(DevNum, reg) == value);
}



U8 nRF24L01_X_test_Reg_PipeAddr_ReadVSValue(U8 DevNum, U8 Px_Addr_Reg, nRF24L01_PipeAddr_tag PipeAddr, U8 Addr_Width)  //test pipe addr
{
    nRF24L01_PipeAddr_tag pa;
    U8 buf[5];
    U8 temp;

    temp = Addr_Width;
    
    nRF24L01_X_read_pipe_addr(DevNum, Px_Addr_Reg, buf, 5);  //read pipe address   5 bytes
    pa.h4=((unsigned long)buf[4]<<24) + ((unsigned long)buf[3]<<16) + ((unsigned long)buf[2]<<8) + ((unsigned long)buf[1]);
    pa.l1=buf[0];
    
    return (pa.h4==PipeAddr.h4)&&(pa.l1==PipeAddr.l1);
}

U8 nRF24L01_X_test_Reg_ReadVSWrite(U8 DevNum, U8 reg, U8 value) //test write register value
{
    nRF24L01_X_write_reg(DevNum, reg, value);
    return nRF24L01_X_test_Reg_ReadVSValue(DevNum, reg, value);
}


U8 nRF24L01_X_test_Reg_PipeAddr_ReadVSWrite(U8 DevNum, U8 Px_Addr_Reg, nRF24L01_PipeAddr_tag PipeAddr, U8 Addr_Width) //test write pipe address value 
{
    U8 buf[5];
    
    buf[4]=(U8)(PipeAddr.h4>>24);
    buf[3]=(U8)(PipeAddr.h4>>16);
    buf[2]=(U8)(PipeAddr.h4>>8);
    buf[1]=(U8)(PipeAddr.h4>>0);
    buf[0]=PipeAddr.l1;
    nRF24L01_X_write_pipe_addr(DevNum, Px_Addr_Reg, buf, 5);
    return nRF24L01_X_test_Reg_PipeAddr_ReadVSValue(DevNum, Px_Addr_Reg, PipeAddr, Addr_Width);
}


nRF24L01_PipeAddr_tag nRF24L01_X_RX_ADDR_P0_RESET_VALUE_struct = {nRF24L01_RX_ADDR_P0_RESET_VALUE_H4, nRF24L01_RX_ADDR_P0_RESET_VALUE_L1};  //pipe0 address 0xE7E7E7E7E7
nRF24L01_PipeAddr_tag nRF24L01_X_RX_ADDR_P1_RESET_VALUE_struct = {nRF24L01_RX_ADDR_P1_RESET_VALUE_H4, nRF24L01_RX_ADDR_P1_RESET_VALUE_L1};  //pipe1 address 0xC2C2C2C2C2
nRF24L01_PipeAddr_tag nRF24L01_X_TX_ADDR_RESET_VALUE_struct      = {nRF24L01_TX_ADDR_RESET_VALUE_H4, nRF24L01_TX_ADDR_RESET_VALUE_L1};      //TX address 0xE7E7E7E7E7


U8 nRF24L01_X_read_fifo_status(U8 DevNum)    //Bank0 Bank1 status has the same address
{
	U8 status=0x00;
	status=nRF24L01_X_read_reg(DevNum,nRF24L01_FIFO_STATUS);
	return status;			
}
U8 nRF24L01_X_read_cfg_word(U8 DevNum)
{
	U8 cfg_word=0x00;
	HS6200_Bank0_Activate(DevNum);			//in Bank0 status
	cfg_word=nRF24L01_X_read_reg(DevNum,nRF24L01_FIFO_STATUS);
	return cfg_word;	
}

#endif

/*--------------------------------------End Of File----------------------------------------*/
