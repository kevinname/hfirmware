/*
    application - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/user/.../menu.h
 * @brief   user function
 * @details 
 *
 * @addtogroup  lib user's function
 * @details 
 * @{
 */
#ifndef __LIB_MENU_H__
#define __LIB_MENU_H__

#include "lib.h"

/* 
 * define user self message, here used for mode change 
 */
#define USER_MSG_MODE_CHANGED     (HS_CFG_EVENT_USER_BEGAN + 1)
#define USER_MSG_MODE_RECORD      (USER_MSG_MODE_CHANGED + 1)


typedef enum
{
  USER_MODE_BT        = 0,   
  USER_MODE_MUSIC1    ,
  USER_MODE_MUSIC2    ,
  USER_MODE_FM        ,
  USER_MODE_AUX       ,
    
  USER_MODE_NUM       ,
  USER_MODE_REC       ,
}hs_usermode_t;

void hs_menu_init(hs_usermode_t eDefaultMode);
hs_usermode_t hs_menu_getMode(void);
void hs_menu_doKey(uint32_t val);

#endif
 /** @} */
