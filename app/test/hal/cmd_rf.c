//------------------------------------------------------------------------------------
// test_rf.c
//------------------------------------------------------------------------------------
// Copyright 2015, HunterSun Electronics Co., Ltd.
// shangzhou.hu@huntersun.com.cn
// 2015-03-20
//
// Program Description: MAC6200 RF test program
//
//
//

#include <string.h>
#include <stdlib.h>
#include "ch.h"
#include "hal.h"
#include "cmsis_os.h"
#include "chprintf.h"

#define HS_MAC6200_USE_BTPHY TRUE //true for 66xx; false for 62xx

#if HAL_USE_RF

#define PKT_LENGTH  32

#define ROLE_PTX    0x00
#define ROLE_PRX    0x01
#define COMM_ROLE   0x01

#define CRC_1BYTE    0x04
#define CRC_2BYTE    0x06
#define NO_CRC       0x00
#define COMM_CRC     0x06

#define DR_2MBPS     0x00
#define DR_1MBPS     0x08
#define DR_500KBPS   0x10
#define DR_250KBPS   0x18
#define COM_DR       0x18

#define TX_GARD_ENABLE  0x20
#define TX_GARD_DISABLE 0x00
#define COM_TX_GARD  0x20

#define MODE_SPL     0x00
#define MODE_DPL     0x80
#define COM_PL_MODE  0x80

#define MODE_NOACK   0x00
#define MODE_ACK     0x40
#define COM_MODE     0x40

#define COM_MUTIFIFO_ENABLE  0x10
#define MUTIFIFO_DISABLE     0x00
#define MUTIFIFO_ENABLE      0x10

#define	ETIMEDOUT	110	/* Connection timed out */

#define delay_us(n)  chSysPolledDelayX(CPU_DEFAULT_CLOCK/1000000 * n)

extern void HS6601_mac6200_init(void);
extern void MAC6200_Analog_Init(void);
extern void MAC6200_Calibration(void);
static int mhz = 2440, timeout = 5000, window = 100, interval = 100, count_run = 100;

static bool mode_dpl = false;
static bool mode_ack = false;
static uint8_t mode_rate = MAC6200_BANK0_DR_1M;
static uint8_t mode_crc = 0;
static uint8_t mode_code = 0; //ch_code
static bool mode_guard = true;
static bool mode_fifo = false;
#if HS_MAC6200_USE_BTPHY
static uint8_t txgain_lvl = 0;
#else
static uint8_t txgain_lvl = 15;
#endif

static uint8_t  TX_payload[PKT_LENGTH*3];
static uint8_t  RX_payload[PKT_LENGTH*3];

static uint8_t   error_flag = 0;
static uint16_t  error_bit = 0;
static uint16_t  error_count = 0;
static uint16_t  Right_Count = 0;

static uint8_t m_reg_bytes[] = {
  1, 1, 1, 1, 1, 1, 1, 1, //bank0
  1, 1, 5, 1, 1, 1, 1, 1,
  5, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 1, 1, 5, 3,
  2, 4, 4, 5, 1, 1, 0, 1, //bank1
  2, 4, 3, 3, 1, 1, 1, 2,
  2, 4, 4, 4, 0, 0, 0, 1,
  1, 0, 1, 1, 4, 4, 4, 4,
};

static  uint8_t bitcount(uint8_t n)  
{
  uint8_t count=0 ;
  while (n)  {
    count++ ;
    n &= (n - 1) ;
  }
  return count ;
}

static int rf62_wait_rx(BaseSequentialStream *chp)
{
  uint32_t start, end;
  uint8_t status;

  //palSetPadMode(IOPORT0, 10 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));  // 6
  //palClearPad(IOPORT0, 10);
  start = osKernelSysTick();
  MAC6200_Read_Reg(MAC6200_BANK0_STATUS, &status, 1);
  while (0 == (status & MAC6200_RX_DR)) {
    end = osKernelSysTick();
    if (timeout && ((int)(end - start) > timeout)) {
      uint8_t state = 0;
      MAC6200_Bank1_Activate();
      MAC6200_Read_Reg(MAC6200_BANK1_STATE, &state, 1);
      MAC6200_Bank0_Activate();
      chprintf(chp, "status=%02x state=%02x\r\n", status, state);
      chprintf(chp, "\r\nch:%d  Total:%d  Fail:%u  Pass:%u  ErrorBits:%u\r\n", mhz, (error_count+Right_Count), error_count, Right_Count, error_bit);
      return -ETIMEDOUT;
    }
    MAC6200_Read_Reg(MAC6200_BANK0_STATUS, &status, 1);
  }
  //palSetPad(IOPORT0, 10);
  /* w1c */
  status &= MAC6200_RX_DR;
  MAC6200_Write_Reg(MAC6200_BANK0_STATUS, &status, 1);
  chprintf(chp, ".");
  return 0;
}

static int rf62_wait_tx(BaseSequentialStream *chp)
{
  uint32_t start, end;
  uint8_t status;
  int err = 0;

  start = osKernelSysTick();
  MAC6200_Read_Reg(MAC6200_BANK0_STATUS, &status, 1);
  while (0 == (status & (MAC6200_TX_DS | MAC6200_MAX_RT))) {
    end = osKernelSysTick();
    if (timeout && ((int)(end - start) > timeout)) {
      uint16_t state;
      MAC6200_Bank1_Activate();
      MAC6200_Read_Reg(MAC6200_BANK1_STATE, (uint8_t *)&state, 2);
      MAC6200_Bank0_Activate();
      chprintf(chp, "status=%02x state=%02x\r\n", status, state);
      //return -ETIMEDOUT;
      err = -ETIMEDOUT;
      goto out;
    }
    MAC6200_Read_Reg(MAC6200_BANK0_STATUS, &status, 1);
  }
  /* w1c */
  status &= MAC6200_TX_DS | MAC6200_MAX_RT;
  MAC6200_Write_Reg(MAC6200_BANK0_STATUS, &status, 1);
  if (status & MAC6200_MAX_RT) {
    err = -1;
    goto out;
  }
  chprintf(chp, ".");

 out:
  MAC6200_dump_RF_register(chp);
  return err;
}

