/*---------------------------------------------------------------------------
Description: HID Sample.
---------------------------------------------------------------------------*/
#include "app_main.h"

#ifdef CONFIG_HID

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static const UINT8 s_hid_description[] = {
0x05, 0x01,     // USAGE_PAGE (Generic Desktop)
0x09, 0x06,     // USAGE (Keyboard)
0xa1, 0x01,     // COLLECTION (Application)
0x85, 0x01,        // report id (1)
0x75, 0x01,        // report size (1)
0x95, 0x08,        // report count (8)
0x05, 0x07,        //   USAGE_PAGE (Keyboard)
0x19, 0xe0,        //   USAGE_MINIMUM (Keyboard LeftControl)
0x29, 0xe7,        //   USAGE_MAXIMUM (Keyboard Right GUI)
0x15, 0x00,        //   LOGICAL_MINIMUM (0)
0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
0x81, 0x02,        //   INPUT (Data,Var,Abs)
0x95, 0x01,        //   REPORT_COUNT (1)
0x75, 0x08,        //   REPORT_SIZE (8)
0x81, 0x03,        //   INPUT (Cnst,Var,Abs)
0x95, 0x05,        //   REPORT_COUNT (5)
0x75, 0x01,        //   REPORT_SIZE (1)
0x05, 0x08,        //   USAGE_PAGE (LEDs)
0x19, 0x01,        //   USAGE_MINIMUM (Num Lock)
0x29, 0x05,        //   USAGE_MAXIMUM (Scroll Lock)
0x91, 0x02,        //   OUTPUT (Data,Var,Abs)
0x95, 0x01,        //   REPORT_COUNT (1)
0x75, 0x03,        //   REPORT_SIZE (3)
0x91, 0x03,        //   OUTPUT (Cnst,Var,Abs)
0x95, 0x06,        //   REPORT_COUNT (6)
0x75, 0x08,        //   REPORT_SIZE (8)
0x15, 0x00,        //   LOGICAL_MINIMUM (0)
0x26, 0xFF, 0x00,  //   LOGICAL_MAXIMUM (255)
0x05, 0x07,        //   USAGE_PAGE (Keyboard)
0x19, 0x00,        //   USAGE_MINIMUM (Reserved (no event indicated))
0x29, 0xFF,        //   USAGE_MAXIMUM (Keyboard Application)
0x81, 0x00,        //   INPUT (Data,Ary,Abs)
0xc0,              // END_COLLECTION
0x05, 0x0C,        // USAGE_PAGE (Consumer)
0x09, 0x01,        // USAGE (consumer control)
0xa1, 0x01,        // COLLECTION (Application)
0x85, 0x02,        // report id (2)
0x15, 0x00,        //   LOGICAL_MINIMUM (0)
0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
0x75, 0x01,        // report size (1)
0x95, 0x15,        // report count (21)
0x0A, 0x94, 0x01,  //   USAGE_PAGE (AL Local Machine Brower)
0x0a, 0x92, 0x01,  // usage al calculator
0x0a, 0x83, 0x01,  // usage al consumer control configuration
0x0a, 0x23, 0x02,  // usage al home
0x0a, 0x8a, 0x01,  // usage al email reader
0x0a, 0xb1, 0x01,  // usage al screen saver
0x0a, 0x21, 0x02,  // usage ac search
0x0a, 0x24, 0x02,  // usage ac back
0x0a, 0x25, 0x02,  // usage ac forward
0x0a, 0x2a, 0x02,  // usage ac bookmarks
0x0a, 0x27, 0x02,  // usage ac refresh
0x09, 0xb6,        // usage scan previous track
0x09, 0xB5,        // usage scan next track
0x09, 0x40,        // usage menu
0x09, 0xB0,        // usage play
0x09, 0xE9,        // usage vol increment
0x09, 0xEA,        // usage vol decrement
0x09, 0xE2,        // usage mute
0x09, 0xCD,        // usage playpause
0x09, 0x30,        // usage power
0x09, 0xB8,        // usage eject
0x81, 0x02,        // input (data, value, absolute, bit field)
0x95, 0x01,        // report count (1) 
0x75, 0x03,        // report size (3)
0x81, 0x03,        // input (constant, value, abs, bit fiele)
0xC0,              // END_COLLECTION
0x05, 0x0C,        // usage page (consumer)
0x09, 0x01,        // usage (consumer control)
0xA1, 0x01,        // collection(application)
0x85, 0x05,        // report id (5)
0x05, 0x01,        // usage page (generic desktop)
0x09, 0x06,        //  usage (keyboard)
0xA1, 0x02,        // collection (logical)
0x06, 0x00, 0xFF,  // usage page (vendor-defined 0xff00)
0x25, 0x01,        // logical maximum (1)
0x75, 0x01,        // report size (1)
0x95, 0x02,        // report count (2)
0x0A, 0x03, 0xFE,  // usage (vendor-define 0xfe03)
0x0A, 0x04, 0xFE,  // usage (vendor-defined 0xfe04)
0x81, 0x02,        // input (data, value ,abs ,bitfield)
0x95, 0x06,        // report count (6)
0x81, 0x03,        // input
0xC0,              // end collection
0xC0               // end collection
};

