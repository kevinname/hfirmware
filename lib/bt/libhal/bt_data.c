/*
   app_evnet - Copyright (C) 2012~2014 HunterSun Technologies
   zhiyuan.chen@huntersun.com.cn
   */

/**
 * @file    bt_data.c
 * @brief   handle uart at command and event for bt trans file.
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
#include "bt_config.h"

#define RX_TIMEOUT              MS2ST(1000)
#define UART_MAX_PKT_SIZE       300
#define TASK_STK_SIZE_BT_DATA   (4096+1024)

typedef struct bt_data_trans_def
{
    SerialDriver *device;
    uint8_t      *rx_buffer;
    osThreadId   pThd;
    uint32_t     status;
}BtDataTransDef_t;

typedef enum
{
    BT_DATA_TRANS_STATUS_START,
    BT_DATA_TRANS_STATUS_RX_CONFIG,
    BT_DATA_TRANS_STATUS_RX_DATA,
    BT_DATA_TRANS_STATUS_STOP
}BtDataTransStatus_e;

typedef void (*BtDataEvtFunction_t)(uint16_t len, uint8_t *arg);
typedef struct
{
    uint8_t              *pCmd;
    BtDataEvtFunction_t  pfnOper;                /*!< event or status type corresponding operation       */
}BtDataCmdOperDef_t;

static BtDataTransDef_t *g_pBtDataVar = NULL;

void _bt_host_data_uart_exception_event(void)
{
    uint8_t event[6];
    if ((!g_pBtDataVar) || g_pBtDataVar->status == BT_DATA_TRANS_STATUS_STOP)
    {
        return;
    }
    event[0] = 'F';
    event[1] = 'A';
    event[2] = 'I';
    event[3] = 'L';
    event[4] = 0x0D;
    event[5] = 0x0A;
    sdWrite(g_pBtDataVar->device, event, 6);
}

void _bt_host_data_complete_event(void)
{
    uint8_t event[4];
    if ((!g_pBtDataVar) || g_pBtDataVar->status == BT_DATA_TRANS_STATUS_STOP)
    {
        return;
    }
    event[0] = 'O';
    event[1] = 'K';
    event[2] = 0x0D;
    event[3] = 0x0A;
    sdWrite(g_pBtDataVar->device, event, 4);
}

void _bt_host_data_response(uint8_t *pdata, uint8_t len)
{
    sdWrite(g_pBtDataVar->device, pdata, len);
}

void _bt_host_data_bt_addr(uint16_t len, uint8_t *arg)
{
    bt_host_sys_config_t *p_host_config;
    uint8_t *pdata = arg;
    uint8_t res[22];
    if (len == 0 || !(pdata[0] == '?' || pdata[0] == '='))
    {
        _bt_host_data_uart_exception_event();
        return;
    }
    // ?
    if (pdata[0] == '?')
    {
        sprintf(res, "+BTADSS:%02x%02x%02x%02x%02x%02x\r\n",
                p_sys_config->bd_addr[0],
                p_sys_config->bd_addr[1], 
                p_sys_config->bd_addr[2],
                p_sys_config->bd_addr[3], 
                p_sys_config->bd_addr[4], 
                p_sys_config->bd_addr[5]);
        _bt_host_data_response(res, sizeof(res));
    }
    else // =addr
    {
        if (len-1 != BD_ADDR_LEN)
        {
            _bt_host_data_uart_exception_event();
            return;
        }
        pdata++;
        // set ram data
        bt_host_get_sys_config(&p_host_config);
        memcpy(p_host_config->addr, pdata, BD_ADDR_LEN);
        memcpy(p_sys_config->bd_addr, pdata, BD_ADDR_LEN);

        // save to flash
        hs_cfg_setDataByIndex(HS_CFG_BT_HC, (uint8_t *)p_sys_config, sizeof(bt_hc_sys_config_t), 0);
        //hs_cfg_flush(FLUSH_TYPE_ALL);
    }
    // send HCI_EVENT_CMD_COMPLETE event
    _bt_host_data_complete_event();
}

//void _bt_host_data_ble_addr(uint16_t len, uint8_t *arg)
//{
//}