static void _rf62_setup_txgain(uint8_t lvl)
{
#if HS_MAC6200_USE_BTPHY
  HS_ANA->REGS.GSEL_PA = lvl & 0x03; //HS6601 only
#else
  uint8_t rf_setup;

  MAC6200_Read_Reg(MAC6200_BANK0_RF_SETUP, &rf_setup, 1);
  rf_setup &= ~((1u << 6) | (7u << 0)); //PA_PWR[3:0]
  rf_setup |= lvl & 0x8 ? (1u << 6) : 0;
  rf_setup |= lvl & 0x7;
  MAC6200_Write_Reg(MAC6200_BANK0_RF_SETUP, &rf_setup, 1);
#endif
}

static void _rf62_setup_guard(bool en)
{
  uint8_t cal_ctl[5];
  uint8_t config;

#if HS_MAC6200_USE_BTPHY //HS6601 & HS6620
  MAC6200_Read_Reg(MAC6200_BANK0_CONFIG, &config, 1);
  if (en)
    config |= 0x80;
  else
    config &= ~0x80;
  MAC6200_Write_Reg(MAC6200_BANK0_CONFIG, &config, 1);
#endif

  MAC6200_Bank1_Activate();
  MAC6200_Read_Reg(MAC6200_BANK1_CAL_CTL, cal_ctl, 5);
  if (en)
    cal_ctl[2] |= (1u << 4);  //scramble_en
  else
    cal_ctl[2] &= ~(1u << 4);  //和nrf2401通讯，需要去白化
  MAC6200_Write_Reg(MAC6200_BANK1_CAL_CTL, cal_ctl, 5);
  MAC6200_Bank0_Activate();
}

static void _rf62_setup_rate(uint8_t rate)
{
  uint8_t rf_setup;

  MAC6200_Read_Reg(MAC6200_BANK0_RF_SETUP, &rf_setup, 1);
  rf_setup &= ~((1u << 5) | (1u << 3)); //RF_DR_LOW,RF_DR_HIGH
  rf_setup |= rate;
  MAC6200_Write_Reg(MAC6200_BANK0_RF_SETUP, &rf_setup, 1);
}

static void _rf62_setup_code(uint8_t code)
{
  uint8_t fagc_ctrl[4];

  MAC6200_Bank1_Activate();
  MAC6200_Read_Reg(MAC6200_BANK1_FAGC_CTRL, fagc_ctrl, 4);
  fagc_ctrl[0] &= ~((1u << 7) | (1u << 6)); //ch_code
  fagc_ctrl[0] |= code << 6;
  MAC6200_Write_Reg(MAC6200_BANK1_FAGC_CTRL, fagc_ctrl, 4);
  MAC6200_Bank0_Activate();
}

static void _rf62_setup_crc(uint8_t crc)
{
  uint8_t config;

  MAC6200_Read_Reg(MAC6200_BANK0_CONFIG, &config, 1);
  if (crc == 0) {
    uint8_t en_aa = 0;
    config &= ~MAC6200_CRCO;
    config &= ~MAC6200_EN_CRC;
    /* clear EN_AA before clear CONFIG.EN_CRC */
    MAC6200_Write_Reg(MAC6200_BANK0_EN_AA, &en_aa, 1);
  }
  else {
    config &= ~MAC6200_CRCO;
    config |= MAC6200_EN_CRC;
    if (crc == 1)
      config |= MAC6200_CRCO_1_BYTES;
    else if (crc == 2)
      config |= MAC6200_CRCO_2_BYTES;
  }
  MAC6200_Write_Reg(MAC6200_BANK0_CONFIG, &config, 1);
}

static void _rf62_setup_freq(int freq)
{
#if HS_MAC6200_USE_BTPHY

  HS_BTPHY->SPI_APB_SWITCH=0;

  /* workaround: [8]bt_synth_con_mmd_mn=1 [7:0]bt_synth_con_mmd=0x00 to turn off mmd during BT's RXENA is high, before change the new frequency */
  //__hal_set_bitsval(HS_ANA->SDM_CFG, 0, 8, 0x100);
  osDelay(1);

  HS_BTPHY->ANALOGUE[0x44] = freq; //frequency: 2392 ~ 2490 MHz
  HS_BTPHY->ANALOGUE[0x44] = freq+(1<<12); //[12]: w1 trigger?

  /* this step is not required if BT TRX? */
  HS_ANA->VCO_AFC_CFG[0] |= (1<<16); //[16]rf_pll_afc_tlu

  /* workaround: [8]bt_synth_con_mmd_mn=1 to turn on mmd, FM PLL will become lock */
  //__hal_set_bitsval(HS_ANA->SDM_CFG, 0, 8, 0x000);

  HS_BTPHY->SPI_APB_SWITCH=1;

#else

  uint8_t reg_val;
  reg_val = mhz - 2400;
  MAC6200_Write_Reg(MAC6200_BANK0_RF_CH, &reg_val, 1);    //channel

#endif
}


/* _____________________________________________argu bitmap______________________________________________________
|                                                                                                                |
| BIT7 |      BIT6         |    BIT5       |   BIT4        |     BIT3       |   BIT2     |    BIT1  |    BIT0    |
|________________________________________________________________________________________________________________|
|                                                          |                                                     |
|  DPL  |   COM_MODE       |   TX_GARD     |   MutiFifo    |    Addr_Width   |  CRC_EN    |	  CRC     |  ROLE    |
|________________________________________________________________________________________________________________|
|                                                                                                                |
| 0:SPL |    0:MODE_NOACK  |  00: disable  |  0: disable   |   0:5Byte    |  0: disable  |  0: 1Byte  |   0:PTX  |
| 1:DPL |    1:MODE_ACK    |  01: enable   |  1: enable    |   1:4Byte    |  1: enable   |  1: 2Byte  |   1:PRX  |
|       |                  |               |               |              |	             |  	      |  	     |
|________________________________________________________________________________________________________________|
*/   

