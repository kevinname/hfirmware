/*
    demo - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    main.c
 * @brief   a demo process.
 * @details 
 *
 * @addtogroup  demo
 * @details 
 * @{
 */

#include "lib.h"
#ifdef TEST_ENABLE
#include "test.h"
#endif

int main(void) 
{
  hs_boardInit(); 

  #ifdef TEST_ENABLE
  hs_test_init();  
  #endif  

  /* start the service of audio rtythm and rgb led */
  hs_led_flash();

  /* led screen init */ 
  hs_led_init();
  
  hs_menu_init(USER_MODE_BT);

  while(true) 
  {
    #ifdef TEST_ENABLE
    hs_test_chkExit();
    #endif
    
    msleep(500);
#if HS_USE_BT
    hs_bt_chkLowpower();
#endif
  }
}
