/*
 * infrared_interface.c
 *
 *  Created on: May 27, 2017
 *      Author: woshi  duo yuntao
 */

#include "lib.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "infrared_key_info.h"
#define REF 1 // 此控制的代码是对遥控信息的接收和后续控制的一种参考,用户可自主设计其他适合自身的方式

#if HAL_USE_IR
#include "lib_infrared.c"

#if REF != 1
void hs_user_handle(void *parg)
{
	nec_mesg_t *nec_mesg_parg = (nec_mesg_t *)parg;
	/*
	 *    用户会接收的消息  消息结构参加"lib_infrared.h" 中的 nec_mesg_t
	 *    根据消息的name等用户可以进行自主操作开发
	 *    {
	 *      nec_key_info_t     nec_key;
	 *      nec_key_type_t     signal_long;
	 *    }
	 *    EXP ：
	 *    if      nec_mesg_parg->nec_key.key_name = "stop"
	 *    then    hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_STOP)
	 */


	if(nec_mesg_parg!=NULL)
	{
		//在此添加用户操作代码


   }
}
#else

typedef struct
{
	uint8_t key_index;
	uint8_t special_arg_cnt;
}key_index_argcnt_t;
static bool need_ir_arg = false;   // true接受参数状态      false 不需要参数或参数已经接受完毕
static bool prepare_recv = false;  // 是否准备还接收参数
static key_index_argcnt_t key_index_argcnt[5]={};  // 同名的按键操作索引以及所需参数数目
static uint8_t  special_arg_cnt_kind = 0;  // 最多纪录5个同名操作及其参数数目
static uint8_t  special_arg_cnt_kind_min = 6; // 最小有效参数数目(最多接收5个参数)
static uint8_t  special_arg_cnt_kind_max = 0; // 最小有效参数数目(最多接收5个参数)
static uint8_t  recv_arg_cnt = 0;   // 获得的参数数目
static uint8_t *arg = NULL, *base_arg = NULL;
static uint8_t warning_mask = 0;  // 警告
static os_timer_type nec_arg_timer_type=osTimerOnce;   // 单次定时器
static osTimerId nec_arg_TimerID=NULL;
static osTimerDef_t nec_arg_stTmDef;
static uint32_t wait_arg_time = 2000;  // 2s的参数等待时间


static void _timer_arg(void const *arg)     // 定时器服务函数
{
	 (void)arg;
	 need_ir_arg = false;
	 hs_user_handle((void*)(uint32_t)(-1));
}