static void MAC6200_RF_Test_SPL_No_ACK(unsigned char argu, BaseSequentialStream *chp)
{
    uint8_t  i=0;
    uint8_t reg_value;
 	int count = count_run;
    int payload_Width;

    reg_value=0x00;
	MAC6200_Write_Reg(MAC6200_BANK0_EN_AA, &reg_value, 1);    //ENAA
    reg_value=PKT_LENGTH;
	MAC6200_Write_Reg(MAC6200_BANK0_RX_PW_P0, &reg_value, 1);  //Payload width 32Byte
    reg_value=0x00;
	MAC6200_Write_Reg(MAC6200_BANK0_DYNPD, &reg_value, 1);   
    reg_value=0x00;
	MAC6200_Write_Reg(MAC6200_BANK0_FEATURE, &reg_value, 1); 
	chprintf(chp, "静长静无答模式->ch:%d\r\n", mhz);   
    MAC6200_dump_RF_register(chp);

	for(i=0;i<PKT_LENGTH;i++)
		RX_payload[i] = i+1;	
	for(i=0;i<PKT_LENGTH;i++)
		TX_payload[i] = i+1;

	if((argu&COMM_ROLE) == ROLE_PTX)    //PTX
	{
        MAC6200_CE_High();
        
		while(count--)
		{
            //for(i=0x00; i<PKT_LENGTH; i++) chprintf(chp, "%02x ", TX_payload[i]); chprintf(chp, "\r\n");

		    MAC6200_Write_Tx_Payload(TX_payload, PKT_LENGTH);

            if (-ETIMEDOUT == rf62_wait_tx(chp))
                break;

            osDelay(20);
            MAC6200_Flush_Rx_Fifo();
			MAC6200_Flush_Tx_Fifo();
		}
        chprintf(chp, "\r\nch=%d: %u sent\r\n", mhz, count_run);
	}
	else { //PRX
		while(1) {
		    memset(RX_payload, 0, PKT_LENGTH);
            
            MAC6200_CE_High();

            if (-ETIMEDOUT == rf62_wait_rx(chp))
                break;
            
			payload_Width = MAC6200_Read_Rx_Payload_Width();
			MAC6200_Read_Rx_Payload(RX_payload, payload_Width);
	
		    reg_value=0x70;
		    MAC6200_Write_Reg(MAC6200_BANK0_STATUS, &reg_value,1);   //clear STATUS register.
		    MAC6200_Flush_Rx_Fifo();
		    MAC6200_Flush_Tx_Fifo();
	
		    for(i=0x00;i<payload_Width;i++)
			{	
				if((i+1) != RX_payload[i]) {
				   error_flag = 1;
				   error_bit += bitcount((i+1)^RX_payload[i]);	
				}
			}
	
			if(error_flag)
			  error_count++;	
			else 
			  Right_Count++;
	
			if(error_flag) {
				error_flag = 0;
				chprintf(chp, "err Pkt:");
				for(i=0; i<payload_Width; i++) 
					chprintf(chp, "%02x ", RX_payload[i]);
				chprintf(chp, "\r\n");
			}		
		}
	}
    MAC6200_CE_Low();
}



static void MAC6200_RF_Test_DPL_No_ACK(unsigned char argu, BaseSequentialStream *chp)
{
    uint8_t  i=0, j;
    uint8_t reg_value;
 	int count = count_run;
    int payload_Width;

    reg_value=0x00;
	MAC6200_Write_Reg(MAC6200_BANK0_EN_AA, &reg_value, 1);    //ENAA
    reg_value=PKT_LENGTH;
	MAC6200_Write_Reg(MAC6200_BANK0_RX_PW_P0, &reg_value, 1);  //Payload width 32Byte
    reg_value=0x3f;
	MAC6200_Write_Reg(MAC6200_BANK0_DYNPD, &reg_value, 1);   
    reg_value=0x04;
	MAC6200_Write_Reg(MAC6200_BANK0_FEATURE, &reg_value, 1); 
	chprintf(chp, "动长静无答模式->ch:%d\r\n", mhz);   
    MAC6200_dump_RF_register(chp);

	for(i=0;i<PKT_LENGTH;i++)
		RX_payload[i] = i+1;	
	for(i=0;i<PKT_LENGTH;i++)
		TX_payload[i] = i+1;

	if((argu&COMM_ROLE) == ROLE_PTX)    //PTX
	{
		j=1;
		while(count--)
		{
            //for(i=0x00; i<j; i++) chprintf(chp, "%02x ", TX_payload[i]);  chprintf(chp, "\r\n");
		    MAC6200_Write_Tx_Payload(TX_payload, j);

		    MAC6200_CE_High();
            if (-ETIMEDOUT == rf62_wait_tx(chp))
                break;
	        MAC6200_CE_Low(); 

		    reg_value=0x70;
		    MAC6200_Write_Reg(MAC6200_BANK0_STATUS, &reg_value,1);   //clear STATUS register.
	
		    MAC6200_Flush_Rx_Fifo();
			MAC6200_Flush_Tx_Fifo();
			j++;
			if(j>32) {
                //chprintf(chp, "ch=%d,  %uth send\r\n", mhz, count_run-count);
				j=1;
			}
            osDelay(50);
		}
        chprintf(chp, "\r\nch=%d: %u sent\r\n", mhz, count_run);
	}
	else { //PRX

		while(1) {
		    for(i=0x00; i<PKT_LENGTH; i++)
		       RX_payload[i]=0;
	
		    MAC6200_CE_High();
            if (-ETIMEDOUT == rf62_wait_rx(chp))
                break;
	        MAC6200_CE_Low();
            
		    payload_Width=MAC6200_Read_Rx_Payload_Width();  //read rx payload length.
		    MAC6200_Read_Rx_Payload(RX_payload, payload_Width);
	
		    reg_value=0x70;
		    MAC6200_Write_Reg(MAC6200_BANK0_STATUS, &reg_value,1);   //clear STATUS register.
		    MAC6200_Flush_Rx_Fifo();
		    MAC6200_Flush_Tx_Fifo();
	
		    for(i=0x00;i<payload_Width;i++)
			{	
				if((i+1) != RX_payload[i]) {
				   error_flag = 1;
				   error_bit += bitcount((i+1)^RX_payload[i]);	
				}
			}
	
			if(error_flag)
			  error_count++;	
			else 
			  Right_Count++;
	
			if(error_flag) {
				error_flag = 0;
				chprintf(chp, "err Pkt:");
				for(i=0; i<payload_Width; i++) 
					chprintf(chp, "%02x ", RX_payload[i]);
				chprintf(chp, "\r\n");
			}		
		}
	}
    MAC6200_CE_Low(); 
}


