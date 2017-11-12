#ifndef _SPP_UI_H
#define _SPP_UI_H

#include "stdlib.h"
#include "global.h"

typedef enum _tSpp_Event {
	/* Server Events */
	SPP_EV_SERVER_START_CFM = 1,	/* struct SppStartServerCfmStru* */

	/* Common Events */
	SPP_EV_CONNECT_COMPLETE,		/* struct SppConnectionInforStru* */
	SPP_EV_DATA_CFM,				/* struct SppDataCfmStru* */
	SPP_EV_DATA_IND,				/* struct SppDataIndStru* */
	SPP_EV_DISCONNECT_COMPLETE,		/* struct SppConnectionInforStru* */

	SPP_EV_INVALID = 0xFF,
} tSpp_Event;

typedef enum _tSpp_Connection_Role {
	SPP_ROLE_CLIENT,
	SPP_ROLE_SERVER
} tSpp_Connection_Role;

typedef enum _tSpp_Error_Code {
	SPP_ER_SUCCESS,
	SPP_ER_RFCOMM_CONNECT_FAILED,
	SPP_ER_SDAP_FAILED,
} tSpp_Error_Code;

typedef void (*tSpp_Callback)(UINT32 inst_handle, tSpp_Event event, void *param);

/* Parameter for APP_SPP_EV_SERVER_START_CFM */
struct SppStartServerCfmStru {
	UINT8  server_channel;
	UINT32 service_record_handle;
};

/* Parameter for APP_SPP_EV_CONNECT_COMPLETE */
struct SppConnectionInforStru {
	UINT32			connection_handle;
	UINT8			bd[6];
	tSpp_Error_Code		result;
	tSpp_Connection_Role	role;
};

/* Parameter for APP_SPP_EV_DATA_CFM */
struct SppDataCfmStru {
	UINT8	*data;
	UINT32	data_size;
	UINT32	sent_size;
};

/* Parameter for APP_SPP_EV_DATA_IND */
struct SppDataIndStru {
	UINT8	*data;
	UINT32	data_size;
};

/* API */
FUNC_EXPORT void SPP_Init(void);
FUNC_EXPORT void SPP_Done(void);
FUNC_EXPORT void SPP_StartServerReq(tSpp_Callback server_cbk, UINT8 max_credits, UINT8 max_connections, UINT16 max_mtu);
FUNC_EXPORT void SPP_StopServerReq(UINT32 server_handle);
FUNC_EXPORT void SPP_ConnectReq(tSpp_Callback server_cbk, UINT8 *bd, UINT8 max_credits, UINT8 server_channel);
FUNC_EXPORT void SPP_DisconnectReq(UINT32 connection_handle);
FUNC_EXPORT void SPP_DataReq(tSpp_Callback data_cfm_cbk, UINT32 connection_handle, UINT8 *data, UINT32 data_size);

#endif /* _AUDIO_TST_H */