void hs_user_handle(void *parg)
{
	nec_mesg_t *nec_mesg_parg = (nec_mesg_t *)parg;
	/*
	 *    用户会接收的消息  消息结构参加"lib_infrared.h" 中的 nec_mesg_t
	 *    根据消息的name等用户可以进行自主操作开发
	 *    {
	 *      nec_key_info_t     nec_key;
	 *      nec_key_type_t     signal_long;
	 *    }
	 *    EXP ：
	 *    if      nec_mesg_parg->nec_key.key_name = "stop"
	 *    then    hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_STOP)
	 */

	// ----------------------------------以下部分仅做参考------------------------------------------


	int i,j;
	if(nec_mesg_parg!=NULL )
	{

		// --------- 开辟内存以及定时器の初始化 --------
		if(base_arg==NULL)
		{
			base_arg = hs_malloc(sizeof(uint8_t)*5, __MT_Z_GENERAL);
			if(base_arg==NULL)
			{
				hs_printf("Not enough memory for special_arg\r\n");
				return;
			}
		}
		if(nec_arg_TimerID==NULL)
		{
			// Create Timer
			nec_arg_stTmDef.ptimer = _timer_arg;
			nec_arg_TimerID = oshalTimerCreate(&nec_arg_stTmDef,nec_arg_timer_type, NULL);

			if(nec_arg_TimerID==NULL)
			{
				hs_printf("Create Timer Error\r\n");
				return;
			}
		}
		// --------------------------------------------
		if((uint32_t)nec_mesg_parg == (uint32_t)(-1))
			goto SEND;
		/*
		 *  a 有的命令不需要参数，则接收到之后直接发送消息；
		 *  b 有的命令需要参数，可能还不止一个，则当在一定的时间内通过遥控器获取足够的参数后发送相关消息
		 *     b.0当在设定的时间内没有获得足够的合法参数的时候，跳过，不作处理；
		 *     b.1 多个参数时候  输入完一个参数,(定时器重新计时)开始接受下一个参数;
		 *     b.3 目前参数只接受0-9的数字
		 */

		// @ 1 匹配按键命令名字的索引,并获取其需要的special参数数目

		if(need_ir_arg != true || special_arg_cnt_kind == 0)
		{
			//  未解析出操作名之前不接收0-9参数    或   不需要参数
			if(nec_mesg_parg->nec_key.key_index<=9)
			{
				//  没有合法的得到合法的操作码 或者 没有其他操作码之前就输入数字0-9
				warning_mask |= 0x02;
				goto END;
			}
		}


		for(i=0;i<nec_event_cnt;i++)
		{
			if(nec_mesg_parg->nec_key.key_index<=9)
				break;
			if(i==0)
			{
				// 收到操作名，重新搜索匹配
				special_arg_cnt_kind = 0;     // 最多纪录5个同名操作及其参数数目
				special_arg_cnt_kind_min = 6; // 最小有效参数数目(最多接收5个参数)
				special_arg_cnt_kind_max = 0; // 最大有效参数数目(最多接收5个参数)
				recv_arg_cnt = 0;   // 获得的参数数目
				need_ir_arg=false;  // 默认不需要参数
				memset(key_index_argcnt,0,sizeof(key_index_argcnt_t)*5);
				warning_mask = 0;
			}
			if(strcmp(nec_mesg_parg->nec_key.key_name,ir_sys_mesg_dict[i].key_name)==0)
			{
				key_index_argcnt[special_arg_cnt_kind].key_index = i;
				key_index_argcnt[special_arg_cnt_kind].special_arg_cnt = ir_sys_mesg_dict[i].special_arg_cnt;
				if(special_arg_cnt_kind_min>ir_sys_mesg_dict[i].special_arg_cnt)
					special_arg_cnt_kind_min = ir_sys_mesg_dict[i].special_arg_cnt;
				if(special_arg_cnt_kind_max<ir_sys_mesg_dict[i].special_arg_cnt)
					special_arg_cnt_kind_max = ir_sys_mesg_dict[i].special_arg_cnt;
				special_arg_cnt_kind += 1;
				if(special_arg_cnt_kind>=5)
				{
					hs_printf("Five operations of the same name are supported at most\r\n");
					break;
				}

			}
			if(i==nec_event_cnt-1)
			{
				if( special_arg_cnt_kind==0 )
				{
					//  没有匹配合法的操作
					warning_mask |= 0x01;
					goto END;
				}
				else
				{
					if(special_arg_cnt_kind_max!=0)
					{
						//  需要输入参数
						need_ir_arg = true;
					}
				}
			}
		}



		// @ 2 发送消息
		//      2.1需要参数则负责接收
		if(need_ir_arg==true)
		{
			if(prepare_recv == false)
			{

				//  准备工作
				arg = base_arg;
				oshalTimerStop(nec_arg_TimerID);
				oshalTimerStart(nec_arg_TimerID,wait_arg_time);
				prepare_recv = true;

			}
			else
			{
				// 接收并存放参数
				if(nec_mesg_parg->nec_key.key_index<=9)
				{
					*arg = nec_mesg_parg->nec_key.key_index;
					arg++;
					hs_printf("Receive : %d\r\n",*(arg-1));

					recv_arg_cnt+=1;
					if(recv_arg_cnt<special_arg_cnt_kind_max)
					{
						//  接收下一个参数
						oshalTimerStop(nec_arg_TimerID);
						oshalTimerStart(nec_arg_TimerID,wait_arg_time);
					}
					else
					{
						// 接收完了最多需要的参数个数
						hs_printf("The special_args is full\r\n");
						need_ir_arg = false;
						oshalTimerStop(nec_arg_TimerID);
					}
				}

			}

		}


		// 2.2  参数合法或不需要参数，则开始发送消息
SEND:
		if(need_ir_arg==false && special_arg_cnt_kind!=0)
		{
			// 根据接收到参数（数目）进行对应操作
			if(recv_arg_cnt<special_arg_cnt_kind_min)
			{
				warning_mask |= 0x04;
				goto END;
			}
			hs_printf("@Sending.......\r\n\n");
            for(i=0;i<special_arg_cnt_kind;i++)
            {
            	if(key_index_argcnt[i].special_arg_cnt==recv_arg_cnt)
            	{
            		 j = key_index_argcnt[i].key_index;
            		 if(j>=18&&j<=21)
            		 {
            			 hs_cfg_sysSendMsg((hs_cfg_mod_t)ir_sys_mesg_dict[j].u8EventMod, HS_CFG_SYS_EVENT,
											(hs_cfg_event_type_t)(ir_sys_mesg_dict[j].u16Event | FAST_EVENT_MASK),
											(void*)((uint32_t)(*base_arg)));
            		 }
            		 else
            		 {
            			 hs_cfg_sysSendMsg((hs_cfg_mod_t)ir_sys_mesg_dict[j].u8EventMod, HS_CFG_SYS_EVENT,
											(hs_cfg_event_type_t)(ir_sys_mesg_dict[j].u16Event | FAST_EVENT_MASK),
											(recv_arg_cnt==0?(void *)ir_sys_mesg_dict[j].u32Arg:(void*)base_arg));

            		 }


            	}

            }

			special_arg_cnt_kind = 0;  // 最多纪录5个同名操作及其参数数目
			special_arg_cnt_kind_min = 6; // 最小有效参数数目(最多接收5个参数)
			special_arg_cnt_kind_max = 0; // 最小有效参数数目(最多接收5个参数)
			recv_arg_cnt = 0;   // 获得的参数数目
			prepare_recv = false;
			memset(key_index_argcnt,0,sizeof(key_index_argcnt_t)*5);
			warning_mask = 0;
		}


 END:
	 if((warning_mask & 0x01) != 0 )
	 {

		 //  没有对应的合法操作
		 hs_printf("........ Warning 0x01 .......\r\n");
		 hs_printf("No legal operation\r\n\n");
		 warning_mask &= ~(0x01);
	 }
	if((warning_mask & 0x02) != 0 )
	 {
		 hs_printf("........ Warning 0x02.......\r\n");
		 hs_printf("Please input operation before digital_args\r\n");
		 hs_printf("If the operation need digital_args please input in given time\r\n\n");
		 warning_mask &= ~(0x02);
	 }
	 if((warning_mask & 0x04)!=0)
	 {
		 hs_printf("........ Warning 0x04.......\r\n");
		 hs_printf("Illegal arg_number\r\n\n");
		 warning_mask &= ~(0x04);
		 special_arg_cnt_kind = 0;  // 最多纪录5个同名操作及其参数数目
		 special_arg_cnt_kind_min = 6; // 最小有效参数数目(最多接收5个参数)
		 special_arg_cnt_kind_max = 0; // 最小有效参数数目(最多接收5个参数)
		 recv_arg_cnt = 0;   // 获得的参数数目
		 prepare_recv = false;
		 memset(key_index_argcnt,0,sizeof(key_index_argcnt_t)*5);
	 }
	}
}


#endif    //  end REF

#endif    //  end HS_IR_USE





