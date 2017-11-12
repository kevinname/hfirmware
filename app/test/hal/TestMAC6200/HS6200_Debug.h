
/*
 *  HS6200测试工程的配置说明：
 *  1.工程的错误输出信息是可以通过宏（macro）控制是否输出，HS6200_DEBUG macro就是起到控制错误信息是否输出
 *	2.DISP_OUT_ERR macro 控制错误输出的方式，一种通过LED输出，一种通过USB的输出。
 *  3.HS6200_DEVx 控制DEV_x是HS6200还是nRF24L01,
 */


#ifndef HS6200_DEBUG_H
#define HS6200_DEBUG_H

#include "nRF24L01_X.h"

extern void LED_Out_Err_Info(U8 Err_Info);
extern void USB_Out_Err_Info(U8 DevNum,U8 Err_Info); 
extern void USB_ACK_Diagnose_Info(U8 DevNum);
 

//global config
 
//output error number by LED，更改时，同时需对HS6200_test_sys.c文件下的Port_IO_Init()函数进行修改
#define DISP_OUT_ERR	   P4    
#define LED_OUT_ERR        0x01
#define USB_OUT_ERR        0x10



#define HS6200_DEBUG			 //控制是否输出错误信息
#define DEBUG_OUT_ERR   USB_OUT_ERR   	 //控制通过何种方式输出错误信息

//软件Debug配置
#define DBG_ON       0x01
#define DBG_OFF      0x00 





/*-------------------------------------------------Err Information Macro----------------------------------------------------------*/
#define ERR_USB_PACKET_FUN_NOT_FINSH         0xFF	 //表示该功能没有实现


//define err infomation
#define ERR_USB_PACKET_PID				     0x01

// Communication err
//#define ERR_USB_PACKET_COMM				           0x02
#define ERR_USB_PACKET_COMM_CMD              0x10 
#define ERR_USB_PACKET_COMM_RF_DEV           0x11
#define ERR_USB_PACKET_COMM_LENGTH           0x12
#define ERR_USB_PACKET_COMM_PIPE             0x13
#define ERR_USB_PACKET_COMM_DATA             0x14

//setup err
#define ERR_USB_PACKET_SETUP_CMD             0x20              /*LSBit may be 0*/
#define ERR_USB_PACKET_SETUP_RF_DEV          0x21
#define ERR_USB_PACKET_SETUP_LENGTH          0x22
#define ERR_USB_PACKET_SETUP_DATA            0x23

//SPI err
#define ERR_USB_PACKET_SPI_CMD               0x30
#define ERR_USB_PACKET_SPI_RF_DEV            0x31
#define ERR_USB_PACKET_SPI_LENGTH            0x32
#define ERR_USB_PACKET_SPI_DATA              0x33

//spec err
#define ERR_USB_PACKET_SPEC_CMD              0x40
#define ERR_USB_PACKET_SPEC_RF_DEV           0x41
#define ERR_USB_PACKET_SPEC_LENGTH           0x42
#define ERR_USB_PACKET_SPEC_DATA             0x43  

//ditgital test err
#define ERR_USB_PACKET_DT_CMD                0x90
#define ERR_USB_PACKET_DT_RF_DEV             0x91
#define ERR_USB_PACKET_DT_LENGTH             0x92
#define ERR_USB_PACKET_DT_DATA               0x93
   
//analog test err
#define ERR_USB_PACKET_AT_CMD                0xA0
#define ERR_USB_PACKET_AT_RF_DEV             0xA1
#define ERR_USB_PACKET_AT_LENGTH             0xA2
#define ERR_USB_PACKET_AT_DATA               0xA3



//set err
#define ERR_USB_PACKET_SET_CMD               0xC0
#define ERR_USB_PACKET_SET_RF_DEV            0xC1
#define ERR_USB_PACKET_SET_LENGTH            0xC2
#define ERR_USB_PACKET_SET_DATA              0xC3

//get err
#define ERR_USB_PACKET_GET_CMD               0xD0
#define ERR_USB_PACKET_GET_RF_DEV            0xD1
#define ERR_USB_PACKET_GET_LENGTH            0xD2
#define ERR_USB_PACKET_GET_DATA              0xD3

//debug err
#define ERR_USB_PACKET_DBG_CMD               0xE0
#define ERR_USB_PACKET_DBG_RF_DEV            0xE1
#define ERR_USB_PACKET_DBG_LENGTH            0xE2
#define ERR_USB_PACKET_DBG_DATA              0xE3


//#define ERR_NRF24L01_0_PLOS                  0x26
//#define ERR_NRF24L01_1_PLOS                  0x27  
//#define ERR_NRF24L01_0_RX_PAYLOAD_WIDTH      0x28
//#define ERR_NRF24L01_0_RX_PAYLOAD_CONTEXT    0x29
//#define ERR_NRF24L01_1_RX_PAYLOAD_WIDTH      0x2A
//#define ERR_NRF24L01_1_RX_PAYLOAD_CONTEXT    0x2B 

//--------------------------控制输出信息-----------------------------------
extern U8 SoftWare_Date[6];  //日期
extern U8 SoftWare_Ver;      //版本号
extern U8 Debug_ce_High_Low_Stop_Flag;

#define DEBUG_SOFTWARE_DATE                    0x01    //只读
#define DEBUG_SOFTWARE_VER                     0x02    //只读 
#define DEBUG_DEVICE_FLG                       0x03    //可手动设置Dev_Flag
#define DEBUG_MCU_INT                          0x04    //只读，可通过上位机软 件修改其值
#define DEBUG_DEV_CE                           0x05    //只读，可通过上位机软 件修改其值
#define DEBUG_CAL_AFTER_ACK                    0x06    //只读，可通过上位机软 件修改其值
#define DEBUG_CE_LOW_BEFORE_WRITE              0x07    //只读，可通过上位机软 件修改其值
#define DEBUG_DEV_BUSY                         0x08    //nRF24L01_X_Busy[]可设置
#define DEBUG_CE_STATE_BEFORE_FLUSH            0x09

#define DEBUG_READ_RX_PAYLOAD                  0x0A 
#define DEBUG_READ_ADDR                        0x0B
#define DEBUG_CE_HIGH_LOW_START                0x0C
#define DEBUG_CE_HIGH_LOW_STOP                 0x0D


#define DEBUG_TEST_MODE_OTHER_SPI_WRITE        0xA0    //用于测试模式              

#endif  /*HS6200_DEBUG_H*/

/*---------------------------------End Of File----------------------------------------*/
