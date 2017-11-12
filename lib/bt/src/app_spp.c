/*---------------------------------------------------------------------------
Description: SPP Sample.
---------------------------------------------------------------------------*/
#include "app_main.h"

#ifdef CONFIG_SPP

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static UINT8  s_spp_multi_link_num = 2;
static struct ListStru s_server_hdl_list;
static struct ListStru s_connection_hdl_list;
static AppSpp_Callback s_spp_app_cb = NULL;
static UINT32 s_spp_inst_handle = 0;
static UINT16  s_spp_cur_state = APP_SPP_NOT_CONNECT;

void SppAppCbk(UINT32 inst_handle, tSpp_Event event, void *param);

void SppAppSvrStartCfm(UINT32 server_handle, struct SppStartServerCfmStru *param)
{
   UINT8 svc_name[64] = {0};
   UINT32 *item = List_NodeNew(sizeof(UINT32));

   *item = server_handle;
   List_AddTail(&s_server_hdl_list, item);
   sprintf((char*)svc_name, "Serial Port Service - Channel %d", param->server_channel);
   SDAP_ModifyStringAttrib(param->service_record_handle, svc_name, 0x100);
   debug("[SPP][StartCfm]New SPP Server: handle = %ld, channel = %d\r\n>", server_handle, param->server_channel);
}

void SppAppConnectComplete(UINT32 inst_handle, struct SppConnectionInforStru *param)
{
  (void)inst_handle;
   UINT32 *item;
   
   if (param->result == SPP_ER_SUCCESS) 
   {
      item = List_NodeNew(sizeof(UINT32));
      *item = param->connection_handle;
      List_AddTail(&s_connection_hdl_list, item);
      s_spp_inst_handle = param->connection_handle;
      s_spp_cur_state = APP_SPP_CONNECTED;
      if (param->role == SPP_ROLE_CLIENT) 
      {
	 debug("[SPP][Connect] clent connected  with %02x:%02x:%02x:%02x:%02x:%02x!\r\n>", param->bd[5], param->bd[4], param->bd[3], param->bd[2], param->bd[1], param->bd[0]);
      }
      else
      {
         debug("[SPP][Connect] server %ld is connected by %02x:%02x:%02x:%02x:%02x:%02x!\r\n>", inst_handle, param->bd[5], param->bd[4], param->bd[3], param->bd[2], param->bd[1], param->bd[0]);
      }
   }
   else
   {
      debug("[SPP][Connect] Fail to create SPP connection!\r\n>");
   }
}

void SppAppDisconnectComplete(struct SppConnectionInforStru *param)
{
   UINT32 *item;
   
   item = s_connection_hdl_list.head;
   while (item) 
   {
      if (*item == param->connection_handle) 
      {
         List_RemoveAt(&s_connection_hdl_list, item);
         LFREE(item);
         break;
      }
      item = LNEXT(item);
   }
  
   s_spp_cur_state = APP_SPP_NOT_CONNECT;
   debug("[SPP][Disconnect] with %02x:%02x:%02x:%02x:%02x:%02x is disconnected!\r\n>", param->bd[5], param->bd[4], param->bd[3], param->bd[2], param->bd[1], param->bd[0]);
}

void SppAppDataCfm(UINT32 inst_handle, struct SppDataCfmStru *param)
{
  (void)inst_handle;
  (void)param;
   s_spp_cur_state = APP_SPP_IDLE;
   debug("[SPP][SendDataCfm]req %ld bytes; send %ld bytes\r\n>", param->data_size, param->sent_size);
   //FREE(param->data);
}

void SppAppDataInd(UINT32 inst_handle, struct SppDataIndStru *param)
{
  (void)inst_handle;
    debug("[SPP][DataInd]:inst_hnadle=%ld, data size=%ld\r\n>", inst_handle, param->data_size);
    //SPP_DataReq(SppAppCbk, inst_handle, param->data, param->data_size);
    App_SetBtState2(NULL, APP_BT_STATE_CONNECTED_TRANSPORT);
    if (s_spp_app_cb != NULL)
    {
	    s_spp_app_cb(param->data, param->data_size);
    }
}

void SppAppCbk(UINT32 inst_handle, tSpp_Event event, void *param)
{
   switch (event) 
   { 
      case SPP_EV_SERVER_START_CFM:
         SppAppSvrStartCfm(inst_handle, param);
         break;
      case SPP_EV_CONNECT_COMPLETE:
         SppAppConnectComplete(inst_handle, param);
         break;
      case SPP_EV_DATA_CFM:
         SppAppDataCfm(inst_handle, param);
         break;
      case SPP_EV_DATA_IND:
         SppAppDataInd(inst_handle, param);
         break;
      case SPP_EV_DISCONNECT_COMPLETE:
         SppAppDisconnectComplete(param);
         break;
      default:
         break;
   }
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_SPP_Start(UINT8 num)
{
   debug("[SPP][App_Spp_Start]!\r\n>");

   if (num > 0)
   {
      s_spp_multi_link_num = num;
      SPP_StartServerReq(SppAppCbk, 7, s_spp_multi_link_num, g_bt_host_config.attrs.sppMaxMtu);
   }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_SPP_Stop(void)
{
   UINT32 *item;

   item = s_server_hdl_list.head;
   while (item) 
   {
      SPP_StopServerReq(*item);
      item = LNEXT(item);
   }
   debug("[SPP][App_Spp_Stop]!\r\n>");
   List_RemoveAll(&s_server_hdl_list);
   List_RemoveAll(&s_connection_hdl_list);
   s_spp_app_cb = NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_SPP_Connect(UINT8 *bd)
{
  (void)bd;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_SPP_Disconnect(UINT16 connection_handle)
{
  (void)connection_handle;
}

UINT8 App_SPP_Register(AppSpp_Callback cb)
{
	if (s_spp_app_cb == NULL)
	{
		s_spp_app_cb = cb;
		return APP_SPP_ERR_SUCCESS;
	}
	return APP_SPP_ERR_NOT_REGISTER;
}

void App_SPP_Unregister(void)
{
	s_spp_app_cb = NULL;
}

void App_SPP_SendData(UINT8* data, UINT32 size)
{
   s_spp_cur_state = APP_SPP_BUSY;
   App_SetBtState2(NULL, APP_BT_STATE_CONNECTED_TRANSPORT);
   SPP_DataReq(SppAppCbk, s_spp_inst_handle, data, size);
}

UINT16 App_SPP_GetState(void)
{
	return s_spp_cur_state;
}
#endif
