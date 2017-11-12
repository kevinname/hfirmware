/*
    drvhal - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/audio/lib_ans.h
 * @brief   codec include file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */
#ifndef __LIB_ANS_H__
#define __LIB_ANS_H__

#include "lib.h"

#if HAL_USE_AUDIO

#define FRAME_SIZE        60
#define BLOCK_SIZE        256
#define LOG_2_BLOCK       8
#define FIFO_SIZE         6
#define FIFO_DEPTH        1
#define BANK_SIZE         19
#define D                 40
#define V                 10
#define U                 4
#define LOG_DB_BITS       6
#define INIT_LOG_SEARCH   32

#define OL_SCALE          0x7685

#define MAX_16U           65535
#define MAX_15U           32768
#define P8_16U	          0xCCCD
#define MAX_32U	          0xFFFFFFFF

typedef struct
{
  long  micWin[BLOCK_SIZE];
  short micIn[4][FRAME_SIZE];
  short mic_in_frame_index;
  short youtBuffer[BLOCK_SIZE];

  long  micWin_f_real_fifo[FIFO_SIZE][BLOCK_SIZE/2+1];
  long  micWin_f_imag_fifo[FIFO_SIZE][BLOCK_SIZE/2+1];
  short fifo_index;
  long  pn_fifo[FIFO_SIZE][BANK_SIZE];
  long  px_fifo[FIFO_SIZE][BANK_SIZE];

  unsigned long px_fifo_pre[BANK_SIZE];
  unsigned long pn_fifo_pre[BANK_SIZE];

  unsigned long lmin_flag[BANK_SIZE];

  short first_frame;
  unsigned long pmin_act[BANK_SIZE];
  unsigned long pmin_act_sub[BANK_SIZE];
  unsigned short subwc;
  unsigned long pmin_sub[U][BANK_SIZE];
  short pmin_sub_index;
  unsigned long pmin[BANK_SIZE];

  unsigned short alpha_max;
  unsigned short cmax1;
  unsigned short cmax;
  unsigned short alpha_c;
  unsigned short alpha_min;
  unsigned short Md;
  unsigned short Md_sub;
  unsigned short alpha_v;
  unsigned short snr_post_th;
  unsigned short alpha_snr_pri;
  unsigned long gain_nb_min;
  unsigned short noise_min_thd;
  unsigned short gain_max_db;	//Q0
  unsigned short alpha_gain_max;
  unsigned short alpha_gain_min;
  unsigned short alpha_gain_nb_min;

  unsigned long gain_nb[BANK_SIZE];
  unsigned long SNR_post[BANK_SIZE];
  unsigned short px_mean[BANK_SIZE];
  unsigned long px_square[BANK_SIZE];

  long micWin_f_imag[BLOCK_SIZE];
  long micWin_f_real[BLOCK_SIZE];
  unsigned long micWin_f_amp [BLOCK_SIZE/2+1];
  unsigned long pn[BANK_SIZE];
  unsigned long px[BANK_SIZE];
  unsigned long px_sum;
  unsigned long pr_sum;
  unsigned long pr[BANK_SIZE];

  unsigned short alpha_ce;
  unsigned short SNR_temp[BANK_SIZE];	//Q14
  unsigned short alpha[BANK_SIZE];
  unsigned short beta_q[BANK_SIZE];  //Q16
  unsigned long px_var[BANK_SIZE];
  unsigned short Q_eq[BANK_SIZE];	//Q15
  unsigned short Q_eq_e[BANK_SIZE]; //Q8
  unsigned short Q_eq_e_sub[BANK_SIZE]; //Q8
  unsigned long Q_eq_recip;
  unsigned short Bmin[BANK_SIZE];		//Q8
  unsigned short Bmin_sub[BANK_SIZE];	//Q8
  unsigned short Q_1;	//Q15
  unsigned short Q_1_sqrt;
  unsigned short Bc;	//Q14
  unsigned short k_mode[BANK_SIZE];
  unsigned short noise_slope_max;	//Q8
  unsigned short noise_increase_flag;

  unsigned short SNR_post_1[BANK_SIZE];
  unsigned short I_1[BANK_SIZE];

  unsigned long SNR_post_pre[BANK_SIZE];
  unsigned long SNR_pri[BANK_SIZE];

  unsigned long gain_nb_pre[BANK_SIZE];
  unsigned long px_avg;
  unsigned long gain_nb_min_pre;
  unsigned short px_avg_sqrt;
  unsigned short gain_nb_db[BANK_SIZE];//Q0

  unsigned short alpha_gain[BANK_SIZE];//Q16
  unsigned long gain[BLOCK_SIZE/2+1];
}hs_ansvar_t;


#ifdef __cplusplus
extern "C" {
#endif

void hs_ans_init(void);
void hs_ans_uninit(void);
void hs_ans_process(short *xin, short *yout);

#ifdef __cplusplus
}
#endif

#endif

#endif
 /** @} */