static void MAC6200_RF_Test_DPL_ACK(unsigned char argu, BaseSequentialStream *chp)
{
	uint16_t  null_ack_count=0;
	uint8_t  max_retry_count=0;
	uint16_t  no_ack_count=0;
	uint16_t ack_count=0;
    uint8_t  i=0, j;
    uint8_t reg_value;
 	int count = count_run;
    int payload_Width;

    reg_value=0x3f;
	MAC6200_Write_Reg(MAC6200_BANK0_EN_AA, &reg_value, 1);    //ENAA
    reg_value=PKT_LENGTH;
	MAC6200_Write_Reg(MAC6200_BANK0_RX_PW_P0, &reg_value, 1);  //Payload width 32Byte
    reg_value=0x3f;
	MAC6200_Write_Reg(MAC6200_BANK0_DYNPD, &reg_value, 1);   
    reg_value=0x07;
	MAC6200_Write_Reg(MAC6200_BANK0_FEATURE, &reg_value, 1); 
	chprintf(chp, "动长动载答模式->ch:%d\r\n", mhz);
    MAC6200_dump_RF_register(chp);

	for(i=0;i<PKT_LENGTH;i++)
		RX_payload[i] = i+1;	
	for(i=0;i<PKT_LENGTH;i++)
		TX_payload[i] = i+1;

	if((argu&COMM_ROLE) == ROLE_PTX)    //PTX
	{
		j=3;
		while(count--)
		{
            int err;
			if(1) {//rand()%2) {	 //随机以MAC6200_W_TX_PAYLOAD_NOACK或MAC6200_W_TX_PAYLOAD命令发包
			    chprintf(chp, "TX      : ");
			    for(i=0x00; i<j; i++)
			       chprintf(chp, "%d ", (int)TX_payload[i]);
				chprintf(chp, "\r\n");
				MAC6200_Write_Tx_Payload(TX_payload, j);
				ack_count++;
			}
			else {	
              chprintf(chp, "TX_NoACK: ");
              for(i=0x00; i<j; i++)
                 chprintf(chp, "%d ", (int)TX_payload[i]);
              chprintf(chp, "\r\n");
              MAC6200_Write_Tx_Payload_No_Ack(TX_payload, j);
              no_ack_count++;
			}
			
						
            MAC6200_CE_High();
            err = rf62_wait_tx(chp);
            if (err == -ETIMEDOUT)
                break;
            osDelay(1);
	        MAC6200_CE_Low();

			if (err) {  // Max retry interrupt
				chprintf(chp, "MAX_RT interrupt\r\n");
				max_retry_count++;
			}
			else {				   // recevice ack packet
			    payload_Width=MAC6200_Read_Rx_Payload_Width();  //read ack payload length.
				if(	payload_Width>0 ) {
		        	MAC6200_Read_Rx_Payload(RX_payload, payload_Width);	//read ack payload
					chprintf(chp, "ACK     : ");
					for(i=0; i<payload_Width; i++) 
						chprintf(chp, "%d ", (int)RX_payload[i]);
					chprintf(chp, "\r\n");
	
			   	    for(i=0x00;i<payload_Width;i++)		//check ack payload packet
					{	
						if((i+1) != RX_payload[i]) {
						   error_flag = 1;
						   error_bit += bitcount((i+1)^RX_payload[i]);	
						}
					}
					if(error_flag) {
					  error_count++;
					  error_flag = 0;
					}	
					else 
					  Right_Count++;
				}
			    else 
				   null_ack_count++;
			}
			    

		    reg_value=0x70;
		    MAC6200_Write_Reg(MAC6200_BANK0_STATUS, &reg_value,1);   //clear STATUS register.
	
		    MAC6200_Flush_Rx_Fifo();
			MAC6200_Flush_Tx_Fifo();
			j++;
			if(j>32) {
				j=1;
                //chprintf(chp, "T:%d TX_NoACK:%d TX_ACK:%d MAX_RT:%d NULL_ACK:%d ", (ack_count+no_ack_count), no_ack_count, (ack_count), max_retry_count, null_ack_count);            
                //chprintf(chp, "ACK_Right:%d ACK_Err:%d ACK_ErrBit:%d \r\n", Right_Count, error_count, error_bit);
			}
            osDelay(50);
		}
		chprintf(chp, "ch:%d Total:%u TX_NoACK:%u TX_ACK:%u MAX_RT:%u NULL_ACK:%u ", mhz, (ack_count+no_ack_count),
                 no_ack_count, (ack_count), max_retry_count, null_ack_count);            
		chprintf(chp, "ACK_Pass:%u ACK_Fail:%u ACK_ErrorBits:%u \r\n", Right_Count, error_count, error_bit);
	}
	else { //PRX
		j=0;
     
		while(1) {

		    memset(RX_payload, 0, PKT_LENGTH); 
			MAC6200_Write_Ack_Payload(0, TX_payload, j); //write ack payload to tx fifo
	
            MAC6200_CE_High();
            if (-ETIMEDOUT == rf62_wait_rx(chp))
                break;
            /* workaround? */
            //_rf62_setup_freq(mhz - 1);
            osDelay(1);
	        MAC6200_CE_Low();
	
		    payload_Width=MAC6200_Read_Rx_Payload_Width();  //read rx payload length.
            chprintf(chp, "Rx payload in %d\r\n", payload_Width);
		    MAC6200_Read_Rx_Payload(RX_payload, payload_Width);
	
		    reg_value=0x70;
		    MAC6200_Write_Reg(MAC6200_BANK0_STATUS, &reg_value,1);   //clear STATUS register.
			osDelay(5);
		    MAC6200_Flush_Rx_Fifo();
		    MAC6200_Flush_Tx_Fifo();
	
		    for(i=0x00;i<payload_Width;i++)		 //verify payload
			{	
				if((i+1) != RX_payload[i]) {
				   error_flag = 1;
				   error_bit += bitcount((i+1)^RX_payload[i]);		//calculate sensitivity
				}
			}
	
			if(error_flag)		    //update count of packet
			  error_count++;	
			else 
			  Right_Count++;
	
			if(error_flag) {		 //dump error packet
				error_flag = 0;
				chprintf(chp, "err Pkt:");
				for(i=0; i<payload_Width; i++) 
					chprintf(chp, "%d ", (int)RX_payload[i]);
				chprintf(chp, "\r\n");
			}
			
			j++;	   //update ack payload length
			if(j>32)
				j=0;		
        }
	}
    reg_value=0x70;
    MAC6200_Write_Reg(MAC6200_BANK0_STATUS, &reg_value,1);   //clear STATUS register.
    osDelay(5);
    MAC6200_Flush_Rx_Fifo();
    MAC6200_Flush_Tx_Fifo();
    MAC6200_CE_Low();
}

