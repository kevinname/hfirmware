#ifndef _APP_MAIN_H
#define _APP_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "bt_config.h" //hs_cfg_xxx()
#include "bt_os.h"     //debug()

/*
 * ivt related as below
 */

#include "global.h"
#include "hci_ui.h"
#include "btinit.h"

//#include "app_tl.h"
#define HCI_TLPKT_COMMAND          				0x01
#define HCI_TLPKT_ACLDATA          				0x02
#define HCI_TLPKT_SCODATA          				0x03
#define HCI_TLPKT_EVENT            				0x04

#include "bthost_uapi.h"
#include "app_fsm.h"
#include "app_gap.h"

#ifdef CONFIG_HFP
#include "app_hfp.h"
#else
#define BT_HFP_CALL_STATUS_STANDBY     0x0001
#define BT_HFP_CALL_STATUS_CONNECT     0x0002
#endif

#ifdef CONFIG_A2DP
#include "app_a2dp.h"
#endif

#ifdef CONFIG_AVRCP
#include "app_avrcp.h"
#endif

#ifdef CONFIG_SPP
#include "app_spp.h"
#endif

#ifdef CONFIG_HID
#include "app_hid.h"
#endif

#ifdef CONFIG_BLE
#include "app_ble.h"
#endif

#endif /* _APP_MAIN_H */