void _bt_host_data_set_visibility(uint16_t len, uint8_t *arg)
{
    bt_host_sys_config_t *p_host_config;

    // set ram data
    // bit 0:spp visibility; bit 1: spp connectable; bit 2:ble adv enable;
    uint8_t param = (arg[0]<<4 | ((arg[0]>>2)<<3));
    bt_host_get_sys_config(&p_host_config);
    p_host_config->sys_padding[0] &= 0x07;
    p_host_config->sys_padding[0] |= param;
    App_CFG_SetBtHostVisibility(p_host_config->sys_padding[0]);
    //hs_cfg_flush(FLUSH_TYPE_ALL);

    bt_host_classic_scan(arg[0]&0x01);
    bt_host_ble_adv((arg[0]>>2)&0x01);

    // send HCI_EVENT_CMD_COMPLETE event
    _bt_host_data_complete_event();
}

void _bt_host_data_bt_name(uint16_t len, uint8_t *arg)
{
    bt_host_sys_config_t *p_host_config;
    uint8_t *pdata = arg;
    uint8_t res[BT_NAME_LEN_MAX+10];
    if (len == 0 || !(pdata[0] == '?' || pdata[0] == '='))
    {
        _bt_host_data_uart_exception_event();
        return;
    }
    // ?
    if (pdata[0] == '?')
    {
        sprintf(res, "+BTNAME:%s\r\n",p_host_config->name);
        _bt_host_data_response(res, sizeof(res));
    }
    else // =addr
    {
        // set ram data
        pdata++;

        bt_host_get_sys_config(&p_host_config);
        memset(p_host_config->name, 0, BT_NAME_LEN_MAX);
        if (len-1 < BT_NAME_LEN_MAX)
        {
            memcpy(p_host_config->name, pdata, len-1);
            App_CFG_SetBtHostName(len-1, pdata);
        }
        else
        {
            memcpy(p_host_config->name, pdata, BT_NAME_LEN_MAX);
            App_CFG_SetBtHostName(BT_NAME_LEN_MAX, pdata);
        }
        //hs_cfg_flush(FLUSH_TYPE_ALL);
    }
    // send HCI_EVENT_CMD_COMPLETE event
    _bt_host_data_complete_event();
}

