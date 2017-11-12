/*
 * cfg_infrared.c
 *
 *  Created on: Mar 6, 2017
 *      Author: woshi dou yuntao
 */
#include "lib.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "infrared_key_info.h"

#if HAL_USE_IR

/********************CMD_NEC*********************/
/*
 *    PA6  Timer 1-IO1
 */

static uint8_t  default_nec_kind = 0; // 默认的NEC设备的特征地址
static uint8_t  nec_dev_index = 0; // 默认的NEC设备的索引号
static uint8_t  rcon_kind_unmatch = 0;// 不匹配时为1
static uint8_t  is_training = 0;   // 学习模式的时候大于0
static uint8_t  ready_for_long_press = 0; // 当大于0时候可以解析长按
static uint8_t  terminate_nec_thd = 0;   //  若检测到为1则 退出NEC的decode线程
static uint8_t  silent=1;            // 静默状态
static uint8_t  idle_lead_long=0;    //
static uint8_t  io_fall_irq_en=0;
static uint32_t last_width=0;        // 引导码的特征宽度
static uint8_t  bit_num=0;           // 1-32的bit索引
static uint16_t nec_data[33];        // 存储   引导码+1-32地址和数据
static osThreadId nec_thd=NULL;     // NEC 线程句柄

static ICUDriver *icud = NULL;    				//  ICU  Driver
static icuchannel_t icu_channel = ICU_CHANNEL_1;       //  ICU channel


typedef struct
{
  osThreadId pThd;
}nec_thd_arg_t;

static osThreadDef_t thdDef_nec;           // NEC线程
static nec_thd_arg_t *thd_parg_nec=NULL;

static os_timer_type nec_timer_type=osTimerOnce;   // 单次定时器
static osTimerId nec_TimerID=NULL;
static osTimerDef_t nec_stTmDef;

static void _nec_mesg_service(uint16_t u16Msg, void *parg)
{

	(void) u16Msg;
	nec_mesg_t *nec_mesg_parg = (nec_mesg_t *)parg;
	hs_printf("NEC:  name # %s   (index # %d    data # %d)  @ %s\r\n",nec_mesg_parg->nec_key.key_name,nec_mesg_parg->nec_key.key_index,\
			nec_mesg_parg->nec_key.key_data,(nec_mesg_parg->signal_long == NEC_SIGNAL_PRESS)?"SIGNAL":"LONG");

	//  TODO此添加用户想要的操作
	hs_user_handle(nec_mesg_parg);
	// 释放内存
	hs_free(parg);
}

static void _timer_xms(void const *arg)     // 定时器服务函数
{
	 (void)arg;
	 if(silent==1)
	 {
		 // 定时时间到，但是依然是静默状态
		 // 则认为失去长按的重复对象，设置为不接收长按，需要从头接收新的按键信息(创建新的对象)
		 idle_lead_long=0;
	 }

}

static uint8_t  address,address_m,data,data_m;  // 记录地址    &  数据

static void bit_to_byte()   // 32bit ---> 4byte
{
	uint8_t i;
	address=0;
	address_m=0;
	data=0;
	data_m=0;
	for(i=1;i<33;i++)
	{
		if(i<=8)  // 地址
		{
			address=address<<1;
			if(nec_data[i]==1)
				address=address|0x01;
		}

		if(i>8&&i<=16)   // 地址反码
		{
			address_m=address_m<<1;
			if(nec_data[i]==1)
				address_m=address_m|0x01;
		}

		if(i>16&&i<=24)   // 数据码
		{
			data=data<<1;
			if(nec_data[i]==1)
				data=data|0x01;
		}

		if(i>24&&i<=32)   // 数据反码
		{
			data_m=data_m<<1;
			if(nec_data[i]==1)
				data_m=data_m|0x01;
		}
		nec_data[i]=2;   //  bit位取出后，相应的位置 设为2（即非0非1）
	}

}