static void MAC6200_RF_Test_SPL_No_ACK_Mutififo(unsigned char argu, BaseSequentialStream *chp)
{
	uint8_t  i=0;
    uint8_t reg_value;
 	int count = count_run;
    int payload_Width;
    
    if((argu & MODE_DPL)) //DPL
      return;    
    if((argu & MODE_ACK)) //ACK
      return;

    reg_value=0x00;
	MAC6200_Write_Reg(MAC6200_BANK0_EN_AA, &reg_value, 1);    //ENAA
    reg_value=PKT_LENGTH;
	MAC6200_Write_Reg(MAC6200_BANK0_RX_PW_P0, &reg_value, 1);  //Payload width 32Byte
    reg_value=0x00;
	MAC6200_Write_Reg(MAC6200_BANK0_DYNPD, &reg_value, 1);   
    reg_value=0x00;
	MAC6200_Write_Reg(MAC6200_BANK0_FEATURE, &reg_value, 1); 
	chprintf(chp, "静长静无答模式->ch:%d, Muti Fifo, 1Mbps, ", mhz);   
    MAC6200_dump_RF_register(chp);

	for(i=0;i<PKT_LENGTH*3;i++)
		RX_payload[i] = i+1;	
	for(i=0;i<PKT_LENGTH*3;i++)
		TX_payload[i] = i+1;

	if((argu&COMM_ROLE) == ROLE_PTX)    //PTX
	{
		MAC6200_CE_Low();
//		MAC6200_Flush_Tx_Fifo();
        
		while(count--)
		{

		    MAC6200_Write_Tx_Payload(TX_payload, PKT_LENGTH);	
            MAC6200_Write_Tx_Payload(TX_payload+PKT_LENGTH, PKT_LENGTH);
            MAC6200_Write_Tx_Payload(TX_payload+PKT_LENGTH*2, PKT_LENGTH);
            MAC6200_CE_High();
            delay_us(5);
            MAC6200_CE_Low();

            if (-ETIMEDOUT == rf62_wait_tx(chp))
                break;

            osDelay(2);
//            MAC6200_Read_Reg(MAC6200_BANK0_FIFO_STATUS, &reg_value, 1);
//            chprintf(chp,"reg_value = %02x %d\r\n", reg_value,  interrupt_count);

            MAC6200_CE_High();
            delay_us(5);
            MAC6200_CE_Low();

            if (-ETIMEDOUT == rf62_wait_tx(chp))
                break;

            osDelay(2);
//            MAC6200_Read_Reg(MAC6200_BANK0_FIFO_STATUS, &reg_value, 1);
//            chprintf(chp,"reg_value = %02x %d\r\n", reg_value,  interrupt_count);

            MAC6200_CE_High();
            delay_us(5);
            MAC6200_CE_Low();

            if (-ETIMEDOUT == rf62_wait_tx(chp))
                break;

            MAC6200_Flush_Rx_Fifo();
			MAC6200_Flush_Tx_Fifo();
            osDelay(20);
		}
        chprintf(chp, "\r\nch=%d: %u sent\r\n", mhz, count_run);
	}
	else { //PRX
        while(1) {
		    uint8_t temp = 0;
            count = 0;
          
            memset(RX_payload, 0, PKT_LENGTH*3);
            
            MAC6200_CE_High();
            if (-ETIMEDOUT == rf62_wait_rx(chp))
                break;

			payload_Width = MAC6200_Read_Rx_Payload_Width();
			MAC6200_Read_Rx_Payload(RX_payload, payload_Width);

			if(1 == RX_payload[0])
				temp = 1;
			else if(33 == RX_payload[0])
				temp = 33;
			else
				temp = 65;

			for(i=0x00;i<payload_Width;i++)
			{	
				if((i+temp) != RX_payload[i]) {
				   error_flag = 1;
				   error_bit += bitcount((i+1)^RX_payload[i]);	
				}
			}
	
			if(error_flag)
			  error_count++;	
			else 
			  Right_Count++;
	
			if(error_flag) {
				error_flag = 0;
				chprintf(chp, "err Pkt:");
				for(i=0; i<payload_Width; i++) 
					chprintf(chp, "%d ", (int)RX_payload[i]);
				chprintf(chp, "\r\n");
			}
		}
	}
    MAC6200_CE_Low();
}

