<...........................
我这里使用的是PA6  Timer1
............................>



1、lib\include\halconf.h     HAL_USE_IR   ： 若要使用红外遥控功能请在halconf.h 文件中似的 HAL_USE_IR 为TRUE

2、lib\include\config\config.h       ： 添加消息类型

                      #if  HAL_USE_IR
  				HS_CFG_EVENT_IR_INFO                              = 0x107,
 			 	HS_CFG_EVENT_MODE_CHANGE                          = 0x108,
		      #endif

3、os\hal\board\HUNTERSUN_HS6601\ ... board.h     HS_IR_INPUT   :  控制IR信号引脚

4、在main函数中添加 
#if HAL_USE_IR
#include"lib_infrared.h"
#endif

------

#if HAL_USE_IR
  hs_infrared_init();
#endif

5、在app\test\cmd.c中添加

#if HAL_USE_IR
extern void cmd_nec(BaseSequentialStream *chp, int argc, char *argv[]);
#endif


6、lib\include\mcuconf.h

#define HS_PWM_USE_TIM0           TRUE
#define HS_PWM_USE_TIM1           TRUE
#define HS_ICU_USE_TIM2           TRUE
     
     ICU Driver1  : Timer1  PA6 PA7(channel_1/2)
     ICU Driver2  : Timer2  PB5 PB4(channel_1/2)


7、配置事项：

修改IR输入管脚 必须与 所使用的 ICU_Timer相对应   

8、 用户只需在infrared_key_info.h　　infrared_interface.c　做实现自主功能的修改　　　　