static uint8_t check_key(uint8_t const data)
{
	uint8_t key_code=111;
	int i,j;
	for(j=0;j<nec_dev_num;j++)
	{
		if(nec_dev_dict[j].dev_address == default_nec_kind)
		{
			nec_dev_index = j;
			break;
		}

	}
	for(i=0;i<nec_dict_len;i++)
	{

		if(nec_dev_dict[nec_dev_index].nec_key_dictionary[i].key_data == data )
		{
			key_code = nec_dev_dict[nec_dev_index].nec_key_dictionary[i].key_index;
			break;
		}
	}
	return key_code;
}


static void icuwidthcb(ICUDriver *icup)
{

  static uint8_t long_press_time=0;
  last_width = icuGetWidth(icup);    //  特征脉宽

  if(bit_num==0)
  {
	  if(last_width>430&&last_width<500)
	  {
		  //  Start 特征
		  nec_data[bit_num]=last_width;
		  idle_lead_long=1;
		  long_press_time=0;

	  }

	  else if((last_width>225&&last_width<300)&&is_training==0)
	  {
		  // Long(Repeat) 特征
		  if(idle_lead_long==0)
		  {
			  // 若现在的状态不接收长按 则返回
			  return;
		  }
		  else
		  {
			  if(ready_for_long_press==0)
			  {
				  return;
			  }
			  long_press_time+=1;
			  if(long_press_time==long_press_effect)
			  {
				  // 连续X次长按 算作一次有效长按，防止误触发
				 long_press_time=0;
				 nec_data[bit_num]=last_width;
				 idle_lead_long=2;
				 bit_num=101;
			  }
			  return;
		  }
	  }
	  else
	  {
		  //其他不合法的特征
		  return;
	  }

  }

  else if(bit_num>0 && bit_num<33)
  {
	  //  bit 1-32
	  // 采集地址和数据位
	  if(last_width>50&&last_width<70)
	 	  nec_data[bit_num]=0;
	   else if(last_width>160&&last_width<186)
	 	  nec_data[bit_num]=1;
  }

  bit_num++;
}


static ICUConfig icu_pio_cfg = {
  ICU_INPUT_ACTIVE_HIGH,
  100000,                                    /* 100KHz ICU clock frequency.   */
  icuwidthcb,
  NULL,
  NULL,
  ICU_CHANNEL_1,              //  ICU  channel
  0
#if ICU_USE_DMA
  ,
  FALSE,
  NULL
#endif
};

static void _rising_eadge(ioportid_t port, uint8_t pad)
{
	(void)port;
	(void)pad;
	return;
}
static void _falling_eadge(ioportid_t port, uint8_t pad)
{
	(void)port;
	(void)pad;
	if(silent==1)
	{
		// 静默状态接收到下降沿 则结束静默状态------可能是长按 也可能是新的按键
		silent=0;
	}

	if(io_fall_irq_en==1)
	{
		// 发送信号，接收特征值
		oshalSignalSet(nec_thd,1);
	}

	else
	{
		// 发送信号，解析命令
		if(bit_num==32||bit_num>100)
			oshalSignalSet(nec_thd,2);
		return;
	}

}


/*
 *    NEC 线程服务函数
 */