static void rf62_power_up(bool rx_en)
{
  uint8_t config;

  MAC6200_Read_Reg(MAC6200_BANK0_CONFIG, &config, 1);
  config |= MAC6200_PWR_UP;
  if (rx_en)
    config |= MAC6200_PRIM_RX;
  else
    config &= ~MAC6200_PRIM_RX;
  MAC6200_Write_Reg(MAC6200_BANK0_CONFIG, &config, 1);
}

static void rf62_power_down(void)
{
  uint8_t config;

  MAC6200_Read_Reg(MAC6200_BANK0_CONFIG, &config, 1);
  config &= ~MAC6200_PWR_UP;
  MAC6200_Write_Reg(MAC6200_BANK0_CONFIG, &config, 1);
}

static void rf62_tx_pulse(BaseSequentialStream *chp)
{
  uint8_t i, reg_value;

  rf62_power_up(false/*rx_en*/);

  reg_value=0x00;
  MAC6200_Write_Reg(MAC6200_BANK0_EN_AA, &reg_value, 1);    //ENAA
  reg_value=PKT_LENGTH;
  MAC6200_Write_Reg(MAC6200_BANK0_RX_PW_P0, &reg_value, 1);  //Payload width 32Byte
  reg_value=0x00;
  MAC6200_Write_Reg(MAC6200_BANK0_DYNPD, &reg_value, 1);   
  reg_value=0x00;
  MAC6200_Write_Reg(MAC6200_BANK0_FEATURE, &reg_value, 1); 
  chprintf(chp, "静长静无答模式->ch:%d\r\n", mhz);   
  MAC6200_dump_RF_register(chp);

  MAC6200_CE_High();
  for (i = 0; i < count_run; i++) {
    if (interval > window)
      MAC6200_CE_High();

    _rf62_setup_freq(mhz);
    MAC6200_Write_Tx_Payload(TX_payload, PKT_LENGTH);

    if (-ETIMEDOUT == rf62_wait_tx(chp))
      break;

    osDelay(20);
    MAC6200_Flush_Rx_Fifo();
    MAC6200_Flush_Tx_Fifo();

    osDelay(window);
    if (interval > window) {
      MAC6200_CE_Low(); 
      osDelay(interval - window);
    }
  }

  MAC6200_CE_Low(); 
  /* power down after tx */
  rf62_power_down();
}

static void rf62_tx(BaseSequentialStream *chp)
{
  rf62_power_up(false/*rx_en*/);

  if (mode_dpl) {
    if (mode_ack)
      MAC6200_RF_Test_DPL_ACK(ROLE_PTX, chp);
    else
      MAC6200_RF_Test_DPL_No_ACK(ROLE_PTX, chp);
  }
  else {
    if (mode_fifo)
      MAC6200_RF_Test_SPL_No_ACK_Mutififo(ROLE_PTX, chp);
    else
      MAC6200_RF_Test_SPL_No_ACK(ROLE_PTX, chp);
  }

  /* power down after tx */
  rf62_power_down();
}

static void rf62_rx(BaseSequentialStream *chp)
{
  rf62_power_up(true/*rx_en*/);

  if (mode_dpl) {
    if (mode_ack)
      MAC6200_RF_Test_DPL_ACK(ROLE_PRX, chp);
    else
      MAC6200_RF_Test_DPL_No_ACK(ROLE_PRX, chp);
  }
  else {
    if (mode_fifo)
      MAC6200_RF_Test_SPL_No_ACK_Mutififo(ROLE_PRX, chp);
    else
      MAC6200_RF_Test_SPL_No_ACK(ROLE_PRX, chp);
  }

  /* power down after rx */
  rf62_power_down();
}

static void rf62_parse_mode(char *mode, int arg)
{
  if (strncmp(mode, "default", strlen("default")) == 0) {
    mode_dpl = false;
    mode_ack = false;
    mode_rate = MAC6200_BANK0_DR_1M;
    mode_crc = 0;
    mode_code = 0;
    mode_guard = true;
    _rf62_setup_guard(mode_guard);
    _rf62_setup_rate(mode_rate);
    _rf62_setup_code(mode_code);
    _rf62_setup_crc(mode_crc);
  }
  else if (strncmp(mode, "spl", strlen("spl")) == 0)
    mode_dpl = false;
  else if (strncmp(mode, "dpl", strlen("dpl")) == 0)
    mode_dpl = true;
  else if (strncmp(mode, "nak", strlen("nak")) == 0)
    mode_ack = false;
  else if (strncmp(mode, "ack", strlen("ack")) == 0)
    mode_ack = true;

  else if (strncmp(mode, "rate", strlen("rate")) == 0) {
    switch (arg) {
    case 250:  mode_rate = MAC6200_BANK0_DR_250K; break;
    case 500:  mode_rate = MAC6200_BANK0_DR_500K; break;
    case 1000: mode_rate = MAC6200_BANK0_DR_1M;   break;
    case 2000: mode_rate = MAC6200_BANK0_DR_2M;   break;
    default: break;
    }
    _rf62_setup_rate(mode_rate);
  }

  else if (strncmp(mode, "crc", strlen("crc")) == 0) {
    mode_crc = arg;
    _rf62_setup_crc(mode_crc);
  }

  else if (strncmp(mode, "code", strlen("code")) == 0) {
    mode_code = arg;
  /* ch_code */
#if !HS_MAC6200_USE_BTPHY
    _rf62_setup_code(mode_code);
#endif
  }

  else if (strncmp(mode, "guard", strlen("guard")) == 0) {
    mode_guard = arg ? true : false;
    _rf62_setup_guard(mode_guard);
  }

  else if (strncmp(mode, "single", strlen("single")) == 0)
    mode_fifo = false;
  else if (strncmp(mode, "mfifo", strlen("mfifo")) == 0)
    mode_fifo = true;
}

