/*
   app_evnet - Copyright (C) 2012~2014 HunterSun Technologies
   zhiyuan.chen@huntersun.com.cn
   */

/**
 * @file    bt_control.c
 * @brief   handle bt event file.
 * @details 
 *
 * @addtogroup  app
 * @details 
 * @{
 */
#include "lib.h"
#if HS_USE_BT == HS_BT_DATA_TRANS
#include "bluetooth.h"
#include "btstack_uapi.h"

static void bt_host_event_callback(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    (void)channel;
    switch(packet_type)
    {
        case BT_EVENT_ACL_DATA_RECV:
            hs_printf("bt recv data!\r\n");
            break;
        case BT_EVENT_ACL_DATA_SENT_DONE:
            {
                break;
            }
        case BT_EVENT_DISCONNECT:
            {
                bt_host_classic_scan(1);
                bt_host_ble_adv(1);
            }
            break;
        case BT_EVENT_CONNECT:
            {
                bt_host_sys_config_t *p_host_cfg;
                bt_host_get_sys_config(&p_host_cfg);
                //bt_cfg_write(p_host_cfg);

                bt_host_classic_scan(0);
            }
            break;
        case BT_EVENT_ACTIVATED:
            {
                hs_printf("bt start ok!\r\n");
            }
            break;
        default:
            break;
    }
}

void bt_host_control_init(void)
{
    bt_register_packet_handler(bt_host_event_callback);
}
void bt_host_control_uninit(void)
{
    bt_register_packet_handler(NULL);
}
#endif
/** @} */
