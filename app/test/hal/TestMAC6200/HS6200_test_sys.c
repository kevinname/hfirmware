
#include "HS6200_test_sys.h"
#include "hal.h"

U8 Dev_Flag[2]={	  
                DEV_NONE,
                DEV_NONE
                };


void LED_Disp_Err(void)   //使用LED输出错误信息
{	
  U8 i=0x00;
  for(i=0x00;i<0x08;i++)
  {
    LED1_ON;
    LED2_OFF;
    chThdSleepMilliseconds(100);
    LED1_OFF;
    LED2_ON;
    chThdSleepMilliseconds(100);
  }
} 
                        
void Dev_Scan(void)
{
  Dev_Flag[0]=HS6200_DEV;
  Dev_Flag[1]=HS6200_DEV;
}

/*-----------------------------End Of File---------------------------*/