static const UINT8 s_hid_service_name[] = {'H', 'S', '6', '6', '0', '0', '-','H', 'I', 'D'};
static const UINT8 s_hid_service_desc[] = {'H', 'S', '6', '6', '0', '0', '-','K', 'e', 'y', 'b', 'o', 'a', 'r', 'd'};
static const UINT8 s_hid_provider_name[] = {'H', 'u', 'n', 't', 'e', 'r', 'S', 'u', 'n'};

static UINT8   s_hid_role = APP_HID_ROLE_NONE;
static UINT16  s_hid_state = APP_HID_STATE_IDLE;
static UINT32  s_hid_sdp_handle = 0;
static UINT8   s_hid_remote_addr[BD_ADDR_LEN];
static UINT8   s_hid_remote_mtu = 0;
static UINT8   s_hid_report_id = 0;
static UINT16  s_hid_mask = 0;
static UINT8  *s_hid_desc = NULL;
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static void hid_HandleData(struct HID_DataStru *buf)
{
   UINT8 i = 0;
   debug("hid data in, type=%d, len=%d\r\n", buf->report_type, buf->len);
   for (i =0; i< buf->len; i++) {
      debug("%x ", buf->data[i]);
   }
   debug("--end\r\n");
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static void hid_HandleConnect(struct HID_ConnectCompleteStru* buf)
{
   struct AppConnInst *connection;

   if (buf != NULL) {
    
     connection  = App_FindConnectionInstByBD(CLS_HID, buf->bd_addr, APP_CONNECT_IGNORE_ROLE);

     if (buf->result == BT_SUCCESS) {
         memcpy(s_hid_remote_addr, buf->bd_addr, BD_ADDR_LEN);
         s_hid_remote_mtu = buf->mtu;
         s_hid_state = APP_HID_STATE_CONNECTED;

         if (connection == NULL) {
            connection = App_AddConnectionInst(CLS_HID, buf->bd_addr, APP_CONNECT_RESPONDER);
	 }
         hsc_UpdateConnectionInst(connection, APP_CONNECT_STATE_CONNECTED);
     }
   } 
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static void hid_HandleDisconnect(struct HID_ConnectCompleteStru* buf)
{
   struct AppConnInst *connection;
   if (buf != NULL) {
      connection  = App_FindConnectionInstByBD(CLS_HID, buf->bd_addr, APP_CONNECT_IGNORE_ROLE);
      memset(s_hid_remote_addr, 0, BD_ADDR_LEN);
      s_hid_remote_mtu = 0;
      s_hid_state = APP_HID_STATE_IDLE;
 
      if (connection != NULL) {
         App_DeleteConnectionInst(connection);
      }
   } 
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static void hid_Callback(UINT16 msg, void *arg)
{
   switch(msg) {
      case HID_EV_CONNECT_COMPLETE:
	 debug("hid connect\r\n");
         hid_HandleConnect((struct HID_ConnectCompleteStru*)arg);
         break;
      case HID_EV_DISCONNECT_COMPLETE:
         debug("hid disconnect\r\n");
         hid_HandleDisconnect((struct HID_ConnectCompleteStru*)arg);
         break;
      case HID_EV_DATA_IND:
         hid_HandleData((struct HID_DataStru*)(arg));
         break;
      case HID_EV_DATA_CFM:
         debug("hid data cfm\r\n");
         break;
      default:
         break;
   }
   FREE(arg);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_HID_FsmConnect(struct FsmInst *fi, UINT8 event, UINT8 *bd)
{
  (void)fi;
  (void)event;
   struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_HID, bd, APP_CONNECT_IGNORE_ROLE);
   struct HID_ConnectReqStru *in;

   if (NULL == connection) {
      in = NEW(sizeof(struct HID_ConnectReqStru));
      memcpy(in->bd, bd, BD_ADDR_LEN);
      App_AddConnectionInst(CLS_HID, bd, APP_CONNECT_INITIATOR);
      HID_Connect_Req(in);
   }
   FREE(bd);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static void hid_readCfg(struct HID_AttrStru *hid_attr)
{
#if HS_USE_CONF
   hs_cfg_res_t res;
   struct AppBtHidAttrStru hid;

   res = hs_cfg_getDataByIndex(HS_CFG_CLASSIC_HID, (uint8_t *)&hid, sizeof(hid));

   if (res == HS_CFG_OK) {
      hid_attr->release_num = hid.releaseNum;
      hid_attr->parser_version = hid.parserVersion;
      hid_attr->sub_class = hid.subClass;
      hid_attr->country_code = hid.countryCode;
      hid_attr->profile_version = hid.profileVersion;
      hid_attr->supervision_timeout = hid.supervisionTimeout;
      hid_attr->max_latency = hid.maxLatency;
      hid_attr->min_timeout = hid.minTimeout;
      hid_attr->name_length = hid.nameLen;
      hid_attr->service_name =  hid.serviceName;
      hid_attr->name_desc_length = hid.nameDescLen;
      hid_attr->name_desc = hid.nameDesc;
      hid_attr->pro_length = hid.proNameLen;
      hid_attr->pro_name = hid.proName;

      s_hid_desc = NEW(hid.descLen);
      res = hs_cfg_getPartDataByIndex(HS_CFG_CLASSIC_HID, s_hid_desc, hid.descLen, sizeof(hid));
      if (res != HS_CFG_OK) {
         hid_attr->desc_length = sizeof(s_hid_description);
         hid_attr->descriptor = (UINT8*)s_hid_description;
      }
      else {
         hid_attr->desc_length =  hid.descLen;
         hid_attr->descriptor = s_hid_desc;
      }

      s_hid_mask = hid.mask;
      
      s_hid_report_id = hid.reportId;
   } else
#endif
   {
      hid_attr->release_num = 0x0100;
      hid_attr->parser_version = 0x0111;
      hid_attr->sub_class = APP_HID_MINOR_KEYBOARD;
      hid_attr->country_code = 0x21;
      hid_attr->profile_version = 0x0100;
      hid_attr->supervision_timeout = 0x1f40;
      hid_attr->max_latency = 0x0318;
      hid_attr->min_timeout = 0x0012;
      hid_attr->desc_length = sizeof(s_hid_description);
      hid_attr->descriptor = (UINT8*)s_hid_description;
      hid_attr->name_length = sizeof(s_hid_service_name);
      hid_attr->service_name = (UINT8 *)s_hid_service_name;
      hid_attr->name_desc_length = sizeof(s_hid_service_desc);
      hid_attr->name_desc = (UINT8 *)s_hid_service_desc;
      hid_attr->pro_length = sizeof(s_hid_provider_name);
      hid_attr->pro_name = (UINT8 *)s_hid_provider_name;
      
      s_hid_mask = HID_MASK_VCABLE_TRUE| HID_MASK_RECONNINIT_TRUE 
           | OPTATTR_MASK_SUPERTO | OPTATTR_MASK_BATTERYPWR
           | OPTATTR_MASK_RMWAKE | OPTATTR_MASK_NORMCONN
           | HID_MASK_BOOTDEVICE_TRUE;
      s_hid_report_id = HID_REPORT_ID_KEYBOARD;
   }
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_HID_Start(UINT8 role)
{
   struct HID_RegCbkStru reg;
   struct HID_AttrStru hid_attr;

   debug("[HID][Start][%d]\r\n", role);

   s_hid_role = role;
   if (s_hid_role > 0)
   {
      reg.cbk = hid_Callback;
      HID_RegCbk(&reg);
      
      hid_readCfg(&hid_attr);

      s_hid_sdp_handle =  HID_RegisterService(s_hid_report_id, s_hid_mask, &hid_attr);

      if (s_hid_desc != NULL) {
         FREE(s_hid_desc);
         s_hid_desc = NULL;
      }
   }
   s_hid_state = APP_HID_STATE_IDLE;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_HID_Stop(void)
{
   struct HID_RegCbkStru reg;
   reg.cbk = NULL;
   HID_RegCbk(&reg);
   HID_UnregisterService(s_hid_sdp_handle);
   
   s_hid_sdp_handle = 0;
   s_hid_state = APP_HID_STATE_IDLE;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_HID_Connect(UINT8 *bd)
{
   UINT8 *in = NEW(BD_ADDR_LEN);
   memcpy(in, bd, BD_ADDR_LEN);
   App_UI_FsmEvent(App_HID_FsmConnect, in);
   debug("app hid connect\r\n");
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void hsc_HID_Disconnect(void)
{
   HID_Disconnect_Req();
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
UINT16 App_HID_GetState(void)
{
   return s_hid_state;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_HID_SendData(UINT8* data, UINT32 len)
{
   struct HID_DataStru* param = NEW(sizeof(struct HID_DataStru));

   if (param == NULL)
      return;

   param->report_type = HID_REPORT_TYPE_INPUT;
   memcpy(param->bd_addr, s_hid_remote_addr, BD_ADDR_LEN);
   memcpy(param->data, data, len);
   param->len = len;
   HID_SendDataByControl(param);
   //debug("[HID] send data\r\n");
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_HID_SendDataByInterrupt(UINT8* data, UINT32 len)
{
   struct HID_DataStru* param = NEW(sizeof(struct HID_DataStru));

   if (param == NULL)
      return;

   param->report_type = HID_REPORT_TYPE_INPUT;
   param->len = len;
   memcpy(param->data, data, len);
   memcpy(param->bd_addr, s_hid_remote_addr, BD_ADDR_LEN);

   HID_SendDataByInterrupt(param);
   //debug("[HID] send data from interrupt\r\n");
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_HID_SendKey1(void)
{
   UINT8 data[9] = {0x01, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00};
   App_HID_SendDataByInterrupt(data, 9);
   msleep(1);
   data[3] = 0x00;
   App_HID_SendDataByInterrupt(data, 9);
   msleep(1);
   
   //hs_printf("[HID] send camera key\r\n");
#if HS_USE_CONF
   hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_HID_KEY);
#endif
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_HID_SendKey2(void)
{
   UINT8 data[5] = {0x02, 0x00, 0x80, 0x00, 0x00};
   App_HID_SendDataByInterrupt(data, 5);
   msleep(1);
   memset(data, 0, 5);
   data[0] = 0x02;
   App_HID_SendDataByInterrupt(data, 5);
   msleep(1);
   //hs_printf("[HID] send camera mouse\r\n");
#if HS_USE_CONF
   hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_HID_KEY);
#endif
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/

void App_HID_SendKey3(void)
{
   UINT8 data[5] = {0x02, 0x00, 0x80, 0x00, 0x00};
   App_HID_SendDataByInterrupt(data, 5);
   memset(data, 0, 5);
   data[0] = 0x02;
   App_HID_SendDataByInterrupt(data, 5);
   //debug("[HID] send camera mouse\r\n");
#if HS_USE_CONF
   hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_HID_KEY);
#endif
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/

void App_HID_SendKey4(void)
{
   UINT8 data[5] = {0x02, 0x00, 0x80, 0x00, 0x00};
   App_HID_SendDataByInterrupt(data, 5);
   memset(data, 0, 5);
   data[0] = 0x02;
   App_HID_SendDataByInterrupt(data, 5);
   //debug("[HID] send camera mouse\r\n");
#if HS_USE_CONF
   hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_HID_KEY);
#endif
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/

#endif