static void _nec_schl(void const *arg)
{
	//nec_thd_arg_t *parg = (nec_thd_arg_t *)arg;
	(void)arg;
	//-----------------------INIT DEVICE -----------------
#if HS_ICU_USE_TIM1
	icud = &ICUD1;
#elif HS_ICU_USE_TIM2
	icud = &ICUD2;
#endif
	icu_channel = (IR_pin==PA6 || IR_pin==PB5)?ICU_CHANNEL_1:ICU_CHANNEL_2;
	icu_pio_cfg.channel = icu_channel;
	//----------------------------------------------------
	osEvent evt_signal;

	static uint8_t wait_to_start_icu=1;
	static uint8_t wait_to_stop_icu=0;

	uint8_t key;
	static uint8_t last_key;


	// Create Timer
	nec_stTmDef.ptimer = _timer_xms;
	nec_TimerID = oshalTimerCreate(&nec_stTmDef, nec_timer_type, NULL);

	 // the GPIO Pin for IRQ   PA6  下降沿
	palSetPadMode((IR_pin<16)?IOPORT0:IOPORT1, IR_pin, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
	palRegEvent(IR_pin, FALLING_EDGE, (ioevent_t)_falling_eadge);
	io_fall_irq_en=1;   // 开启下降沿中断 标志

	chRegSetThreadName("NEC.SCHL");
	hs_printf("Nec_Schl Thread OK\r\n");

	while(1)
	{

		if(terminate_nec_thd==1)
		{
			/* Check and Exit the thread */
			palRegEvent(IR_pin, FALLING_EDGE, (ioevent_t)_rising_eadge);
			io_fall_irq_en=0;
			terminate_nec_thd = 0;
			break;
		}

		if(wait_to_start_icu==1)
		{
			// 等待    启动   解析LEAD

			evt_signal=oshalSignalWait(1,1);
			if(evt_signal.status==osEventTimeout)
			{
				continue;
			}
			wait_to_start_icu=0;
			io_fall_irq_en=0;
			// Begin ICU MODE . ICU TIMER 1
			palSetPadMode((IR_pin<16)?IOPORT0:IOPORT1, IR_pin, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(PAD_FUNC_TIMER2_3) | PAL_MODE_DRIVE_CAP(3));
			icuStart(icud, &icu_pio_cfg);
			icuEnable(icud);
			wait_to_stop_icu=1;
		}
		if(wait_to_stop_icu==1)
		{
			/* 等待解析   地址&数据*/
			evt_signal=oshalSignalWait(2,1);
			if(evt_signal.status==osEventTimeout)
			{
				continue;
			}
			palRegEvent(IR_pin, RISING_EDGE, (ioevent_t)_rising_eadge);


			wait_to_stop_icu=0;
			icuDisable(icud);
			icuStop(icud);

			if(bit_num==33)
			{
				//  THE NEW DATA
				bit_to_byte();
				if((data&data_m)==0)
				{
					//hs_printf("\r\nLead %d \r\n",nec_data[0]);
					nec_data[0]=2;
					/*默认0x00的遥控器，需要检测地址数否匹配  RCON_Kind是否match*/

					if(address!=default_nec_kind)
					{
						//  地址 不匹配     说明是不同的遥控器
						//  查询并切换
						rcon_kind_unmatch = 1;
						// 查询NEC设备字典
						int k;
						for(k=0;k<nec_dev_num;k++)
						{
							if(nec_dev_dict[k].dev_address!=default_nec_kind)
							{
								if(nec_dev_dict[k].dev_address == address)
								{
									default_nec_kind = address;
									nec_dev_index = k;
									rcon_kind_unmatch=0;
									break;
								}
							}
						}
					}
					// （即不存类型）时才进行
					// 且需要得到的地址是符合NEC标准的
					if(rcon_kind_unmatch==1)
					{
						// 需要进行学习
						is_training = 1;
					}

					// ---------------------------------------Train
					if(is_training==1)
					{
						hs_printf("\r\nTrain Mode: Address: %d    Data: %d \r\n",address, data);
					}
					// ---------------------------------------Train
					else
					{
						key=check_key(data); // 错误时候返回111
						if(key!=111)
						{
							if(nec_dev_dict[nec_dev_index].nec_key_dictionary[key].spt_longpress==1)
							{
								ready_for_long_press = 1;
							}
							else
							{
								ready_for_long_press = 0;
							}
							last_key=key;
							/*
							hs_printf("Key_Index: %d  Key_Data: %d  Key_Name: %s  SPT_LongPress: %s\r\n\r\n",key,\
									nec_key_dictionary[key].key_data,nec_key_dictionary[key].key_name,\
									(nec_key_dictionary[key].spt_longpress==1)?"YES":"NO");
							*/
							nec_mesg_t  *mesg_arg = hs_malloc(sizeof(nec_mesg_t), __MT_Z_GENERAL);
							mesg_arg->nec_key.key_index = key;
							mesg_arg->nec_key.key_data = nec_dev_dict[nec_dev_index].nec_key_dictionary[key].key_data;
							strcpy(mesg_arg->nec_key.key_name, nec_dev_dict[nec_dev_index].nec_key_dictionary[key].key_name);
							mesg_arg->signal_long = NEC_SIGNAL_PRESS;
							hs_cfg_sysSendMsg(HS_CFG_MODULE_USER1,HS_CFG_SYS_EVENT,\
										     HS_CFG_EVENT_IR_INFO,(void *)mesg_arg);
						}

					}

				}
				osDelay(80);
				palRegEvent(IR_pin,FALLING_EDGE, (ioevent_t)_falling_eadge);
				oshalTimerStop(nec_TimerID);
				oshalTimerStart(nec_TimerID,250);
				silent=1;
				bit_num=0;
				io_fall_irq_en=1;
				wait_to_start_icu=1;

			}

			else if(bit_num>100)
			{
				// LONG PRESS
				oshalTimerStop(nec_TimerID);
				if((data&data_m)==0)
				{
					//hs_printf("\r\nLong Press %d \r\n",nec_data[0]);
					nec_data[0]=2;
					/*
					hs_printf("Key_Index: %d  Key_Data: %d  Key_Name: %s  SPT_LongPress: %s\r\n\r\n",last_key,\
								nec_key_dictionary[last_key].key_data,nec_key_dictionary[last_key].key_name,\
								(nec_key_dictionary[last_key].spt_longpress==1)?"YES":"NO");
					*/
					nec_mesg_t  *mesg_arg = hs_malloc(sizeof(nec_mesg_t), __MT_Z_GENERAL);
					mesg_arg->nec_key.key_index = last_key;
					mesg_arg->nec_key.key_data = nec_dev_dict[nec_dev_index].nec_key_dictionary[last_key].key_data;
					strcpy(mesg_arg->nec_key.key_name,nec_dev_dict[nec_dev_index].nec_key_dictionary[last_key].key_name);
					mesg_arg->signal_long = NEC_LONG_PRESS;
					hs_cfg_sysSendMsg(HS_CFG_MODULE_USER1,HS_CFG_SYS_EVENT,\
									 HS_CFG_EVENT_IR_INFO,(void *)mesg_arg);
				}
				osDelay(30);
				palRegEvent(IR_pin,FALLING_EDGE, (ioevent_t)_falling_eadge);
				oshalTimerStop(nec_TimerID);
				oshalTimerStart(nec_TimerID,180);
				silent=1;
				bit_num=0;
				io_fall_irq_en=1;
				wait_to_start_icu=1;

			}
		}
	}

}

void cmd_nec(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)chp;
	if(argc==1)
	{
		if(strcmp(argv[0],"1")==0)
		{
			if(nec_thd!=NULL)
			{
				hs_printf("Nec_Schl Thread Is Running Already\r\n");
				goto HELP;
			}

			//  Create THD for NEC
			thd_parg_nec = hs_malloc(sizeof(nec_thd_arg_t), __MT_Z_GENERAL);
			if(thd_parg_nec == NULL)
			 {
				hs_printf( "To get memory for NEC Thread error\r\n");
				return;
			 }
			thd_parg_nec->pThd = curthread();    //  当前线程句柄
			thdDef_nec.pthread   = (os_pthread)_nec_schl;
			thdDef_nec.stacksize = 512;
			thdDef_nec.tpriority = osPriorityRealtime;
			// create thread
			nec_thd = oshalThreadCreate(&thdDef_nec, thd_parg_nec);
			terminate_nec_thd = 0;

			hs_cfg_sysListenMsg(HS_CFG_EVENT_IR_INFO, _nec_mesg_service);
			return;
		}

		else if(strcmp(argv[0],"0")==0)
		{
			if(nec_thd==NULL)
			{
				hs_printf("Nec_Schl Thread Is Not Running\r\n");
				goto HELP;
			}

			thd_parg_nec->pThd = curthread();   // 当前线程句柄
			terminate_nec_thd=1;
			while(terminate_nec_thd==1)
			{
				msleep(50);
				continue;
			}
			oshalThreadTerminate(nec_thd);
			nec_thd = NULL;
		    hs_free(thd_parg_nec);
			hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_IR_INFO, _nec_mesg_service);
			hs_printf("Nec_Schl Thread Over\r\n");
			return;

		}

		else if(strcmp(argv[0],"t")==0)
		{
			if(nec_thd==NULL)
			{
				hs_printf("Nec_Schl Thread Is Not Running\r\n");
				goto HELP;
			}
			if(is_training!=1)
			{
				hs_printf("Start to learn new NEC, Need the user input with shell\r\n");
				is_training = 1;
				return;
			}
			else
			{
				hs_printf("Nec_Schl Thread Is Training Already\r\n");
				goto HELP;
        	}

		}
		else if(strcmp(argv[0],"to")==0)
		{
			if(nec_thd==NULL)
			{
				hs_printf("Nec_Schl Thread Is Not In Running\r\n");
				goto HELP;
			}
			if(is_training==0)
			{
				hs_printf("Nec_Schl Thread Is Not In Training\r\n");
				goto HELP;
			}
			hs_printf("Train new NEC Over\r\n");
			is_training = 0;
			default_nec_kind = address;
			return;
		}

		else if(strcmp(argv[0],"d")==0)
		{

			int i,j;
			hs_printf("\r\nCurrent NEC_DEV Address:  %d \r\n",default_nec_kind);
			for(j=0;j<nec_dev_num;j++)
			{
				hs_printf("\r\nNEC_DEV Address:  %d \r\n",nec_dev_dict[j].dev_address);
				hs_printf("-------------------------------\r\n");
				hs_printf("Key_index     Key_data     Key_name     SPT_LongPress\r\n");
				hs_printf("---------------------------------------------------------\r\n");
				for(i=0;i<nec_dict_len;i++)
				{
					hs_printf("%3d            %5d   %8s            %s\r\n",nec_dev_dict[j].nec_key_dictionary[i].key_index,\
							nec_dev_dict[j].nec_key_dictionary[i].key_data,nec_dev_dict[j].nec_key_dictionary[i].key_name,\
							 (nec_dev_dict[j].nec_key_dictionary[i].spt_longpress==1)?"YES":"NO");
				}
				hs_printf("---------------------------------------------------------\r\n");
			}
			return;
		}

		else
		{
			goto HELP;
		}
	}

	else if(argc==2 && strcmp(argv[0],"d")==0)
	{
		int i = atoi(argv[1]);
		if(i>nec_dict_len || i<0)
		{
			hs_printf("key_index is in 0-20\r\n");
			return;
		}
		hs_printf("\r\nDictionary Index:  %d \r\n",i);
		hs_printf("-------------------------------\r\n");
		hs_printf("Key_index     Key_data     Key_name     SPT_LongPress\r\n");
		hs_printf("---------------------------------------------------------\r\n");
		hs_printf("%3d            %5d           %s            %s\r\n",nec_dev_dict[nec_dev_index].nec_key_dictionary[i].key_index,\
				nec_dev_dict[nec_dev_index].nec_key_dictionary[i].key_data,nec_dev_dict[nec_dev_index].nec_key_dictionary[i].key_name,\
					 (nec_dev_dict[nec_dev_index].nec_key_dictionary[i].spt_longpress==1)?"YES":"NO");
		hs_printf("---------------------------------------------------------\r\n");
		return;

	}

	else if(argc==4||argc==5)
	{
		if(strcmp(argv[0],"t")!=0)
		{
			goto HELP;
		}
		if(nec_thd==NULL)
		{
			hs_printf("Nec_Schl Thread Is Not In Running\r\n");
			goto HELP;
		}
		if(is_training==0)
		{
			hs_printf("Nec_Schl Thread Is Not In Training\r\n");
			goto HELP;
		}

		uint16_t key_index;
		uint16_t key_data;
		char    key_name[7]="UNK";
		uint8_t  spt_longpress;

        int i;
        // .................index
		key_index = atoi(argv[1]);
		if(key_index>=nec_dict_len)
		{
			hs_printf("key_index is in 0-20\r\n");
			return;
		}

		// ..................data
		key_data = atoi(argv[2]);
		if(key_data>0xFF)
		{
			hs_printf("key_data is in 0-255\r\n");
			return;
		}

		for(i=0;i<nec_dict_len;i++)
		{
			if((i!=key_index)&&(nec_dev_dict[nec_dev_index].nec_key_dictionary[i].key_data == key_data))
			{
				// 为防止重复的key data
				// 将原来的 擦除  设为-1
				nec_dev_dict[nec_dev_index].nec_key_dictionary[i].key_data = -1;
				break;
			}
		}

		// ...................name
		if(strlen(argv[3])>=7)
		{
			hs_printf("key_name's length is must bellow 7\r\n");
			return;

		}
		strcpy(key_name,argv[3]);
		for(i=0;i<nec_dict_len;i++)
		{
			if((i!=key_index)&&(strcmp(nec_dev_dict[nec_dev_index].nec_key_dictionary[i].key_name,key_name)==0))
			{
				// 不允许重名
				// 将原来的 擦除  设为UNK
				strcpy(nec_dev_dict[nec_dev_index].nec_key_dictionary[i].key_name,"UNK");
				break;
			}
		}
		// ...................SPT_LongPress
		spt_longpress = nec_dev_dict[nec_dev_index].nec_key_dictionary[key_index].spt_longpress;
		if(argc==5)
		{
			spt_longpress = atoi(argv[4]);
			if(spt_longpress!=0 && spt_longpress!=1)
			{
				hs_printf("spt_longpress's value is only 0 OR 1\r\n");
				return;
			}
		}

		nec_dev_dict[nec_dev_index].nec_key_dictionary[key_index].key_data = key_data;
		strcpy(nec_dev_dict[nec_dev_index].nec_key_dictionary[key_index].key_name,key_name);
		nec_dev_dict[nec_dev_index].nec_key_dictionary[key_index].spt_longpress = spt_longpress;
		return;
	}

