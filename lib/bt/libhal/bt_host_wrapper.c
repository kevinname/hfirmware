/*
 * the wrapper of bt host stack
 */
#include "bt_config.h"

extern uint8_t *App_CFG_GetDeviceAddr(void);
extern uint8_t *App_CFG_GetDeviceName(void);

/*
 * return current state of bluetooth profiles
 */
uint16_t app_gap_get_state(void)
{
  return App_GetBtState();
}

/*
 * return the bluetooth device address.
 */
uint8_t *app_gap_get_addr(void)
{
  return App_CFG_GetDeviceAddr();
}

/*
 * return the bluetooth device name.
 */
uint8_t *app_gap_get_name(void)
{
  return App_CFG_GetDeviceName();
}

/*
 * Check whether the state of bluetooth profiles is conneced.
 * return: 1-connected 
 *         0-not
 */
uint8_t app_gap_is_connected(void)
{
  return App_GetBtState() >= APP_BT_STATE_CONNECTED ? 1 : 0;
}

/*
 * fill the result buffer with pair list
 * out@res_buf: the pointer to result buffer
 * in@u8GetCnt: the number of paired devices to get
 * return:      the number of paired devices
 */
uint8_t app_gap_get_pair_list(uint8_t *res_buf, uint8_t u8GetCnt)
{
  return App_GetPairList(res_buf, u8GetCnt);
}

/*
 * fill the result buffer with connection list
 * out@res_buf: the pointer to result buffer
 * in@u8GetCnt: the number of connected devices to get
 * return:      the number of connected devices
 */
uint8_t app_gap_get_conn_list(uint8_t *res_buf, uint8_t u8GetCnt)
{
  return App_GetConnectList(res_buf, u8GetCnt);
}

uint16_t app_a2dp_get_state(void)
{
  return hsc_A2DP_GetState();
}

uint16_t app_hfp_get_state(void)
{
  return hsc_HFP_GetState();
}

/*
 * Check whether the state of bluetooth spp is conneced.
 * return: 1-connected 
 *         0-not
 */
uint8_t app_spp_is_connected(void)
{
  return App_SPP_GetState() == APP_SPP_CONNECTED ? 1 : 0;
}

/*
 * Check whether the state of bluetooth spp is idle.
 * return: 1-connected 
 *         0-not
 */
uint8_t app_spp_is_idle(void)
{
  return App_SPP_GetState() == APP_SPP_IDLE ? 1 : 0;
}

