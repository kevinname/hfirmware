/*
 * lib_infrared.h
 *
 *  Created on: Mar 9, 2017
 *      Author: woshi
 */

#ifndef LIB_INFRARED_H_
#define LIB_INFRARED_H_

#ifdef __cplusplus
extern "C" {
#endif

#if HAL_USE_IR


//  按键类型
typedef enum
{
  NEC_SIGNAL_PRESS = 0,
  NEC_LONG_PRESS
}nec_key_type_t;

// 按键属性(索引      特征值    名    是否支持长按)
typedef struct
{
	uint8_t key_index;
	uint8_t key_data;
	char    key_name[7];
	uint8_t spt_longpress;
}nec_key_info_t;


//遥控器支持列表（设备特征地址   按键属性表）
typedef struct
{
	uint8_t dev_address;
	nec_key_info_t *nec_key_dictionary;
}nec_dev_dict_t;


//解码遥控按键消息 (按键属性      此次按键的类型(单击/长按))
typedef struct
{
	nec_key_info_t     nec_key;
	nec_key_type_t     signal_long;
}nec_mesg_t;


//  IR消息结构
typedef struct
{
	char       key_name[7];        // 按键名
	uint8_t    special_arg_cnt;    // 额外需要的参数数目（最小有效数目）
	uint8_t    u8EventMod;         // 消息层索引  ..... 消息类型默认HS_CFG_SYS_EVENT
	uint16_t   u16Event;           // 消息key值
	uint32_t   u32Arg;             // 消息附加参数
}ir_sys_mesg_t;

void hs_infrared_init(void);
void hs_infrared_uninit(void);
void hs_user_handle(void *parg);
#endif


#ifdef __cplusplus
}
#endif


#endif /* LIB_INFRARED_H_ */
