#ifndef _APP_BTLED_H
#define _APP_BTLED_H

void App_BTLED_Init(void);
void App_BTLED_Uninit(void);
void App_BTLED_SendData(UINT8* data, UINT16 size);
#endif
