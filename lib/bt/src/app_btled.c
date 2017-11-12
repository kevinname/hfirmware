/*---------------------------------------------------------------------------
Description: BLE Sample.
---------------------------------------------------------------------------*/
#include "app_main.h"
#include "app_btled.h"
#include <time.h>

#ifdef CONFIG_BLE

#if HAL_USE_RTC
extern void rtcGetTimeTm(RTCDriver *rtcp, struct tm *timp);
extern void rtcSetTimeTm(RTCDriver *rtcp, struct tm *timp);
#endif

typedef void (APP_BTLED_CbkTableFunc) (UINT8* data, UINT32 size);

__PACKED struct APP_BTLED_HandleStru {
	UINT16 uuid;
	void *func;
} __PACKED_GCC;

#if defined(_CHIBIOS_RT_)
static virtual_timer_t s_btled_systmr;
#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static void App_BTLED_HandleFFB1(UINT8* data, UINT32 size)
{
  (void)data;
  (void)size;
}

static void App_BTLED_HandleFFB2(UINT8* data, UINT32 size)
{
  (void)data;
  (void)size;
}

static void App_BTLED_FFB3Timer(void *arg)
{
	(void)arg;
	UINT8 data[7];
#if HAL_USE_RTC
	RTCDriver rtcp;
	struct tm timp;
	rtcGetTimeTm(&rtcp, &timp);
	
	data[0] = (UINT8)(timp.tm_year);
	data[1] = (UINT8)(timp.tm_year>>8);
	data[2] = timp.tm_mon;
	data[3] = timp.tm_mday;
	data[4] = timp.tm_hour;
	data[5] = timp.tm_min;
	data[6] = timp.tm_sec;
#else
	memset(data, 0x00, 7);
#endif
	App_BLE_SetValue(0xFFB3, data, 7);

#if defined(_CHIBIOS_RT_)
	chSysLockFromISR();
	chVTSetI(&s_btled_systmr, MS2ST(1000), App_BTLED_FFB3Timer, NULL);
	chSysUnlockFromISR();
#endif
}

static void App_BTLED_HandleFFB3(UINT8* data, UINT32 size)
{
  (void)size;
#if HAL_USE_RTC
	RTCDriver rtcp;
	struct tm tmp;
	tmp.tm_year = (data[0] | data[1]<<8);
	tmp.tm_mon = data[2];
	tmp.tm_mday = data[3];
	tmp.tm_hour = data[4];
	tmp.tm_min = data[5];
	tmp.tm_sec = data[6];
	rtcSetTimeTm(&rtcp, &tmp); 
#else
  (void)data;
#endif

#if defined(_CHIBIOS_RT_)
	chSysLockFromISR();
	chVTSetI(&s_btled_systmr, MS2ST(1000), App_BTLED_FFB3Timer, NULL);
	chSysUnlockFromISR();
#endif
	return;
}

static void App_BTLED_HandleFFB4(UINT8* data, UINT32 size)
{
  (void)data;
  (void)size;
}

static struct APP_BTLED_HandleStru s_btled_handlist[] = 
{
	{0xFFB1, App_BTLED_HandleFFB1},
	{0xFFB2, App_BTLED_HandleFFB2},
	{0xFFB3, App_BTLED_HandleFFB3},
	{0xFFB4, App_BTLED_HandleFFB4},
};
static UINT16 handlesize = sizeof(s_btled_handlist)/sizeof(struct APP_BTLED_HandleStru);
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static void App_BTLED_Callback(UINT16 uuid, UINT8* data, UINT32 size)
{
	UINT16 index = 0;
	for(index =0;index<handlesize; index++)
	{
		if (uuid == s_btled_handlist[index].uuid)
		{
			(*((APP_BTLED_CbkTableFunc *)s_btled_handlist[index].func))(data, size);
			break;
		}
	}
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_BTLED_Init(void)
{
    App_BLE_Register(App_BTLED_Callback);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_BTLED_Uninit(void)
{
    App_BLE_Unregister();
}

void App_BTLED_SendData(UINT8* data, UINT16 size)
{
  (void)data;
  (void)size;
	//APP_BLE_SendData(UINT16 uuid, data, size, 1);
}
#endif

