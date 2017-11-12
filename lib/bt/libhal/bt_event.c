/*
    app_evnet - Copyright (C) 2012~2014 HunterSun Technologies
                zhiyuan.chen@huntersun.com.cn
 */

/**
 * @file    app_event.c
 * @brief   handle event file.
 * @details 
 *
 * @addtogroup  app
 * @details 
 * @{
 */
#include "lib.h"
#include "bt_config.h"

/*
 * @brief               handle the bluetooth classic  mode key event
 *                      
 * @param[in] index:    event index (obsoleted after unified bt events)
 * @param[in] parg:     funcion index to bt events
 *                      
 * @return              void
 */
void hs_bt_handle(uint16_t index, void *parg)
{
  (void)index;
#if HS_USE_BT == HS_BT_AUDIO
  uint32_t u32Func = (uint32_t)parg;
  APP_CFG_ActionFunc actionFunc;
  if( App_GetBtState() == APP_BT_STATE_NULL) return;
  
  actionFunc = App_CFG_GetAction(u32Func & 0xff);
  if(actionFunc != NULL) {
    actionFunc();
  }
#else
  (void)parg;
#endif
}

/** @} */
