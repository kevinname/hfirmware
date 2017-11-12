/*---------------------------------------------------------------------------
Description: BLE Sample.
---------------------------------------------------------------------------*/
#include "app_main.h"

#ifdef CONFIG_BLE

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/

#define BLE_ADV_PARAM_LEN 15
#define BLE_ADV_DATA_LEN_MAX 31
#define BLE_SCAN_DATA_LEN_MAX 31

static UINT16  s_ble_state = APP_BLE_STATE_IDLE;
static struct HCI_AddressStru  s_ble_remote_addr;
static UINT16  s_ble_dbsize = 0;
static UINT8  *s_ble_db = NULL;
static AppBle_Callback s_ble_callback = NULL;
static UINT8 s_ble_pwm[4];

__PACKED struct AppBleConnUpdateParamStru {
   UINT16 interval_min;
   UINT16 interval_max;
   UINT16 slave_latency;
   UINT16 timeout_multiplier;
} __PACKED_GCC;
static struct AppBleConnUpdateParamStru s_ble_connupdateparam;

//static struct AppBleUuidStru s_ble_uuidlist;
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
__ROOT static void ble_HandleConnect(struct GATT_ConnectCompleteStru* buf)
{
   struct AppConnInst *connection;
   struct L2CAP_ConnectionParameterUpdateStru in;
   if (buf != NULL) {
     memcpy(&(in.addr), &(buf->addr), sizeof(struct HCI_AddressStru));
     in.interval_min = s_ble_connupdateparam.interval_min;
     in.interval_max = s_ble_connupdateparam.interval_max;
     in.slave_latency = s_ble_connupdateparam.slave_latency;
     in.timeout_multiplier = s_ble_connupdateparam.timeout_multiplier;
     //GAP_LEConnUpdateParam(&in);
     connection  = App_FindConnectionInstByBD(PROTOCOL_ATT, buf->addr.bd, APP_CONNECT_IGNORE_ROLE);

     if (buf->result == BT_SUCCESS) {
	 memcpy(&s_ble_remote_addr, &(buf->addr), sizeof(struct HCI_AddressStru));
         s_ble_state = APP_BLE_STATE_CONNECTED;

         if (connection == NULL) {
            connection = App_AddConnectionInst(PROTOCOL_ATT, buf->addr.bd, APP_CONNECT_RESPONDER);
	 }
         hsc_UpdateConnectionInst(connection, APP_CONNECT_STATE_CONNECTED);
     }
   } 
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
__ROOT static void ble_HandleDisconnect(struct GATT_ConnectCompleteStru* buf)
{
   struct AppConnInst *connection;
   if (buf != NULL) {
      connection  = App_FindConnectionInstByBD(PROTOCOL_ATT, buf->addr.bd, APP_CONNECT_IGNORE_ROLE);
      memset(&s_ble_remote_addr, 0, sizeof(struct HCI_AddressStru));
      s_ble_state = APP_BLE_STATE_IDLE;
 
      if (connection != NULL) {
         App_DeleteConnectionInst(connection);
      }
   } 
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
__ROOT static void ble_Callback(UINT16 msg, void *arg)
{
   switch(msg) {
      case GATT_EV_CONNECT_COMPLETE:
	 ble_HandleConnect((struct GATT_ConnectCompleteStru*)arg);
         break;
      case GATT_EV_DISCONNECT_COMPLETE:
         ble_HandleDisconnect((struct GATT_ConnectCompleteStru*)arg);
         break;
      default:
         break;
   }
   FREE(arg);
}

//static UINT16 ble_ReadCbk(UINT16 handle, UINT16 offset, UINT8 * buffer, UINT16 buffer_size)
//{
//	return 0;
//}

__ROOT static UINT16 ble_WriteCbk(UINT16 handle, UINT16 transaction_mode, UINT16 offset, UINT8 *buffer, UINT16 buffer_size, GATT_SignatureStru * signature)
{
  (void)transaction_mode;
  (void)signature;
	UINT8* pdata = GATT_FindDataByHandle(handle);

	if(pdata == NULL || buffer == NULL || buffer_size ==0)
		return 1;

        memcpy(pdata+offset+8, buffer, buffer_size);

	if (s_ble_callback != NULL)
	{
		UINT16 uuid = (pdata[6]|pdata[7]<<8);
		s_ble_callback(uuid, buffer, buffer_size);
	}

        // test notify
	//if (pdata[6] == 0x02 && pdata[7] == 0x29 && buffer[0] == 0x01)
	//{
	//	UINT8* pnotifydata = GATT_FindDataByHandle(handle - 1);
	//	APP_BLE_SendData(handle -1, pnotifydata+8, 16, 1);
	//}
	return 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
__ROOT static void App_BLE_FsmConnect(struct FsmInst *fi, UINT8 event, UINT8 *bd)
{
  (void)fi;
  (void)event;
   struct AppConnInst *connection = App_FindConnectionInstByBD(PROTOCOL_ATT, bd, APP_CONNECT_IGNORE_ROLE);

   if (NULL == connection) {
      //struct GATT_ConnectReqStru *in = NEW(sizeof(struct GATT_ConnectReqStru));
      //memcpy(in->bd, bd, BD_ADDR_LEN);
      //App_AddConnectionInst(PROTOCOL_ATT, bd, APP_CONNECT_INITIATOR);
      //GATT_ConnectReq(in);
   }
   FREE(bd);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
__ROOT void App_BLE_AdvStart(void)
{
#if HS_USE_CONF
   hs_cfg_res_t res = HS_CFG_OK;
   UINT8 enable = 0;
   UINT16 advtimer = 0;
   
   res = hs_cfg_getDataByIndex(HS_CFG_BLE_BRIDGE, &enable, 1);

   if (res != HS_CFG_OK)
   {
       enable = 0;
   }

   /* HS6600A4 */
   if (HS_PMU->SCRATCH1 & 0x80000000)
       enable = 0;
   if (enable >0)
   {
      hs_cfg_getPartDataByIndex(HS_CFG_BLE_BRIDGE, (UINT8*)(&advtimer), 2, 2+BLE_ADV_PARAM_LEN+BLE_ADV_DATA_LEN_MAX);
      GAP_LESetAdvSetEnable(1, advtimer);
   }
#endif
}
__ROOT void App_BLE_Start(void)
{
   UINT8 enable = 0;
   struct GATT_RegCbkStru reg;
#if HS_USE_CONF
   hs_cfg_res_t res = HS_CFG_OK;
   UINT8 param[BLE_ADV_PARAM_LEN];
   UINT8 advdata[BLE_ADV_DATA_LEN_MAX];  
   UINT8 advlen = 0;
   UINT16 advtimer = 0;
   
   res = hs_cfg_getDataByIndex(HS_CFG_BLE_BRIDGE, &enable, 1);

   if (res != HS_CFG_OK)
   {
       enable = 0;
   }

   /* HS6600A4 */
   if (HS_PMU->SCRATCH1 & 0x80000000)
       enable = 0;
   if (enable >0)
   {
      reg.cbk = ble_Callback;
      GATT_RegCbk(&reg);
      hs_cfg_getPartDataByIndex(HS_CFG_BLE_BRIDGE, param, BLE_ADV_PARAM_LEN, 1);
      GAP_LESetAdvSetParam(param);
      hs_cfg_getPartDataByIndex(HS_CFG_BLE_BRIDGE, &advlen, 1, 1+BLE_ADV_PARAM_LEN);
      hs_cfg_getPartDataByIndex(HS_CFG_BLE_BRIDGE, advdata, advlen, 2+BLE_ADV_PARAM_LEN);
      GAP_LESetAdvData(advlen, advdata);
      hs_cfg_getPartDataByIndex(HS_CFG_BLE_BRIDGE, (UINT8*)(&advtimer), 2, 2+BLE_ADV_PARAM_LEN+BLE_ADV_DATA_LEN_MAX);
      GAP_LESetAdvSetEnable(1, advtimer);
      memset(advdata, 0, BLE_ADV_DATA_LEN_MAX);
      hs_cfg_getPartDataByIndex(HS_CFG_BLE_BRIDGE, &advlen, 1, 4+BLE_ADV_PARAM_LEN+BLE_ADV_DATA_LEN_MAX);
      hs_cfg_getPartDataByIndex(HS_CFG_BLE_BRIDGE, advdata, advlen, 5+BLE_ADV_PARAM_LEN+BLE_ADV_DATA_LEN_MAX);
      GAP_LESetScanResponseData(advlen, advdata);
      hs_cfg_getPartDataByIndex(HS_CFG_BLE_BRIDGE, 
                                (UINT8*)(&s_ble_connupdateparam), 
                                sizeof(struct AppBleConnUpdateParamStru),
                                5+BLE_ADV_PARAM_LEN+BLE_ADV_DATA_LEN_MAX+BLE_SCAN_DATA_LEN_MAX);
      hs_cfg_getPartDataByIndex(HS_CFG_BLE_BRIDGE, 
                                s_ble_pwm, 
                                4,
                                5+BLE_ADV_PARAM_LEN+BLE_ADV_DATA_LEN_MAX+BLE_SCAN_DATA_LEN_MAX+sizeof(struct AppBleConnUpdateParamStru));

      res = hs_cfg_getDataByIndex(HS_CFG_BLE_DATABASE, (UINT8*)(&s_ble_dbsize), 2);
      if (res == HS_CFG_OK && s_ble_dbsize >0)
      {
              if (s_ble_db == NULL)
              {
	         s_ble_db = chHeapAlloc(NULL, s_ble_dbsize);
              }
              if (s_ble_db != NULL)
	      {
                 hs_cfg_getPartDataByIndex(HS_CFG_BLE_DATABASE, s_ble_db, s_ble_dbsize, 2);
	         GATT_SetDatabase(s_ble_db, s_ble_dbsize);
              }
      }
      //GATT_SetReadCallback(ble_ReadCbk);
      GATT_SetWriteCallback(ble_WriteCbk);
   }
#endif
   s_ble_state = APP_BLE_STATE_IDLE;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
__ROOT void App_BLE_Stop(void)
{
   struct GATT_RegCbkStru reg;
   reg.cbk = NULL;
   GATT_RegCbk(&reg);
   GAP_LESetAdvSetEnable(0,0);
   s_ble_state = APP_BLE_STATE_IDLE;
   if (s_ble_db)
     chHeapFree(s_ble_db);
   s_ble_db = NULL;
   s_ble_callback = NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
__ROOT void App_BLE_Connect(UINT8 *bd)
{
   UINT8 *in = NEW(BD_ADDR_LEN);
   memcpy(in, bd, BD_ADDR_LEN);
   App_UI_FsmEvent(App_BLE_FsmConnect, in);
   debug("app ble connect\r\n");
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_BLE_Disconnect(UINT16 handle)
{
  (void)handle;
	return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
__ROOT UINT16 App_BLE_GetState(void)
{
   return s_ble_state;
}

__ROOT UINT8 App_BLE_Register(AppBle_Callback cb)
{
	if (s_ble_callback == NULL)
	{
		s_ble_callback = cb;
		return APP_BLE_ERR_SUCCESS;
	}
	return APP_BLE_ERR_NOT_REGISTER;
}

__ROOT void  App_BLE_Unregister(void)
{
	s_ble_callback = NULL;
}

__ROOT void  APP_BLE_SendData(UINT16 uuid, UINT8* data, UINT32 size, UINT8 notify)
{
	UINT16 handle;
	if (s_ble_state == APP_BLE_STATE_CONNECTED)
	{
		handle = GATT_FindHandleByUuid(uuid);
		if(notify == 1)
		{
			GATT_Notify(s_ble_remote_addr, handle, data, size);
		}
		else
		{
			GATT_Indicate(s_ble_remote_addr, handle, data, size);
		}
	}
}

__ROOT void App_BLE_SetValue(UINT16 uuid, UINT8* data, UINT16 len)
{
	UINT16 handle;
	handle = GATT_FindHandleByUuid(uuid);
	GATT_SetDataValue(handle, data, len);
}
#endif