void _bt_host_data_ble_name(uint16_t len, uint8_t *arg)
{
    bt_host_sys_config_t *p_host_config;
    uint8_t *pdata = arg;
    uint8_t res[BT_NAME_LEN_MAX+20];
    uint8_t adv_index = 0;
    uint8_t ismodify = 0;
    uint8_t name_len = len-1;
    if (len == 0 || !(pdata[0] == '?' || pdata[0] == '='))
    {
        _bt_host_data_uart_exception_event();
        return;
    }
    // ?
    if (pdata[0] == '?')
    {
        if (!(p_host_config->adv_res_en&0x03))
        {
            sprintf(res, "+BLENAME:%s\r\n",p_host_config->name);
        }
        else
        {
            if (!(p_host_config->adv_res_en&0x01))
            {
                for (adv_index =0; adv_index < LE_ADVERTISING_DATA_SIZE;)
                {
                    if (p_host_config->adv_data[adv_index+1] == 0x09)
                    {
                        ismodify = 1;
                        sprintf(res, "+BLENAME:%s\r\n", &p_host_config->adv_data[adv_index+2]);
                        break;
                    }
                    adv_index += (p_host_config->adv_data[adv_index]+1);
                }
            }
            if (ismodify == 0 && !(p_host_config->adv_res_en&0x02))
            {
                for (adv_index =0; adv_index < p_host_config->res_len; adv_index++)
                {
                    if (p_host_config->res_data[adv_index+1] == 0x09)
                    {
                        ismodify = 1;
                        sprintf(res, "+BLENAME:%s\r\n", &p_host_config->res_len[adv_index+2]);
                        break;
                    }
                    adv_index += (p_host_config->res_data[adv_index]+1);
                }
            }
        }
        _bt_host_data_response(res, sizeof(res));
    }
    else // =addr
    {
        // set ram data
        pdata++;

        name_len = name_len>BT_NAME_LEN_MAX?BT_NAME_LEN_MAX:name_len;

        bt_host_get_sys_config(&p_host_config);
        if (!(p_host_config->adv_res_en&0x03))
        {
            p_host_config->adv_res_en &= 0xFF;
            p_host_config->adv_res_en |= 0x01;
            name_len = (name_len+2)>LE_ADVERTISING_DATA_SIZE?LE_ADVERTISING_DATA_SIZE:(name_len+2);
            memset(p_host_config->adv_data, 0 , LE_ADVERTISING_DATA_SIZE);
            p_host_config->adv_data[0] = name_len-1;
            p_host_config->adv_data[1] = 0x09;
            memcpy(&p_host_config->adv_data[2], pdata, name_len-2);
            App_CFG_SetBtHostBle(p_host_config);
        }
        else
        {
            // adv_data or res_data moust have 0x09 flag
            if (p_host_config->adv_res_en&0x01)
            {
                for (adv_index =0; adv_index < LE_ADVERTISING_DATA_SIZE; adv_index++)
                {
                    if (p_host_config->adv_data[adv_index+1] == 0x09)
                    {
                        ismodify = 1;
                        name_len = (name_len+2)>(LE_ADVERTISING_DATA_SIZE-adv_index)?(LE_ADVERTISING_DATA_SIZE-adv_index):(name_len+2);
                        memset(p_host_config->adv_data+adv_index, 0 , name_len);
                        p_host_config->adv_data[adv_index] = name_len-1;
                        p_host_config->adv_data[adv_index+1] = 0x09;
                        memcpy(&p_host_config->adv_data[adv_index+2], pdata, name_len-2);
                        App_CFG_SetBtHostBle(p_host_config);
                        break;
                    }
                }
            }
            if (ismodify == 0 && p_host_config->adv_res_en&0x02)
            {
                for (adv_index =0; adv_index < LE_ADVERTISING_DATA_SIZE; adv_index++)
                {
                    if (p_host_config->res_data[adv_index+1] == 0x09)
                    {
                        ismodify = 1;
                        name_len = (name_len+2)>(LE_ADVERTISING_DATA_SIZE-adv_index)?(LE_ADVERTISING_DATA_SIZE-adv_index):(name_len+2);
                        memset(p_host_config->res_data+adv_index, 0 , name_len);
                        p_host_config->res_data[adv_index] = name_len-1;
                        p_host_config->res_data[adv_index+1] = 0x09;
                        memcpy(&p_host_config->res_data[adv_index+2], pdata, name_len-2);
                        App_CFG_SetBtHostBle(p_host_config);
                        break;
                    }
                }
            }
        }

        //hs_cfg_flush(FLUSH_TYPE_ALL);
    }
    // send HCI_EVENT_CMD_COMPLETE event
    _bt_host_data_complete_event();
}

void _bt_host_data_send_data(uint16_t len, uint8_t *arg)
{
}

void _bt_host_data_version(uint16_t len, uint8_t *arg)
{
    uint8_t res[25];
    if (len == 0 || pdata[0] != '?')
    {
        _bt_host_data_uart_exception_event();
        return;
    }
    sprintf(res, "+VERS:2017-01-09v1.000\r\n");
    _bt_host_data_response(res, sizeof(res));
    _bt_host_data_complete_event();
}

void _bt_host_data_disconnect(uint16_t len, uint8_t *arg)
{
    bd_add_t addr;
    bt_host_disconnect(addr);
    _bt_host_data_complete_event();
}

void _bt_host_data_sleep_mode(uint16_t len, uint8_t *arg)
{
    _bt_host_data_complete_event();
    // todo sleep mode
}

static const BtDataCmdOperDef_t g_stBtDataCmdOper[] =
{
    {"ADSS",     _bt_host_data_bt_addr},
    //{"BLEADSS",  _bt_host_data_ble_addr},
    {"DISCOV",   _bt_host_data_set_visibility},
    {"NAME",     _bt_host_data_bt_name},
    {"BLENAME",  _bt_host_data_ble_name},
    {"VERS",     _bt_host_data_version},
    {"DIS",      _bt_host_data_disconnect},
    {"SLEEP",    _bt_host_data_sleep_mode},
};

