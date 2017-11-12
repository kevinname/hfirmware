#ifndef __LIB_BTSTACK__
#define __LIB_BTSTACK__

#include "stdint.h"

#define BT_HOST_VAR_MODE_AUDIO      0x01
#define BT_HOST_VAR_MODE_HID        0x02

void hs_bt_start(uint16_t u16Idx, void *parg);
void hs_bt_stop(uint16_t u16Idx, void *parg);
void hs_bt_hid_start(uint16_t u16Idx, void *parg);
void hs_bt_hid_stop(uint16_t u16Idx, void *parg);
void hs_bt_chkLowpower(void);

#endif