static void rf62_init(void)
{
  uint8_t reg_value = 0x00;
  uint8_t temp[5];
  uint8_t addr_5B[5] = { 0x46, 0x0b, 0xaf, 0x43, 0x98};
  //uint8_t addr_4B[4] = { 0x48, 0x54, 0x78, 0x79};
  
  HS_MAC6200->SPIRCON |= RF_MAC_SELECT;  //RF PHY Select MAC6200
 
  HS_MAC6200->SPIRCON |= (RF_IRQ_RXFIFO_FULL + RF_IRQ_RXDATA_READY   \
            +RF_IRQ_TxFIFO_EMPTY + RF_IRQ_TXFIFO_READY);  //disable all spi interrupt

#ifndef RUN_RTL_IN_SERVER 
  MAC6200_Calibration();
  MAC6200_Analog_Init();
#endif
  
  reg_value = 0x3f;           
  MAC6200_Write_Reg(MAC6200_BANK0_EN_RXADDR, &reg_value, 1);  //enable all RX pipe
  
  reg_value = 0x28;           
  MAC6200_Write_Reg(MAC6200_BANK0_SETUP_RETR, &reg_value, 1);  //disable translate auto retry

#if 1
  reg_value = MAC6200_AW_5_BYTES;
  MAC6200_Write_Reg(MAC6200_BANK0_SETUP_AW, &reg_value, 1);
  MAC6200_Write_Reg(MAC6200_BANK0_RX_ADDR_P0, addr_5B, sizeof(addr_5B));
  MAC6200_Write_Reg(MAC6200_BANK0_TX_ADDR, addr_5B, sizeof(addr_5B));
#else
  reg_value = MAC6200_AW_4_BYTES;
  MAC6200_Write_Reg(MAC6200_BANK0_SETUP_AW, &reg_value, 1);
  MAC6200_Write_Reg(MAC6200_BANK0_RX_ADDR_P0, addr_4B, sizeof(addr_4B));
  MAC6200_Write_Reg(MAC6200_BANK0_TX_ADDR, addr_4B, sizeof(addr_4B));
#endif

  MAC6200_Flush_Rx_Fifo();
  MAC6200_Flush_Tx_Fifo();
  
  reg_value = 0x70;           
  MAC6200_Write_Reg(MAC6200_BANK0_STATUS, &reg_value, 1);
  
  temp[0] = 0x3c;//+10*24/16; //RX_SETUP_VALUE: the setup time of standby to rx in 16/24us
  temp[1] = 0x4b; //TX_SETUP_VALUE: the setup time of standby to tx in 16/24us
  temp[2] = 0xff; //RX_TM_CNT:      rx timeout counter              in 16/24us
  temp[3] = 0x10; //REG_MBG_WAIT:   main band gap wait counter      in 16/24us
  temp[4] = 0x01; 
  //temp[4] = 0x6f; //REG_LNA_WAIT:   lna wait counter                in cycle @24mhz
  MAC6200_Write_Reg(MAC6200_BANK0_SETUP_VALUE, temp, 5);
  
  MAC6200_Bank1_Activate();

  temp[0] = 0x20;
  temp[1] = 0x08;
  temp[2] = 0x54;// | (1u << 7) | (1u << 3) | (1u << 2) | (1u << 1) | (1u << 0); //[23]bp_ch_chg=1 [20]scramble_en [19]bp_dc=1 [18]bp_dac=1 [17]bp_afc=1 [16]bp_rc=1
  temp[3] = 0x2b; //RX_PLL_WAIT: pll lock wait time in rx mode      in cycle @24mhz
  temp[4] = 0x78; //TX_WAIT_CNT: wait cycles between retransmits    in cycle @24mhz
  MAC6200_Write_Reg(MAC6200_BANK1_CAL_CTL, temp, 5);  

  temp[0] = 0x0f; //REG_AFC_WAIT: the time between RC done and AFC start          in 16/24us
  temp[1] = 0x28; //TX_PLL_WAIT:  pll lock wait time in tx mode                   in 16/24us
  temp[2] = 0x6f; //REG_TX_PA_WAIT: the time between power up PA to transmit data in cycle @24mhz
  temp[3] = 0x00;
  MAC6200_Write_Reg(MAC6200_BANK1_PLL_CTL1, temp, 5);  
  
  MAC6200_Bank0_Activate();

  //nvicEnableVector(IRQ_6200_RF, ANDES_PRIORITY_MASK(HS_RF_INTR_PRIORITY));
}

static void _rf_help(BaseSequentialStream *chp)
{
  chprintf(chp, "Usage: rf init\r\n");
  chprintf(chp, "       rf set [spl|dpl|nak|ack|single|mfifo\r\n");
  chprintf(chp, "       rf set [rate] [250|500|1000|2000]\r\n");
  chprintf(chp, "       rf set [crc]  [0|1|2]\r\n");
  chprintf(chp, "       rf set [code] [0|1|2]\r\n");
  chprintf(chp, "       rf set [guard] [1|0]\r\n");
  chprintf(chp, "       rf tx [mhz(dec)=2440] [count(dec)] [power(hex)={f=5dbm,8=0dbm,4=-6dbm,2=-12dbm,1=-18dbm;0=0dbm,1=6dbm,2=12dbm}] [window_ms(dec)] [interval_ms(dec)]\r\n");
  chprintf(chp, "       rf rx [mhz(dec)=2440] [timeout_ms(dec)=5000]\r\n");
  chprintf(chp, "       rf reg [bank] [addr(hex)] [high_byte(hex)] [...]\r\n");
  chprintf(chp, "Usage: rf func(hex) [mhz(dec)=2440] [timeout_ms(dec)=5000]\r\n");
  chprintf(chp, "00|01    PTX    | PRX\r\n");
  chprintf(chp, "00|04|06 NoCRC  | CRC_1B | CRC_2B\r\n");
  chprintf(chp, "00|08    2Mbps  | 1Mbps (always 1Mbps on HS66xx)\r\n");
  chprintf(chp, "00|10    single | fifo\r\n");
  chprintf(chp, "00|20    NoGard | TxGard\r\n");
  chprintf(chp, "00|40    NOACK  | ACK\r\n");
  chprintf(chp, "00|80    SPL    | DPL\r\n");
}