HELP:
        hs_printf("\r\n      NEC  Shell CMD     \r\n");
        hs_printf("-----------------------------------------------\r\n");
		hs_printf("nec 0             Terminate NEC Thread\r\n");
		hs_printf("nec 1             Start NEC Thread\r\n");
		hs_printf("nec d             Show the NEC Key_Dictionary\r\n");
		hs_printf("nec t             Enable Train \r\n");
		hs_printf("nec to            Disable Train \r\n");
		hs_printf("-----------------------------------------------\r\n");
		hs_printf("nec t key_index  key_data  key_name  SPT_LongPress  Train and Record new NEC key\r\n");
		hs_printf("EXP: nec  t  10  235  mode\r\n");
		hs_printf("EXP: nec  t  10  235  mute  0\r\n");
		hs_printf("-----------------------------------------------\r\n");
		hs_printf("nec ...           Show Help (except these input above)\r\n\n");

}

/*----------------------------------------------*/


// -----------------------------INIT & UNINIT-------------------------
void hs_infrared_init(void)
{
	IR_pin = HS_IR_INPUT;
	char *cmd_init = "1";
	char *this_argv[1];
	this_argv[0] = cmd_init;
	cmd_nec(NULL,1, this_argv);
}
void hs_infrared_uninit(void)
{
	char *cmd_uninit= "0";
	char *this_argv[0];
	this_argv[1] = cmd_uninit;
	cmd_nec(NULL,1, this_argv);
}

#endif
