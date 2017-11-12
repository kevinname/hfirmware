#ifndef _APP_HID_H
#define _APP_HID_H

// error code
#define APP_HID_ERR_SUCCESS      0x00
#define APP_HID_ERR_NOT_REGISTER 0x01

// role
#define APP_HID_ROLE_NONE              0x00
#define APP_HID_ROLE_POINTER           0x01
#define APP_HID_ROLE_MOUSE             0x02
#define APP_HID_ROLE_JOYSTICK          0x04
#define APP_HID_ROLE_GAME_PAD          0x08
#define APP_HID_ROLE_KEYBOARD          0x10
#define APP_HID_ROLE_KEYPAD            0x20

// cod
#define APP_HID_MAJOR_DEVICE_CLASS       0x0500
#define	APP_HID_MINOR_MOUSE              0x80
#define	APP_HID_MINOR_KEYBOARD           0x40

/* APP HID UI */
void App_HID_Start(UINT8 role);
void App_HID_Stop(void);
void App_HID_Connect(UINT8 *bd);
void hsc_HID_Disconnect(void);

void App_HID_SendData(UINT8* data, UINT32 len);
void App_HID_SendDataByInterrupt(UINT8* data, UINT32 len);

#endif