void cmd_rf(BaseSequentialStream *chp, int argc, char *argv[]){

  uint8_t argu;

  if (argc < 1) {
    _rf_help(chp);
    return;
  }

  argu = strtol(argv[0], NULL, 16);
  argc--;
  if (argc >= 1)
    mhz = atoi(argv[1]);

  error_flag = 0;
  error_bit = 0;
  error_count = 0;
  Right_Count = 0;

  if (strncmp(argv[0], "init", strlen("init")) == 0) {
    HS6601_mac6200_init();
    MAC6200_PHY_Enable();

    rf62_init();
    rf62_parse_mode("default", 0);
  }
  else if (strncmp(argv[0], "set", strlen("set")) == 0) {
    if (argc >= 1) {
      int arg = 0;
      if (argc >= 2)
        arg = atoi(argv[2]);
      rf62_parse_mode(argv[1], arg);
    }
    else
      rf62_parse_mode("default", 0);
    chprintf(chp, "%s, %s, Rate=%sbps, CRC=%dB, ch_code=%d, TxGuard=%s\r\n",
             mode_dpl ? "DPL" : "SPL",
             mode_ack ? "ACK" : "NAK",
             mode_rate == MAC6200_BANK0_DR_1M ? "1M" : mode_rate == MAC6200_BANK0_DR_2M ? "2M" : mode_rate == MAC6200_BANK0_DR_500K ? "500K" : mode_rate == MAC6200_BANK0_DR_250K ? "250K" : "unkown",
             mode_crc, mode_code,
             mode_guard ? "yes" : "no");
  }
  else if (strncmp(argv[0], "tx", strlen("tx")) == 0) {
    if (argc >= 2)
      count_run = atoi(argv[2]);
    if (argc >= 3)
      txgain_lvl = strtol(argv[3], NULL, 16) & 0xf;
    if (argc >= 4)
      window = atoi(argv[4]);
    if (argc >= 5)
      interval = atoi(argv[5]);
    if (interval < window)
      interval = window;

    _rf62_setup_txgain(txgain_lvl);
    _rf62_setup_freq(mhz);
    if (argc >= 4)
      rf62_tx_pulse(chp);
    else
      rf62_tx(chp);
  }
  else if (strncmp(argv[0], "rx", strlen("rx")) == 0) {
    if (argc >= 2)
      timeout = atoi(argv[2]);
    _rf62_setup_freq(mhz);
    rf62_rx(chp);
  }
  else if (strncmp(argv[0], "reg", strlen("reg")) == 0) {
    int8_t bank = 0, ii, bytes;
    uint8_t addr = 0, val[5];
    if (argc == 0) {
      MAC6200_dump_RF_register(chp);
      return;
    }
    if (argc >= 1)
      bank = atoi(argv[1]);
    if (argc >= 2)
      addr = strtol(argv[2], NULL, 16);
    bytes = m_reg_bytes[bank*0x20+addr];

    if (argc >= 3) {
      for (ii = 0; ii < bytes; ii++) {
        if (argc >= (3+ii))
          val[bytes-ii-1] = strtol(argv[3+ii], NULL, 16);
        else {
          chprintf(chp, "Warning: %d bytes are required\r\n", bytes);
          return;
        }
      }
      if (1 == bank)
        MAC6200_Bank1_Activate();
      MAC6200_Write_Reg(addr, val, bytes);
    }
    else {
      if (1 == bank)
        MAC6200_Bank1_Activate();
      MAC6200_Read_Reg(addr, val, bytes);
    }
    MAC6200_Bank0_Activate();
    chprintf(chp, "bank=%d addr=0x%02x val=", bank, addr);
    for (ii = 0; ii < bytes; ii++)
      chprintf(chp, "%02x ", val[bytes-ii-1]);
    chprintf(chp, "\r\n");
  }
  else {
    if ((argu&COM_TX_GARD) == TX_GARD_ENABLE)
      mode_guard = true;
    else
      mode_guard = false;
    _rf62_setup_guard(mode_guard);

    if ((argu & COMM_CRC) == CRC_2BYTE)
      mode_crc = 2;
    else if((argu & COMM_CRC) == CRC_1BYTE)
      mode_crc = 1;
    else
      mode_crc = 0;
    _rf62_setup_crc(mode_crc);

    _rf62_setup_rate(mode_rate);
    _rf62_setup_code(mode_code);

    if ((argu&COMM_ROLE) == ROLE_PTX)
      rf62_power_up(false/*rx_en*/);
    else
      rf62_power_up(true/*rx_en*/);

    _rf62_setup_freq(mhz);
    if((argu&COM_PL_MODE) == MODE_SPL) {   //静长
      if((argu&COM_MUTIFIFO_ENABLE)==MUTIFIFO_ENABLE)
        MAC6200_RF_Test_SPL_No_ACK_Mutififo(argu, chp);
      else
        MAC6200_RF_Test_SPL_No_ACK(argu, chp);  
    }
    else {
      if((argu&COM_MODE) == MODE_NOACK) {  //无答
        MAC6200_RF_Test_DPL_No_ACK(argu, chp);
      }
      else {
        MAC6200_RF_Test_DPL_ACK(argu, chp);
      }
    }
  }
}

#endif