static void _bt_host_data_rx_data_handle(uint32_t len, uint8_t *pdata)
{
    uint8_t cmd_len;
    uint16_t index;
    uint16_t pfn_len = sizeof(g_stBtDataCmdOper)/sizeof(BtDataCmdOperDef_t);

    for (index=0; index< pfn_len; index++)
    {
        cmd_len = strlen((char*)g_stBtDataCmdOper[index].pCmd);
        if (len > cmd_len && (!memcmp(g_stBtDataCmdOper[index].pCmd, pdata, cmd_len)))
        {
            if (g_stBtDataCmdOper[index].pfnOper != NULL)
            {
                g_stBtDataCmdOper[index].pfnOper(len-cmd_len, pdata+cmd_len);
            }
            break;
        }
    }

    // send event not opcode handle
    if (index == pfn_len)
    {
        _bt_host_data_uart_exception_event();
    }
}
static void _bt_host_data_thread(void *arg)
{
    uint32_t res;
    uint32_t rx_pos;

    (void)arg;
    chRegSetThreadName("bthostdata");

    // packet type (1B)+opcode(1B)+len(1B)+payload
    while(1)
    {
        // data trans mode

        // at command mode
        rx_pos = 0;
        do {
            res = sdReadTimeout(g_pBtDataVar->device, &g_pBtDataVar->rx_buffer[rx_pos], 1, RX_TIMEOUT);
            if (res <= 0) 
            {
                osDelay(10);
            }
            if ((!g_pBtDataVar) || g_pBtDataVar->status == BT_DATA_TRANS_STATUS_STOP)
            {
                return;
            }
        } while (res <= 0);

        rx_pos = 1;
        g_pBtDataVar->status = BT_DATA_TRANS_STATUS_RX_CONFIG;
        // get the opcode
        do {
            res = sdReadTimeout(g_pBtDataVar->device, &g_pBtDataVar->rx_buffer[rx_pos], 1, RX_TIMEOUT);
            if (res <= 0) 
            {
                osDelay(10);
            }
            else
            {
                rx_pos++;
            }
            if ((!g_pBtDataVar) || g_pBtDataVar->status == BT_DATA_TRANS_STATUS_STOP)
            {
                return;
            }
            if (g_pBtDataVar->rx_buffer[rx_pos-1] == 0x0A && g_pBtDataVar->rx_buffer[rx_pos-2] == 0x0D)
            {
                break;
            }
        } while (rx_pos < UART_MAX_PKT_SIZE);

        if(rx_pos < 3 || !(g_pBtDataVar->rx_buffer[0] == 'A' && g_pBtDataVar->rx_buffer[1] == 'T' && g_pBtDataVar->rx_buffer[2] == '+'))
        {
            _bt_host_data_uart_exception_event();
            continue;
        }
        _bt_host_data_rx_data_handle(rx_pos-3, g_pBtDataVar->rx_buffer+3);
    }
}

void bt_host_data_trans_start(void)
{
    osThreadDef_t thdDef;
    if (g_pBtDataVar) return;

    g_pBtDataVar = (BtDataTransDef_t*)hs_malloc(sizeof(BtDataTransDef_t), __MT_Z_GENERAL);
    if (g_pBtDataVar == NULL) return;

    g_pBtDataVar->rx_buffer = hs_malloc(UART_MAX_PKT_SIZE, __MT_Z_GENERAL);
    if (NULL == g_pBtDataVar->rx_buffer)
    {
        hs_free(g_pBtDataVar);
        g_pBtDataVar = NULL;
        return;
    }

    g_pBtDataVar->device = &SD1;

    thdDef.pthread   = (os_pthread)_bt_host_data_thread;
    thdDef.stacksize = TASK_STK_SIZE_BT_DATA;
    thdDef.tpriority = osPriorityNormal;
    g_pBtDataVar->pThd = oshalThreadCreate(&thdDef, g_pBtDataVar);
    if (!g_pBtDataVar->pThd)
    {
        hs_free(g_pBtDataVar->rx_buffer);
        hs_free(g_pBtDataVar);
        g_pBtDataVar = NULL;
        return;
    }
    g_pBtDataVar->status = BT_DATA_TRANS_STATUS_START;
}
void bt_host_data_trans_stop(void)
{
    if (g_pBtDataVar && g_pBtDataVar->status < BT_DATA_TRANS_STATUS_STOP)
    {
        g_pBtDataVar->status = BT_DATA_TRANS_STATUS_STOP;
        // wait rx timeout
        osDelay(RX_TIMEOUT);
        // wait rx data handle
        osDelay(100);
        oshalThreadTerminate(g_pBtDataVar->pThd);
        hs_free(g_pBtDataVar->rx_buffer);
        hs_free(g_pBtDataVar);
        g_pBtDataVar = NULL;
    }
}
#endif
/** @} */
