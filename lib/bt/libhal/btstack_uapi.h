/**
***********************************************************
* Copyright (C) 2014 All rights reserved.
*
* @file    : btstack_uapi.h
* @brief   : BT host stack user API by Huntersun
* @version : 1.0
* @date    : 2015/9/15 19:33:07
* @author  : chenzhiyuan
*
* @note    :
*
* @history :
*
***********************************************************
*/

#ifndef __BTSTACK_UAPI_H
#define __BTSTACK_UAPI_H

#include <stdint.h>
#include "bluetooth.h"

#if defined __cplusplus
extern "C" {
#endif

#define BT_DEVNAME_LEN      20
#define BT_PINCODE_LEN      4
#define BT_REMOTE_DB_MAX    4

// bt event
#define BT_EVENT_ACTIVATED                      0x00
#define BT_EVENT_DEACTIVATED                    0x01
#define BT_EVENT_CONNECT                        0x02
#define BT_EVENT_DISCONNECT                     0x03
#define BT_EVENT_PAIR_COMFIRM                   0x04
#define BT_EVENT_PASSKEY_REQ                    0x05
#define BT_EVENT_INQURIY_RESULT                 0x06
#define BT_EVENT_SCO_DATA                       0x07
#define BT_EVENT_SCO_CAN_SEND                   0x08
#define BT_EVENT_ACL_DATA_RECV                  0x09
#define BT_EVENT_ACL_DATA_SENT_DONE             0x0a

#define BT_EVENT_BLE_SCAN_RESULT                0xa0
#define BT_EVENT_BLE_MTU                        0xa1
#define BT_EVENT_BLE_DISCOVER_COMPLETE          0xa2
#define BT_EVENT_BLE_DISCOVER_SERVER_RESULT     0xa3
#define BT_EVENT_BLE_DISCOVER_CHAR_RESULT       0xa4
#define BT_EVENT_BLE_DISCOVER_CHAR_DESC_RESULT  0xa5
#define BT_EVENT_BLE_READ_VALUE                 0xa6
#define BT_EVENT_BLE_NOTIFY_VALUE               0xa7

// error code
#define BT_ERROR_OK        0x00
#define BT_ERROR_PARAM     0x01
#define BT_ERROR_LEN       0x02
#define BT_ERROR_RESOURCE  0x03

// ble type
#define BT_BLE_UUID_TYPE      1
#define BT_BLE_UUID_TYPE_128  2

// write operation
#define BT_BLE_OP_INVALID                0x00  /**< Invalid Operation. */
#define BT_BLE_OP_WRITE_REQ              0x01  /**< Write Request. -have response*/
#define BT_BLE_OP_WRITE_CMD              0x02  /**< Write Command. --no response*/
#define BT_BLE_OP_SIGN_WRITE_CMD         0x03  /**< Signed Write Command. */
#define BT_BLE_OP_PREP_WRITE_REQ         0x04  /**< Prepare Write Request. */
#define BT_BLE_OP_EXEC_WRITE_REQ         0x05  /**< Execute Write Request. */

// write flag for exec
#define BT_BLE_WRITE_FLAG_PREPARED_CANCEL 0x00
#define BT_BLE_WRITE_FLAG_PREPARED_WRITE  0x01


typedef struct {
    bd_addr_t       bd_addr;
    link_key_t      link_key;
    link_key_type_t link_key_type;
    uint8_t         valid;
} db_mem_pair_entry_t;

/**
 * @brief config data index define., every data max 4k
 */
struct bt_host_sys_config_s {
    uint8_t  addr[BD_ADDR_LEN];
    uint8_t  name[BT_DEVNAME_LEN];
    uint8_t  pincode[BT_PINCODE_LEN];
    uint8_t  cod[3];
    uint8_t  pair_list_max_entries;

    uint8_t  valid;
    uint8_t  power;
    uint8_t  io;  // IO Capabilities
    // bit 0: 0, standard scan; 1, interlaced scan
    // bit 1: 0, ssp disable; 1, ssp nable;
    // bit 2: 0, user comfirm; 1, auto accept; 
    // bit 3: 0, ble adv disenable; 1, enable; 
    // bit 4: 0, classic discoverable disnable; 1 enable;
    // bit 5: 0, classic connectable disnable; 1 enable;
    uint8_t  sys_padding[1];

    uint16_t advi;
    uint8_t  adv_res_en;
    uint8_t  adv_padding[1]; // xtal
    uint8_t  adv_data[LE_ADVERTISING_DATA_SIZE];
    uint8_t  adv_data_padding[1];
    uint8_t  res_data[LE_ADVERTISING_DATA_SIZE];
    uint8_t  res_data_padding[1];

    uint8_t  ibea;
    uint8_t  ibea_padding[3];
    uint8_t  ibea_uuid[16];
    uint16_t ibea_major;
    uint16_t ibea_minor;
    uint16_t ibea_mea;
    uint16_t ibea_rsv;

    uint8_t  batc;
    uint8_t  batt;
    uint8_t  bas_padding[2];

    db_mem_pair_entry_t pair_list[BT_REMOTE_DB_MAX];

    uint8_t  usr_specific[128];
    uint32_t count;
};

typedef struct bt_host_sys_config_s bt_host_sys_config_t;


//#define BT_HOST_INQUIRY_MAX_DEVICES 10
typedef struct bt_host_inquiry_device_s {
    bd_addr_t  address;
    uint16_t   clock_offset;
    uint32_t   cod;
    uint8_t    psrm; //pageScanRepetitionMode;
    uint8_t    rssi;
    uint8_t    extend_res_len;
    uint8_t    extend_res[240];
}bt_host_inquiry_device;

typedef struct bt_host_connect_event_s {
    bd_addr_t  address;
    uint8_t    module;
    uint8_t    state;   
}bt_host_connect_event;

typedef struct bt_host_acl_data_res_s{
    uint8_t  status; //0, send ok; 1: error
    uint16_t len; // if error; sent len;
} bt_host_acl_data_res;

typedef struct bt_ble_scan_result_event_s {
    uint8_t    event_type;
    uint8_t    addr_type;
    bd_addr_t  address;
    uint8_t    rssi;
    uint8_t    data_len;
    uint8_t    data[1];
}bt_ble_scan_result_event;


typedef struct bt_ble_uuid_s {
    uint8_t    uuid_type;
    uint8_t    uuid[16];
}bt_ble_uuid;


typedef struct bt_ble_service_s{
    uint16_t start_group_handle;
    uint16_t end_group_handle;
    uint16_t uuid16;
    uint8_t  uuid128[16];
} bt_ble_service;

typedef struct bt_ble_characteristic_s{
    uint16_t start_handle;
    uint16_t value_handle;
    uint16_t end_handle;
    uint16_t properties;
    uint16_t uuid16;
    uint8_t  uuid128[16];
} bt_ble_characteristic;

typedef struct bt_ble_read_params_s{
    uint16_t start_handle;
    uint16_t end_handle;
    bt_ble_uuid value_handle;
} bt_ble_read_params;

typedef struct bt_ble_write_params_s{
    uint8_t  operation;
    uint8_t  flags; 
    uint16_t handle;
    uint16_t offset;
    uint16_t len;
    uint8_t  value[1];
} bt_ble_write_params;

typedef struct bt_ble_read_notify_response_s{
    uint16_t value_handle;
    uint16_t value_len;
    uint8_t  *value;
} bt_ble_read_notify_resposne;


/*
 * profile specific
 */
typedef void (*bt_host_packet_handler_t) (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

bt_host_packet_handler_t bt_register_packet_handler(bt_host_packet_handler_t handler);
void bt_host_get_sys_config(bt_host_sys_config_t **pp_host_cfg);
uint8_t bt_host_start_inquiry(uint16_t ms, uint8_t flag);
uint8_t bt_host_classic_scan(uint8_t enable);
uint8_t bt_host_ble_adv(uint8_t enable);

uint8_t bt_host_connect(bd_addr_t addr);
uint8_t bt_host_disconnect(bd_addr_t addr);
uint8_t bt_host_connect_sco(bd_addr_t addr);
uint8_t bt_host_disconnect_sco(bd_addr_t addr);

uint8_t bt_host_send_acl(uint8_t * pdata, uint16_t len);
uint8_t bt_host_send_sco(uint8_t *pdata, uint8_t len);


// ble api
// server
uint8_t bt_ble_custom_service_uuid(uint8_t* old_uuid, uint8_t* new_uuid, uint8_t len); 
uint8_t bt_ble_custom_char_uuid(uint8_t* old_uuid, uint8_t* new_uuid, uint8_t len);

// client
uint8_t bt_ble_scan_enable(uint8_t en);
uint8_t bt_ble_connect(bd_addr_t addr, uint8_t addr_type);
uint8_t bt_ble_read(bt_ble_read_params *pdata);
uint8_t bt_ble_write(bt_ble_write_params* pdata);
uint8_t bt_ble_discover_primary_services(bt_ble_uuid *uuid);
uint8_t bt_ble_discover_characteristic(bt_ble_service *service, bt_ble_uuid *uuid);
uint8_t bt_ble_discover_characteristic_descriptors(bt_ble_characteristic* ch);
#if defined __cplusplus
}
#endif

#endif

