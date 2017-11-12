#ifndef _APP_BLE_H
#define _APP_BLE_H

// error code
#define APP_BLE_ERR_SUCCESS      0x00
#define APP_BLE_ERR_NOT_REGISTER 0x01

// state
#define APP_BLE_STATE_IDLE             0x0001
#define APP_BLE_STATE_CONNECTED        0x0002
#define APP_BLE_STATE_BUSY             0x0004
/*
__PACKED struct AppBleUuidStru {
	UINT16 pwm_start;
	UINT16 pwm_end;
	UINT16 io_start;
	UINT16 io_end;
	UINT16 adc_start;
	UINT16 adc_end;
} __PACKED_GCC;
*/
/* APP BLE UI */
void App_BLE_Start(void);
void App_BLE_Stop(void);
void App_BLE_AdvStart(void);
void App_BLE_Connect(UINT8 *bd);
void App_BLE_Disconnect(UINT16 handle);

UINT16 App_BLE_GetState(void);

typedef void (*AppBle_Callback)(UINT16 uuid, UINT8* data, UINT32 size);
UINT8 App_BLE_Register(AppBle_Callback cb);
void  App_BLE_Unregister(void);
void  APP_BLE_SendData(UINT16 uuid, UINT8* data, UINT32 size, UINT8 notify);
void  App_BLE_GetPwm(UINT8 *pwm);
void App_BLE_SetValue(UINT16 uuid, UINT8* data, UINT16 len);

void App_BTLED_Init(void);

#endif
