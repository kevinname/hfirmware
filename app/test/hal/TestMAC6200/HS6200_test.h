// HS6200 测试程序。
// ***************************************************/
// // 注意在 Keil 软件的项目 Target 设置中，在 C51 选项
// // 部分设置包含文件目录。


 #ifndef __HS6200_TEST_H__
 #define __HS6200_TEST_H__

extern U8 Cal_after_Ack;
extern U8 flush_tx_when_max_rt;

extern void nRF24L01_set_mode(U8 DevNum, rf_config_t NewConfig);
extern void System_Init(void);	  //设备初始化

// /**************************************************/

extern void nRF24L01_set_mode(unsigned char DevNum, rf_config_t NewConfig);  //24L01 set mode


//used for test
//Bank0下：
extern void HS6200_Rst_Bank0_All_Register(U8 DevNum);
extern U8 HS6200_Read_Bank0_All_Register(U8 DevNum,U8 *Reg_Val);
extern void HS6200_Write1_to_Bank0_All_Register(U8 DevNum);
extern void HS6200_Write0_to_Bank0_All_Register(U8 DevNum);

extern U8 HS6200_Read_Bank1_All_Register(U8 DevNum,U8 *Reg_Val);
extern void HS6200_Rst_Bank1_All_Register(U8 DevNum);
extern U8 HS6200_Read_All_Register(U8 DevNum,U8* Reg_Val);

extern void HS6200_Mode_Config(U8 DevNum,U8 Config);

extern void HS6200_DEV0_Int(void);
extern void HS6200_DEV1_Int(void);



 #endif // __HS6200_TEST_H__


/*-----------------------------------End Of File---------------------------------------*/


