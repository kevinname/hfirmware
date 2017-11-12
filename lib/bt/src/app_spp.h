#ifndef _APP_SPP_H
#define _APP_SPP_H

/* APP SPP UI */
void App_SPP_Start(UINT8 num);
void APp_SPP_Stop(void);
void App_SPP_Connect(UINT8 *bd);
void App_SPP_Disconnect(UINT16 connection_handle);


// spp data interface
#define APP_SPP_ERR_SUCCESS      0x00
#define APP_SPP_ERR_NOT_REGISTER 0x01

typedef void (*AppSpp_Callback)(UINT8* data, UINT32 size);
UINT8 App_SPP_Register(AppSpp_Callback cb);
void  App_SPP_Unregister(void);
void  App_SPP_SendData(UINT8* data, UINT32 size);

#endif
