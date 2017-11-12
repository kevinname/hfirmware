/*
    drvhal - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/audio/lib_aec.c
 * @brief   lib_audio file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */

#include "lib_aec.h"
#include "math.h"

#if HAL_USE_AUDIO

#define sub_num             16
#define log_2_sub_num       4
#define sub_num_half        9		//sub_num/2+1
#define down_sample         8

#define hr_poly_len         9
//#define hr_poly_len_half  5  //hr_poly_len/2+1
#define fr_poly_len         5
//#define fr_poly_len_half  3  //fr_poly_len/2+1
#define hr_buf_len          144		//sub_num*hr_poly_len
#define fr_buf_len          10

//#define sub_buf_len       128
#define sub_filter_max_len  32
#define update_rate         1

#define Pmax                0xffffffff
#define noise_winNo         4
#define noise_winLen        150
//#define noise_winLen      149

#define nlp_max             1024

#define filterMuxhalf       (sub_filter_max_len * sub_num_half)


#define   Q23P       16777215
#define   Q23N       (-16777216)


typedef struct
{
  int16_t     s16B1A;
  int16_t     s16B1G;
  int32_t     s32B1X1;
  int32_t     s32B1Y1;

  int16_t     s16B5A;
  int16_t     s16B5G;
  int32_t     s32B5X1;
  int32_t     s32B5Y1;
}hs_aec_eq_t;

typedef struct
{
  uint8_t         u8SwGain;
  uint8_t         u8AecDis;

  short           s16Mu;
  short           s16Beta;
  short           s16FarAct1;
  short           s16FarAct2;
  short           s16PostGain;   

  short           s16NlpInc;
  short           s16AlphaEnergy;
  
  short           s16EqB1A;
  short           s16EqB5A;
  short           s16EqB1Gain;
  short           s16EqB5Gain;
}hs_aec_var_t;

typedef struct
{
  short fr_buf_index_pt;
  short fr_buf_pt;
  //short hr_buf_pt;
  long hr_buf_pt;
  //long hr_buf_pt_index;
  long y_out_buf_pt;
  long fifo_pt;

  short hr_buf_far[hr_buf_len];
  short hr_buf_near[hr_buf_len];

  long y_out_buf[sub_num];

  short near_fifo_real[sub_filter_max_len][sub_num_half];
  short near_fifo_imag[sub_filter_max_len][sub_num_half];
  short far_fifo_real[sub_filter_max_len][sub_num_half];
  short far_fifo_imag[sub_filter_max_len][sub_num_half];
  short far_echo_fifo_real[sub_filter_max_len][sub_num_half];
  short far_echo_fifo_imag[sub_filter_max_len][sub_num_half];

  unsigned long Enear[sub_num_half];
  //long Enear[sub_num_half];
  unsigned long Efar[sub_num_half];
  unsigned long Efar_echo[sub_num_half];
  unsigned long Eres[sub_num_half];
  //unsigned long far_energy[sub_num_half];
  long far_energy[sub_num_half];
  long res_energy[sub_num_half];

  long w_filter_real[sub_filter_max_len][sub_num_half];
  long w_filter_imag[sub_filter_max_len][sub_num_half];

  short w_filter_len[sub_num_half];

  short residue_real[sub_num_half];
  short residue_imag[sub_num_half];
  short w_update[sub_num_half];

  unsigned long Pmin_far[sub_num_half];
  unsigned long PLmin_far[noise_winNo][sub_num_half];
  unsigned long Pn_far[sub_num_half];

  short	wup_en_count[sub_num_half];
  short	wup_dis_count[sub_num_half];
  //unsigned short	nlp_count[sub_num_half];
  //unsigned short	nlp_gain[sub_num_half];
  short	nlp_count[sub_num_half];
  //short	nlp_gain[sub_num_half];
  unsigned short	erl[sub_num_half];
  short	fr_buf[fr_buf_len][sub_num];

  short	noise_cnt;
  long 	lamda;
  short beta;
  short	mu;
  short	far_end_active_th;
  short	far_end_active_th_1;
  short	far_end_active_th_2;
  short far_end_active_1_temp;
  short far_end_active_dis_cnt;
  short far_end_nlp_count;
  short far_end_nlp_count_temp;
  short post_gain;
  short	near_end_active_th;
  short wup_dis_time;
  short wup_en_time;
  short wup_dis_count_clear;
  short wup_en_count_clear;
  short wup_cnt_step;
  short nlp_th;
  short nlp_inc;
  short nlp_dec;
  short nlp_low;
  short alpha_erl;
  short alpha_energy;
  short alpha_energy_comp;
}hs_aecvar_t;


static const short hr_poly[sub_num][hr_poly_len] = {
  {0x0000, 0xfff8, 0x0040, 0xff45, 0x0907, 0xff45, 0x0040, 0xfff8, 0x0000},
  {0x0000, 0xfff8, 0x0036, 0xffa1, 0x08f2, 0xff03, 0x0043, 0xfff9, 0x0000}, 
  {0x0000, 0xfff8, 0x0027, 0x0018, 0x08b2, 0xfedb, 0x0043, 0xfffa, 0x0000}, 
  {0x0000, 0xfff9, 0x0010, 0x00aa, 0x084a, 0xfeca, 0x003e, 0xfffb, 0x0000}, 
  {0x0000, 0xfffc, 0xfff3, 0x0154, 0x07be, 0xfecc, 0x0037, 0xfffc, 0x0000}, 
  {0x0000, 0xffff, 0xffd0, 0x0214, 0x0714, 0xfede, 0x002e, 0xfffd, 0x0000}, 
  {0x0000, 0x0004, 0xffa7, 0x02e4, 0x0651, 0xfefc, 0x0025, 0xfffe, 0x0000}, 
  {0x0000, 0x000b, 0xff7c, 0x03c0, 0x057e, 0xff23, 0x001b, 0xffff, 0x0000}, 
  {0xffff, 0x0012, 0xff4e, 0x04a0, 0x04a0, 0xff4e, 0x0012, 0xffff, 0x0000}, 
  {0xffff, 0x001b, 0xff23, 0x057e, 0x03c0, 0xff7c, 0x000b, 0x0000, 0x0000}, 
  {0xfffe, 0x0025, 0xfefc, 0x0651, 0x02e4, 0xffa7, 0x0004, 0x0000, 0x0000}, 
  {0xfffd, 0x002e, 0xfede, 0x0714, 0x0214, 0xffd0, 0xffff, 0x0000, 0x0000}, 
  {0xfffc, 0x0037, 0xfecc, 0x07be, 0x0154, 0xfff3, 0xfffc, 0x0000, 0x0000}, 
  {0xfffb, 0x003e, 0xfeca, 0x084a, 0x00aa, 0x0010, 0xfff9, 0x0000, 0x0000}, 
  {0xfffa, 0x0043, 0xfedb, 0x08b2, 0x0018, 0x0027, 0xfff8, 0x0000, 0x0000}, 
  {0xfff9, 0x0043, 0xff03, 0x08f2, 0xffa1, 0x0036, 0xfff8, 0x0000, 0x0000},
};

static const short fr_poly[fr_poly_len][sub_num] = {
  {0x0022, 0x0017, 0x001b, 0x001c, 0x0019, 0x0011, 0x0002, 0xffec, 0xffce, 0xffab, 0xff84, 0xff5c, 0xff36, 0xff17, 0xff05, 0xff05}, 
  {0xff1c, 0xff50, 0xffa3, 0x0019, 0x00b1, 0x016a, 0x0241, 0x0330, 0x042f, 0x0534, 0x0635, 0x0727, 0x07ff, 0x08b4, 0x093b, 0x098f}, 
  {0x09ab, 0x098f, 0x093b, 0x08b4, 0x07ff, 0x0727, 0x0635, 0x0534, 0x042f, 0x0330, 0x0241, 0x016a, 0x00b1, 0x0019, 0xffa3, 0xff50}, 
  {0xff1c, 0xff05, 0xff05, 0xff17, 0xff36, 0xff5c, 0xff84, 0xffab, 0xffce, 0xffec, 0x0002, 0x0011, 0x0019, 0x001c, 0x001b, 0x0017}, 
  {0x0022, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
};


static const short w_real[sub_num/2] = {0x7fff, 0x7642, 0x5a82, 0x30fc, 0x0000, 0xcf04, 0xa57e, 0x89be}; //cos(2*pi/N)
static const short w_imag[sub_num/2] = {0x0000, 0x30fc, 0x5a82, 0x7642, 0x7fff, 0x7642, 0x5a82, 0x30fc}; //sin(2*pi/N)

static short peak_table[128] = 
{
  0x0016, 0x0017, 0x0019, 0x001a, 0x001c, 0x001d, 0x001f, 0x0021, 
  0x0023, 0x0025, 0x0027, 0x0029, 0x002c, 0x002e, 0x0031, 0x0034, 
  0x0037, 0x003a, 0x003e, 0x0041, 0x0045, 0x0049, 0x004e, 0x0052, 
  0x0057, 0x005c, 0x0062, 0x0068, 0x006e, 0x0074, 0x007b, 0x0082, 
  0x008a, 0x0092, 0x009b, 0x00a4, 0x00ae, 0x00b8, 0x00c3, 0x00cf, 
  0x00db, 0x00e8, 0x00f6, 0x0104, 0x0114, 0x0124, 0x0135, 0x0148, 
  0x015b, 0x0170, 0x0185, 0x019d, 0x01b5, 0x01cf, 0x01ea, 0x0207, 
  0x0226, 0x0247, 0x0269, 0x028e, 0x02b5, 0x02de, 0x0309, 0x0337, 
  0x0368, 0x039c, 0x03d2, 0x040c, 0x044a, 0x048b, 0x04d0, 0x0519, 
  0x0566, 0x05b8, 0x060e, 0x066a, 0x06cc, 0x0733, 0x07a0, 0x0814, 
  0x088e, 0x0910, 0x0999, 0x0a2b, 0x0ac5, 0x0b68, 0x0c15, 0x0ccd, 
  0x0d8f, 0x0e5d, 0x0f36, 0x101d, 0x1112, 0x1215, 0x1327, 0x1449, 
  0x157d, 0x16c3, 0x181c, 0x198a, 0x1b0d, 0x1ca8, 0x1e5b, 0x2027, 
  0x220f, 0x2413, 0x2637, 0x287a, 0x2ae0, 0x2d6b, 0x301b, 0x32f5, 
  0x35fa, 0x392d, 0x3c90, 0x4027, 0x43f4, 0x47fb, 0x4c3f, 0x50c3, 
  0x558c, 0x5a9e, 0x5ffd, 0x65ad, 0x6bb3, 0x7215, 0x78d7, 0x7fff
};

static short gain_table[128] = 
{
  0x0016, 0x0017, 0x0018, 0x001a, 0x001b, 0x001d, 0x001f, 0x0020, 
  0x0022, 0x0024, 0x0027, 0x0029, 0x002b, 0x002e, 0x0031, 0x0033, 
  0x0036, 0x003a, 0x003d, 0x0041, 0x0045, 0x0049, 0x004d, 0x0052, 
  0x0056, 0x005b, 0x0061, 0x0067, 0x006d, 0x0073, 0x007a, 0x0081, 
  0x0089, 0x0091, 0x009a, 0x00a3, 0x00ac, 0x00b7, 0x00c1, 0x00cd, 
  0x00d9, 0x00e6, 0x00f3, 0x0102, 0x0111, 0x0121, 0x0132, 0x0145, 
  0x0158, 0x016c, 0x0182, 0x0199, 0x01b1, 0x01ca, 0x01e6, 0x0202, 
  0x0221, 0x0241, 0x0263, 0x0288, 0x02ae, 0x02d7, 0x0302, 0x032f, 
  0x0360, 0x0393, 0x03c9, 0x0402, 0x043f, 0x0480, 0x04c4, 0x050c, 
  0x0559, 0x05aa, 0x0600, 0x065b, 0x06bb, 0x0721, 0x078d, 0x0800, 
  0x0879, 0x08fa, 0x0982, 0x0a12, 0x0aab, 0x0b4d, 0x0bf8, 0x0cae, 
  0x0d6e, 0x0e3a, 0x0f12, 0x0ff6, 0x10e8, 0x11e9, 0x12f9, 0x1418, 
  0x1549, 0x168c, 0x17e2, 0x194c, 0x1acc, 0x1c63, 0x1e11, 0x1fd9, 
  0x21bc, 0x23bc, 0x25da, 0x2818, 0x2a79, 0x2cfd, 0x2fa7, 0x327a, 
  0x3578, 0x38a3, 0x3bfe, 0x3f8c, 0x4350, 0x474d, 0x4b86, 0x5000, 
  0x54be, 0x59c3, 0x5f15, 0x64b7, 0x6aaf, 0x7101, 0x77b3, 0x7ecb
};

static long peak=0;
static long gain=0x7fff;
static hs_aecvar_t *g_pstAec;
static hs_aec_eq_t *g_pstAecEq;
static hs_aec_var_t g_stAecInfo;


long _aec_drcLimit (unsigned short rt, unsigned short at, unsigned short rt1, unsigned short at1, short xin, short LT, short NT)
{
	long peak_diff;
	short peak_index;
	short peak_low, peak_high;
	unsigned short gain_k;
	short gain_index;

	int i;

	/////////////////////////////////////////////
	//peak computation
	/////////////////////////////////////////////
	peak_diff = (long)abs(xin)-peak;

	if(peak_diff<0)
	{
		peak_diff=0;
	}

	peak = ((long)(at*peak_diff) + (long)(peak<<16) - (long)(rt*peak))>>16;

	/////////////////////////////////////////////
	//lookup table
	/////////////////////////////////////////////
	peak_low =0;
	peak_high =127;
	peak_index=63;

	for (i=0;i<7;i++)
	{
		if(peak_table[peak_index]<peak)
		{
			peak_low = peak_index;
			peak_index = (peak_index+peak_high)>>1;
		}
		else
		{
			peak_high = peak_index;
			peak_index = (peak_index+peak_low)>>1;
		}
	}

	/////////////////////////////////////////////
	//gain computation
	/////////////////////////////////////////////
	if(peak_index<LT && peak_index>NT)
	{
		if((LT-peak_index)>48)
		{
			gain_index = 127;
		}
		else
		{
			gain_index = 79+(LT-peak_index);
		}
	}
	else if(peak_index>=LT)
	{
		if(peak_index-LT>79)
			gain_index = 0;
		else
			gain_index = 79-(peak_index-LT);
	}
	else
    gain_index = 79-12;
		
	if(gain<gain_table[gain_index])
		gain_k=rt1;
	else
		gain_k=at1;

	gain = ((long)(gain<<16) - (long)(gain_k*gain) + (long)(gain_k*gain_table[gain_index]))>>16;

	return gain;
}

static int _aec_eqFirstOrderHigh (int16_t G, int16_t B0, int32_t *y1, int32_t x0, int32_t *x1)
{
	int32_t y0;  //Q23
	int32_t out; //Q21
	int64_t tmp0; //Q37
	int32_t tmp;

	tmp0 = 0;
	tmp0 += ((int64_t)B0 * (x0<<2))>>11;
	tmp0 -= ((int64_t)B0 * (*y1))>>11;
	tmp0 += (int64_t)((*x1)<<5);


	tmp = (int)(tmp0>>3);

	if ( tmp > Q23P ) y0 = 0x003fffff;
	else if ( tmp < Q23N ) y0 = 0xffc00000;
	else y0 = tmp;

	out = (x0<<2) - y0;

	tmp0 = (int64_t)G*out;

	tmp = (int)(tmp0>>14);

	if ( tmp > Q23P ) out = 0x003fffff;
	else if ( tmp < Q23N ) out = 0xffc00000;
	else out = tmp;

	out = (x0<<2) + out;

	out = out>>2;

	*x1 = x0;
	*y1 = y0;

	return out;	
}

static int _aec_eqFirstOrderLow (int16_t G, int16_t B0, int32_t *y1, int32_t x0, int32_t *x1)
{
	int32_t y0;  //Q23
	int32_t out; //Q21
	int64_t tmp0; //Q37
	int32_t tmp;

	tmp0 = 0;
	tmp0 += ((int64_t)B0 * (x0<<2))>>11;
	tmp0 -= ((int64_t)B0 * (*y1))>>11;
	tmp0 += (int64_t)((*x1)<<5);


	tmp = (int)(tmp0>>3);

	if ( tmp > Q23P ) y0 = 0x003fffff;
	else if ( tmp < Q23N ) y0 = 0xffc00000;
	else y0 = tmp;

	out = (x0<<2) + y0;

	tmp0 = (int64_t)G*out;

	tmp = (int)(tmp0>>14);

	if ( tmp > Q23P ) out = 0x003fffff;
	else if ( tmp < Q23N ) out = 0xffc00000;
	else out = tmp;

	out = (x0<<2) + out;

	out = out>>2;

	*x1 = x0;
	*y1 = y0;

	return out;	
}

static int16_t _aec_calEq(int16_t data)
{
  int32_t xin, dn_out;
  int16_t xout_scale;
  int32_t *X1, *Y1;

  if((g_pstAecEq->s16B1G == 0) && (g_pstAecEq->s16B5G == 0))
    return data;

  xin = data;
  xin = (xin)<<12;
  xin = (xin)>>10;

  X1 = &g_pstAecEq->s32B1X1;
  Y1 = &g_pstAecEq->s32B1Y1;
  dn_out = _aec_eqFirstOrderLow(g_pstAecEq->s16B1G, g_pstAecEq->s16B1A, Y1, xin, X1);

  X1=&g_pstAecEq->s32B5X1;
  Y1=&g_pstAecEq->s32B5Y1;
  dn_out = _aec_eqFirstOrderHigh(g_pstAecEq->s16B5G, g_pstAecEq->s16B5A, Y1, dn_out, X1);

  xout_scale = (int16_t)(dn_out>>2);

  return xout_scale;
}


static void _aec_eqInit(void)
{

  if(!g_pstAecEq)
    return ;
  /*
   * sample rate 8khz
   * < 200hz  +20db
   * > 2000hz -10db
   */

  g_pstAecEq->s16B1A  = g_stAecInfo.s16EqB1A; //(short)0xc957;	/* 200Hz */
  g_pstAecEq->s32B1X1 = 0;
  g_pstAecEq->s32B1Y1 = 0;

  g_pstAecEq->s16B5A  = g_stAecInfo.s16EqB5A; //(short)0xdec1;	/* 2000Hz */
  g_pstAecEq->s32B5X1 = 0;
  g_pstAecEq->s32B5Y1 = 0;

  g_pstAecEq->s16B1G  = g_stAecInfo.s16EqB1Gain; //(short)0x4531;  /* +10dB */
  g_pstAecEq->s16B5G  = g_stAecInfo.s16EqB5Gain; //(short)0xea1f;  /* -10dB */
}

void _aec_ditFFT(short N, short inverse, short imag_0, short m, short *yout_real, short *yout_imag)
{
	short	LH;
	short nm;
	short i, j, k;
	short t;

	short le;
	short L;
	short B;
	short p;
	short ip;

	short t_real;
	short t_imag;
	short w_imag_t;
	long temp;

	LH = N>>1;
	nm = N - 2;                   
	j = LH;

	if(imag_0==0)
	{
		for(i=1;i<=nm;i++)
	{
		if(i<j)
		{
			t = yout_real[j];
			yout_real[j] = yout_real[i];
			yout_real[i] = t;
			//if(imag_0==0)
			//{
				t = yout_imag[j];
				yout_imag[j] = yout_imag[i];
				yout_imag[i] = t;
			//}
		}
		k = LH;
		while(j>=k)
		{
			j=j-k;
			k=k/2;
		}
		j=j+k;
	}
	}
	else
	{
	for(i=1;i<=nm;i++)
	{
		if(i<j)
		{
			t = yout_real[j];
			yout_real[j] = yout_real[i];
			yout_real[i] = t;
			/*
			if(imag_0==0)
			{
				t = yout_imag[j];
				yout_imag[j] = yout_imag[i];
				yout_imag[i] = t;
			}
			*/
		}
		k = LH;
		while(j>=k)
		{
			j=j-k;
			k=k/2;
		}
		j=j+k;
	}
	}

	if(inverse==1)
	{
	for (L=0;L<=m-1;L++)
	{
		le = 1<<(L+1);
		B = le>>1;

		for(j=0;j<=(B-1);j++)
		{
			p=(1<<(m-L-1))*j;

			//if(inverse==1)
			//{
				w_imag_t = w_imag[p];
			/*
			}
			else
			{
				w_imag_t = -w_imag[p];
			}
			*/
 
			for (i=j;i<=N-1;i=i+le)
			{
				ip = i+B;

				temp = (long)yout_real[ip]*(long)w_real[p];
				temp = temp - (long)yout_imag[ip]*(long)w_imag_t;
				t_real = (short)(temp>>15);

				temp = (long)yout_real[ip]*(long)w_imag_t;
				temp = temp + (long)yout_imag[ip]*(long)w_real[p];
				t_imag = (short)(temp>>15);

		
				yout_real[ip]=yout_real[i]-t_real;
				yout_imag[ip]=yout_imag[i]-t_imag;
				yout_real[i]=yout_real[i]+t_real;
				yout_imag[i]=yout_imag[i]+t_imag;
			}
		}
	}
	}
	else
	{
	for (L=0;L<=m-1;L++)
	{
		le = 1<<(L+1);
		B = le>>1;

		for(j=0;j<=(B-1);j++)
		{
			p=(1<<(m-L-1))*j;

			/*
			if(inverse==1)
			{
				w_imag_t = w_imag[p];
			}
			else
			{
			*/
				w_imag_t = -w_imag[p];
			//}
 
			for (i=j;i<=N-1;i=i+le)
			{
				ip = i+B;

				temp = (long)yout_real[ip]*(long)w_real[p];
				temp = temp - (long)yout_imag[ip]*(long)w_imag_t;
				t_real = (short)(temp>>15);

				temp = (long)yout_real[ip]*(long)w_imag_t;
				temp = temp + (long)yout_imag[ip]*(long)w_real[p];
				t_imag = (short)(temp>>15);

		
				yout_real[ip]=yout_real[i]-t_real;
				yout_imag[ip]=yout_imag[i]-t_imag;
				yout_real[i]=yout_real[i]+t_real;
				yout_imag[i]=yout_imag[i]+t_imag;
			}
		}
	}
	}
}

static long _aec_process(short xin, short din)
  {
    short temp_16_1;
    short temp_16_2;
    short temp_16_3;
    long temp_32_1;
    long temp_32_2;
    long temp_32_3;
    long long temp_64_1;
    long long temp_64_2;
  
    unsigned short temp_16u_1;
    //unsigned short temp_16u_2;
    unsigned long temp_32u_1;
    unsigned long temp_32u_2;
    //unsigned long temp_32u_3;
    unsigned long long temp_64u_1;
    unsigned long long temp_64u_2;
  
    short x_sub_far_real[sub_num];
    short x_sub_far_imag[sub_num];
    short x_sub_near_real[sub_num];
    short x_sub_near_imag[sub_num];
    //short nlp_out_real[sub_num_half];
    //short nlp_out_imag[sub_num_half];
    short y_sub_real[sub_num];
    short y_sub_imag[sub_num];
  
    //short k,j,m;
    long  k,j,m;
  
    long denomiFixed;
    long long denomiMult_1;
    long temp_denomi;
    short temp_mu_real;
    short temp_mu_imag;
    long  temp_mu1_real;
    long  temp_mu1_imag;
  
    //short noise_flag;
    short double_talk_active;
    short far_end_active;
    short far_end_active_1;
    short near_end_active;
    short residue_is_small;
  
    long yout;
  
    g_pstAec->hr_buf_far[g_pstAec->hr_buf_pt]=xin;
    g_pstAec->hr_buf_near[g_pstAec->hr_buf_pt]=din;
  
    //if((fr_buf_index_pt%down_sample)==0)
    if(g_pstAec->fr_buf_index_pt==0)
    {
      ///////////////////////////////////////////////////////////////////////////
      //  analysis 
      ///////////////////////////////////////////////////////////////////////////
      for(k=0; k<sub_num; k++)
      {
        temp_32_1 = 0;
        temp_32_2 = 0;
        //temp_16u_1 = k+hr_buf_pt;
        temp_32_3 = k+g_pstAec->hr_buf_pt;
        for(j=0; j<hr_poly_len; j++)
        {
          /*
          if(temp_16u_1>=hr_buf_len)
          {
            temp_16u_1 = temp_16u_1 - hr_buf_len;
          }
          */
          if(temp_32_3>=hr_buf_len)
          {
            temp_32_3 = temp_32_3 - hr_buf_len;
          }
          //temp_16u_1 = (temp_16u_1%hr_buf_len);
          
          /*
          if(j>=hr_poly_len_half)
          {
            if(j==(hr_poly_len-1))
            {
              temp_16_1 = 0;
            }
            else
            {
              if(k==0)
              {
                temp_16_1 = hr_poly[hr_poly_len-1-j][k];
              }
              else
              {
                temp_16_1 = hr_poly[hr_poly_len-2-j][sub_num-k];
              }
            }
          }
          else
          {
            temp_16_1 = hr_poly[j][k];
          }
          */
        
        /*
          temp_16_1 = hr_poly[j][k];
  
          temp_32_1 = hr_buf_far[temp_16u_1]*temp_16_1 + temp_32_1;
          temp_32_2 = hr_buf_near[temp_16u_1]*temp_16_1 + temp_32_2;
          */
  
          temp_32_1 = (long)g_pstAec->hr_buf_far[temp_32_3]*(long)hr_poly[k][j] + temp_32_1;
          temp_32_2 = (long)g_pstAec->hr_buf_near[temp_32_3]*(long)hr_poly[k][j] + temp_32_2;
  
          temp_32_3 = temp_32_3 + sub_num;
        }
        x_sub_far_real[k] = (short)(temp_32_1>>15);
        x_sub_near_real[k] = (short)(temp_32_2>>15);
  
        x_sub_far_imag[k] = 0;
        x_sub_near_imag[k] = 0;
      }
      _aec_ditFFT(sub_num, 1, 1, log_2_sub_num, x_sub_far_real, x_sub_far_imag);
      _aec_ditFFT(sub_num, 1, 1, log_2_sub_num, x_sub_near_real, x_sub_near_imag); 
      ///////////////////////////////////////////////////////////////////////////
      //  analysis end
      ///////////////////////////////////////////////////////////////////////////
  
      ///////////////////////////////////////////////////////////////////////////
      //  adaptive filter
      ///////////////////////////////////////////////////////////////////////////
  
      ///////////////////////////////////////////////////////////////////////////
      //  adaptive filter end
      ///////////////////////////////////////////////////////////////////////////
      for(k=0; k<sub_num_half; k++)
      {
          //temp_16_1 = x_sub_near_real[k];
          //temp_32_1 = temp_16_1*temp_16_1;
          //temp_16_1 = x_sub_near_imag[k];
          //temp_32_1 = temp_32_1 + temp_16_1*temp_16_1;
  
          temp_32_1 =  (x_sub_near_real[k]* x_sub_near_real[k] + x_sub_near_imag[k]*x_sub_near_imag[k])>>8;
          temp_64_1 = (long long)(0x00008000-(long)g_pstAec->alpha_energy)*(long long)temp_32_1;
          temp_64_1 = temp_64_1 + (long long)g_pstAec->Enear[k]*(long long)g_pstAec->alpha_energy;
          g_pstAec->Enear[k] = (long)(temp_64_1>>15);
  
          /*
          temp_32_1 = Enear[k] + (temp_32_1>>8);  
          temp_32_2 = near_fifo_real[fifo_pt][k]*near_fifo_real[fifo_pt][k]+near_fifo_imag[fifo_pt][k]*near_fifo_imag[fifo_pt][k];
          Enear[k] = temp_32_1 - (temp_32_2>>8);
          */
          //temp_32_1 = x_sub_near_real[k]*x_sub_near_real[k]+x_sub_near_imag[k]*x_sub_near_imag[k];
          //temp_32_1 = Enear[k] + (temp_32_1>>8);  
          //temp_32_2 = near_fifo_real[fifo_pt][k]*near_fifo_real[fifo_pt][k]+near_fifo_imag[fifo_pt][k]*near_fifo_imag[fifo_pt][k];
          //temp_32_1 = far_echo_fifo_real[fifo_pt][k]*far_echo_fifo_real[fifo_pt][k]+far_echo_fifo_imag[fifo_pt][k]*far_echo_fifo_imag[fifo_pt][k];
          //Efar_echo[k] = Efar_echo[k] - (temp_32_1>>8);
          //temp_16_1 = near_fifo_real[fifo_pt][k]-far_echo_fifo_real[fifo_pt][k];
          //temp_16_2 = near_fifo_imag[fifo_pt][k]-far_echo_fifo_imag[fifo_pt][k];
          //temp_32_1 = temp_16_1*temp_16_1 + temp_16_2*temp_16_2;
          //Eres[k] = Eres[k] - (temp_32_1>>8);
  
          temp_32_2 = (x_sub_far_real[k]*x_sub_far_real[k]+x_sub_far_imag[k]*x_sub_far_imag[k])>>8;
          g_pstAec->far_energy[k] = g_pstAec->far_energy[k] + temp_32_2;
          temp_64_1 = (long long)g_pstAec->Efar[k]*(long long)g_pstAec->alpha_energy;
          temp_64_1 = temp_64_1 + (long long)(0x00008000-(long)g_pstAec->alpha_energy)*(long long)temp_32_2;
          g_pstAec->Efar[k] = (long)(temp_64_1>>15);
  
          //Efar[k] = Efar[k] + (temp_32_2>>8);
          //far_energy[k] = far_energy[k] + (temp_32_2>>8);
          //temp_32_1 = far_fifo_real[fifo_pt][k]*far_fifo_real[fifo_pt][k]+far_fifo_imag[fifo_pt][k]*far_fifo_imag[fifo_pt][k];
          //Efar[k] = Efar[k] - (temp_32_1>>8);
  
          temp_16_3 = (short)(g_pstAec->fifo_pt + (long)g_pstAec->w_filter_len[k]);
          //if(temp_16_3>=sub_buf_len)
          if(temp_16_3>=sub_filter_max_len)
          {
            //temp_16_3 = temp_16_3 - sub_buf_len;
            temp_16_3 = temp_16_3 - sub_filter_max_len;
          }
          temp_32_1 = (g_pstAec->far_fifo_real[temp_16_3][k]*g_pstAec->far_fifo_real[temp_16_3][k]+g_pstAec->far_fifo_imag[temp_16_3][k]*g_pstAec->far_fifo_imag[temp_16_3][k])>>8;
          g_pstAec->far_energy[k] = g_pstAec->far_energy[k] - temp_32_1;
  
          temp_16_1 = g_pstAec->near_fifo_real[temp_16_3][k] - g_pstAec->far_echo_fifo_real[temp_16_3][k];
          temp_16_2 = g_pstAec->near_fifo_imag[temp_16_3][k] - g_pstAec->far_echo_fifo_imag[temp_16_3][k];
          temp_32_1 = (temp_16_1*temp_16_1 + temp_16_2*temp_16_2);
          g_pstAec->res_energy[k] = g_pstAec->res_energy[k] - (temp_32_1>>8);
  
          g_pstAec->near_fifo_real[g_pstAec->fifo_pt][k] = x_sub_near_real[k];
          g_pstAec->near_fifo_imag[g_pstAec->fifo_pt][k] = x_sub_near_imag[k];
          g_pstAec->far_fifo_real[g_pstAec->fifo_pt][k]  = x_sub_far_real[k];
          g_pstAec->far_fifo_imag[g_pstAec->fifo_pt][k]  = x_sub_far_imag[k];
  
          temp_64_1 = 0;
          temp_64_2 = 0;
  
        if(k==0 || k==(sub_num_half-1))
        {
          for(j=0; j<g_pstAec->w_filter_len[k]; j++)
          {
            //temp_16u_1 = (fifo_pt + j)%sub_buf_len;
            //temp_16u_1 = (fifo_pt + j) & (0x007f);
            //temp_16u_1 = (fifo_pt + j);
            temp_32_1 = (g_pstAec->fifo_pt+j);
            temp_32_1 = temp_32_1 & (0x0000001f);
            temp_64_1 = temp_64_1 + (long long)g_pstAec->w_filter_real[j][k]*(long long)g_pstAec->far_fifo_real[temp_32_1][k];
          }
        }
        else
        {
          for(j=0; j<g_pstAec->w_filter_len[k]; j++)
          {
            //temp_16u_1 = (fifo_pt + j)%sub_buf_len;
            //temp_16u_1 = (fifo_pt + j) & (0x007f);
            //temp_32_3 = (fifo_pt+j) & (0x0000007f);
            temp_32_3 = (g_pstAec->fifo_pt+j);
            temp_32_3 = temp_32_3 & (0x0000001f);
  
            temp_32_1 = g_pstAec->w_filter_real[j][k];
            temp_16_1 = g_pstAec->far_fifo_real[temp_32_3][k];
            temp_64_1 = temp_64_1 + (long long)temp_32_1*(long long)temp_16_1;
            temp_32_1 = g_pstAec->w_filter_imag[j][k];
            temp_16_1 = g_pstAec->far_fifo_imag[temp_32_3][k];
            temp_64_1 = temp_64_1 + (long long)temp_32_1*(long long)temp_16_1;
  
            temp_32_1 = g_pstAec->w_filter_real[j][k];
            temp_16_1 = g_pstAec->far_fifo_imag[temp_32_3][k];
            temp_64_2 = temp_64_2 + (long long)temp_32_1*(long long)temp_16_1;
            temp_32_1 = g_pstAec->w_filter_imag[j][k];
            temp_16_1 = g_pstAec->far_fifo_real[temp_32_3][k];
            temp_64_2 = temp_64_2 - (long long)temp_32_1*(long long)temp_16_1;
            //temp_64_1 = temp_64_1 + (long long)w_filter_real[j][k]*far_fifo_real[temp_16u_1][k] + (long long)w_filter_imag[j][k]*far_fifo_imag[temp_16u_1][k];
            //temp_64_2 = temp_64_2 + (long long)w_filter_real[j][k]*far_fifo_imag[temp_16u_1][k] - (long long)w_filter_imag[j][k]*far_fifo_real[temp_16u_1][k];
          }
        }
  
          g_pstAec->far_echo_fifo_real[g_pstAec->fifo_pt][k] = (short)(temp_64_1>>25);
          g_pstAec->far_echo_fifo_imag[g_pstAec->fifo_pt][k] = (short)(temp_64_2>>25);
  
          temp_32_1 = (g_pstAec->far_echo_fifo_real[g_pstAec->fifo_pt][k]*g_pstAec->far_echo_fifo_real[g_pstAec->fifo_pt][k] + g_pstAec->far_echo_fifo_imag[g_pstAec->fifo_pt][k]*g_pstAec->far_echo_fifo_imag[g_pstAec->fifo_pt][k]);
          //Efar_echo[k] = Efar_echo[k] + (temp_32_1>>8);
          temp_32_1 = temp_32_1>>8;
          temp_64_1 = (long long)g_pstAec->Efar_echo[k]*(long long)g_pstAec->alpha_energy;
          temp_64_1 = temp_64_1 + (long long)(0x00008000-(long)g_pstAec->alpha_energy)*(long long)temp_32_1;
          g_pstAec->Efar_echo[k] = (long)(temp_64_1>>15);
  
          g_pstAec->residue_real[k] = g_pstAec->near_fifo_real[g_pstAec->fifo_pt][k]-g_pstAec->far_echo_fifo_real[g_pstAec->fifo_pt][k];
          g_pstAec->residue_imag[k] = g_pstAec->near_fifo_imag[g_pstAec->fifo_pt][k]-g_pstAec->far_echo_fifo_imag[g_pstAec->fifo_pt][k];
          temp_32_2 = (g_pstAec->residue_real[k]*g_pstAec->residue_real[k] + g_pstAec->residue_imag[k]*g_pstAec->residue_imag[k]);
          temp_32_2 = temp_32_2>>8;
  
          //temp_16_1 = near_fifo_real[temp_16_3][k] - far_echo_fifo_real[temp_16_3][k];
          //temp_16_2 = near_fifo_imag[temp_16_3][k] - far_echo_fifo_imag[temp_16_3][k];
          //temp_32_1 = temp_16_1*temp_16_1 + temp_16_2*temp_16_2;
          
          g_pstAec->res_energy[k] = g_pstAec->res_energy[k] + temp_32_2;
          //Eres[k] = Eres[k] + (temp_32_2>>8);
          
          temp_64_1 = (long long)g_pstAec->Eres[k]*(long long)g_pstAec->alpha_energy;
          temp_64_1 = temp_64_1 + (long long)(0x00008000-(long)g_pstAec->alpha_energy)*(long long)temp_32_2;
          g_pstAec->Eres[k] = (long)(temp_64_1>>15);
        
  
        //w_update[k]=1;
        if((g_pstAec->w_update[k]==1) && ((g_pstAec->fifo_pt%update_rate)==0))
        //if((w_update[k]==1))
        {
          denomiFixed = (g_pstAec->lamda + g_pstAec->far_energy[k] + g_pstAec->beta*g_pstAec->res_energy[k]);  // Q22
          denomiMult_1 = 1;
          if(denomiFixed <= 0x10000)
          {
            //temp_denomi = 0xffffffff;
            temp_denomi = 0x7fffffff;
          }
          else
          {
            //temp_denomi = (unsigned long)((denomiMult_1<<(30+22)) / denomiFixed);
            temp_denomi = (long)((denomiMult_1<<(25+22)) / denomiFixed);
            //temp_denomi = (long)((denomiMult_1<<(29+22)) / denomiFixed);
          }
  
          temp_mu_real = (short)((g_pstAec->mu*g_pstAec->residue_real[k])>>15);
          temp_mu_imag = (short)((-g_pstAec->mu*g_pstAec->residue_imag[k])>>15);
  
          //temp_mu_real = residue_real[k];
          //temp_mu_imag = residue_imag[k];
  
          temp_16_3 = g_pstAec->w_filter_len[k];
  
          if(k==0 || k==(sub_num_half-1))
          {
            for(j=0; j<temp_16_3; j++)
            {
              //temp_16u_1 = (fifo_pt + j)%sub_buf_len;
              //temp_16u_1 = (fifo_pt + j) & (0x007f);
              temp_32_1 = (g_pstAec->fifo_pt + j) & (0x0000001f);
  
              temp_mu1_real = (long)temp_mu_real*(long)g_pstAec->far_fifo_real[temp_32_1][k];
              //temp_mu1_real = residue_real[k]*far_fifo_real[temp_16u_1][k];
              temp_64_1 = (long long)temp_mu1_real*(long long)temp_denomi;
              //w_filter_real[j][k] = w_filter_real[j][k] + (long)(temp_64_1>>34);
              g_pstAec->w_filter_real[j][k] = g_pstAec->w_filter_real[j][k] + (long)(temp_64_1>>30);
            }
          }
          else
          {
            for(j=0; j<temp_16_3; j++)
            { 
              //temp_16u_1 = (fifo_pt + j)%sub_buf_len;
              //temp_16u_1 = (fifo_pt + j) & (0x007f);
              temp_32_1 = (g_pstAec->fifo_pt + j) & (0x0000001f);
  
              temp_mu1_real = (long)temp_mu_real*(long)g_pstAec->far_fifo_real[temp_32_1][k] - (long)temp_mu_imag*(long)g_pstAec->far_fifo_imag[temp_32_1][k];
              temp_mu1_imag = (long)temp_mu_imag*(long)g_pstAec->far_fifo_real[temp_32_1][k] + (long)temp_mu_real*(long)g_pstAec->far_fifo_imag[temp_32_1][k];
              //temp_mu1_real = residue_real[k]*far_fifo_real[temp_16u_1][k] - residue_imag[k]*far_fifo_imag[temp_16u_1][k];
              //temp_mu1_imag = residue_imag[k]*far_fifo_real[temp_16u_1][k] + residue_real[k]*far_fifo_imag[temp_16u_1][k];
              temp_64_1 = (long long)temp_mu1_real*(long long)temp_denomi;
              //w_filter_real[j][k] = w_filter_real[j][k] + (long)(temp_64_1>>34);
              g_pstAec->w_filter_real[j][k] = g_pstAec->w_filter_real[j][k] + (long)(temp_64_1>>30);
              temp_64_1 = (long long)temp_mu1_imag*(long long)temp_denomi;
              //w_filter_imag[j][k] = w_filter_imag[j][k] + (long)(temp_64_1>>34);
              g_pstAec->w_filter_imag[j][k] = g_pstAec->w_filter_imag[j][k] + (long)(temp_64_1>>30);
            }
          }
        }
      }
  
      for(k=0; k<sub_num_half; k++)
      {
        if(g_pstAec->Efar[k]<g_pstAec->Pmin_far[k])
        {
          g_pstAec->Pmin_far[k] = g_pstAec->Efar[k];
        }
      }
  
      if(g_pstAec->noise_cnt == noise_winLen)
      //if(noise_cnt == 0)
      {
        for(k=0; k<sub_num_half; k++)
        { 
          for(m=0; m<(noise_winNo-1); m++)
          {
            g_pstAec->PLmin_far[m][k] = g_pstAec->PLmin_far[m+1][k];
          }
          g_pstAec->PLmin_far[noise_winNo-1][k] = g_pstAec->Pmin_far[k];
  
          /*
          noise_flag = 1;
          for(m=0; m<(noise_winNo-1); m++)
          {
            if(PLmin_far[m][k]>PLmin_far[m+1][k])
            { 
              noise_flag = 0;
              break;
            }
          }
          if(noise_flag==1)
          {
            for(m=0; m<(noise_winNo-1); m++)
            {
              PLmin_far[m][k] = Pmin_far[k];
            }
            Pn_far[k] = Pmin_far[k];
          }
          else
          {*/
            temp_32u_1 = g_pstAec->PLmin_far[0][k];
            for(m=1; m<noise_winNo; m++)
            {
              if(temp_32u_1>g_pstAec->PLmin_far[m][k])
              {
                temp_32u_1 = g_pstAec->PLmin_far[m][k];
              }
            }
            g_pstAec->Pn_far[k] = temp_32u_1;
          //}
          g_pstAec->Pmin_far[k] = Pmax;
        }
      }
  
      for(k=1; k<sub_num_half; k++)
      { 
  
        if(k==1)
        {
        temp_32_1 = g_pstAec->Efar_echo[k];
        temp_64_1 = (long long)temp_32_1*(long long)temp_32_1;
        //temp_32_1 = Efar[k];
        temp_64_1 = temp_64_1 + (long long)g_pstAec->Efar[k]*(long long)g_pstAec->Efar[k];
        //temp_64u_1 = (unsigned long long)Efar[k]*Efar[k]+(unsigned long long)Efar_echo[k]*Efar_echo[k];
        temp_64u_2 = (unsigned long long)g_pstAec->Efar[k]*(unsigned long long)g_pstAec->Eres[k];
  
        //if(Efar[k]*Eres[k]>(Efar[k]*Efar[k]+Efar_echo[k]*Efar_echo[k]))
        if(temp_64u_2 > (unsigned long long)temp_64_1)
        {
          double_talk_active = 1;
        }
        else
        {
          double_talk_active = 0;
        }
  
        temp_32u_1 = ((g_pstAec->far_end_active_th * g_pstAec->Pn_far[k])>>4);
  
        if(g_pstAec->Efar[k]>temp_32u_1)
        {
          far_end_active = 1;
          //if(Efar[k]==0)
          //{
          //  temp_16u_1 = 0x8000;
          //}
          //else
          //{
            //temp_32u_2 = (unsigned long)(((unsigned long long)Efar_echo[k]<<15)/Efar[k]);
            //if(temp_32u_2>0x8000)
          /*
          temp_32u_1 = Efar_echo[k];
          temp_32u_2 = Efar[k];
          if(temp_32u_1>=temp_32u_2)
          {
            temp_16u_1 = 0x8000;
          }
          else
          {
            temp_16u_1 = (unsigned short)(((unsigned long long)temp_32u_1<<15)/temp_32u_2);
              //temp_16u_1 = (unsigned short)(temp_32u_2);
          }
          //}
          erl[k] =  (unsigned short)((erl[k]*alpha_erl + (0x8000-alpha_erl)*temp_16u_1)>>15);
          //temp_32u_1 = erl[k]*alpha_erl + (0x8000-alpha_erl)*temp_16u_1;
          //erl[k] = (unsigned short)(temp_32u_1>>15);
          */
        }
        else
        {
          far_end_active = 0;
        }
  
        temp_32u_1 = g_pstAec->far_end_active_th_1;
        if(g_pstAec->Efar[k]>temp_32u_1)
        {
          far_end_active_1 = 1;
        }
        else
        {
          far_end_active_1 = 0;
        }
  
        }
      }

      for(k=0; k<sub_num_half; k++)
      {
  
        if(far_end_active==1)
        {
          temp_32u_1 = g_pstAec->Efar_echo[k];
          temp_32u_2 = g_pstAec->Efar[k];
          if(temp_32u_1>=temp_32u_2)
          {
            temp_16u_1 = 0x8000;
          }
          else
          {
            temp_16u_1 = (unsigned short)(((unsigned long long)temp_32u_1<<15)/temp_32u_2);
              //temp_16u_1 = (unsigned short)(temp_32u_2);
          }
          //}
          g_pstAec->erl[k] =  (unsigned short)((g_pstAec->erl[k]*g_pstAec->alpha_erl + (0x8000-g_pstAec->alpha_erl)*temp_16u_1)>>15);
        }
  
        if((double_talk_active==1) || (far_end_active==0))
        {
          if(g_pstAec->wup_en_count[k]>g_pstAec->wup_dis_count_clear)
          {
            g_pstAec->wup_dis_count[k] = 0;
          }
          if(g_pstAec->wup_en_count[k]<g_pstAec->wup_en_time)
          {
            g_pstAec->wup_en_count[k] = g_pstAec->wup_en_count[k] + g_pstAec->wup_cnt_step;
          }
          else
          {
            g_pstAec->w_update[k]=0;
          }
        }
        else
        {
          if( g_pstAec->wup_dis_count[k] > g_pstAec->wup_en_count_clear )
          {
            g_pstAec->wup_en_count[k] = 0;
          }
          if( g_pstAec->wup_dis_count[k] < g_pstAec->wup_dis_time )
          {
            g_pstAec->wup_dis_count[k] = g_pstAec->wup_dis_count[k] + g_pstAec->wup_cnt_step;
          }
          else
          {
            g_pstAec->w_update[k] = 1;
          }
        }
      }

	  /*
	  for(k=1; k<sub_num_half; k++)
	  {
		  g_pstAec->w_update[k] = g_pstAec->w_update[0];
	  }
	  */
  
      //noise_cnt = (noise_cnt+1) % noise_winLen;
  
      if(g_pstAec->noise_cnt==noise_winLen)
      {
        g_pstAec->noise_cnt = 1;
        //noise_cnt = 0;
      }
      else
      {
        g_pstAec->noise_cnt = g_pstAec->noise_cnt + 1;
      }
  
      //fifo_pt = (fifo_pt + sub_buf_len-1)%sub_buf_len;
      //fifo_pt = (fifo_pt + sub_filter_max_len-1) & 0x0000001f;

	  near_end_active = 1;
	  residue_is_small = 0;


      for(k=1; k<2; k++)
      {
        temp_32u_1 = ((unsigned long)g_pstAec->near_end_active_th*(unsigned long)g_pstAec->erl[k])>>4;
        temp_64u_1 = (unsigned long long)temp_32u_1*(unsigned long long)g_pstAec->Efar[k];
        temp_32u_2 = (unsigned long)(temp_64u_1>>15);
        if(g_pstAec->Enear[k]<temp_32u_2)
        {
          near_end_active = 0;
        }
        else
        {
          //near_end_active = 1;
        }
  
        temp_64u_1 = (unsigned long long)g_pstAec->Enear[k]*(unsigned long long)g_pstAec->nlp_th;
        temp_32u_1 = (unsigned long)(temp_64u_1>>15);
        if(g_pstAec->Eres[k]<temp_32u_1)
        {
          residue_is_small = 1;
        }
        else
        {
          //residue_is_small = 0;
        }
	  }
   
      for(k=0; k<sub_num_half; k++)
      {
		  /*
        temp_32u_1 = ((unsigned long)g_pstAec->near_end_active_th*(unsigned long)g_pstAec->erl[k])>>4;
        temp_64u_1 = (unsigned long long)temp_32u_1*(unsigned long long)g_pstAec->Efar[k];
        temp_32u_2 = (unsigned long)(temp_64u_1>>15);
        if(g_pstAec->Enear[k]<temp_32u_2)
        {
          near_end_active = 0;
        }
        else
        {
          near_end_active = 1;
        }
  
        temp_64u_1 = (unsigned long long)g_pstAec->Enear[k]*(unsigned long long)g_pstAec->nlp_th;
        temp_32u_1 = (unsigned long)(temp_64u_1>>15);
        if(g_pstAec->Eres[k]<temp_32u_1)
        {
          residue_is_small = 1;
        }
        else
        {
          residue_is_small = 0;
        }
		*/
  
        if((residue_is_small==1) || (near_end_active==0))
        {
          //if(nlp_count[k]>nlp_low)
          if(g_pstAec->nlp_count[k]>(g_pstAec->nlp_low+g_pstAec->nlp_dec))
          {
            g_pstAec->nlp_count[k] = g_pstAec->nlp_count[k] - g_pstAec->nlp_dec;
          }
          else
          {
            g_pstAec->nlp_count[k] = g_pstAec->nlp_low;
          }
        }
        else
        {
          g_pstAec->nlp_count[k] = g_pstAec->nlp_count[k] + g_pstAec->nlp_inc;
          if(g_pstAec->nlp_count[k]>nlp_max)
          {
            g_pstAec->nlp_count[k] = nlp_max;
          }
        }
	  }
  
        //temp_32u_1 = (unsigned long)(nlp_count[k]<<15)/nlp_max;
        //temp_32_2 = (long)(nlp_count[k]<<5);
        //nlp_gain[k] = (short)(temp_32_2); 

	  if(far_end_active_1==1)
		{
			g_pstAec->far_end_nlp_count = g_pstAec->nlp_count[0];
		}
		else
		{
			if(g_pstAec->far_end_nlp_count+g_pstAec->nlp_inc>=g_pstAec->post_gain)
			{
				g_pstAec->far_end_nlp_count=g_pstAec->post_gain;
			}
			else
			{
				g_pstAec->far_end_nlp_count=g_pstAec->far_end_nlp_count+g_pstAec->nlp_inc;
			}
		}

	  for(k=0; k<sub_num_half; k++)
	  {
        temp_16_1 = (g_pstAec->nlp_count[k]<<4);

        //far_end_active_1 = 1;
  
        if(far_end_active_1==1)
        {
  
        if(k==0 || k==(sub_num_half-1))
        {
          //temp_32_1 = (long)residue_real[k]*(long)nlp_gain[k];
          temp_32_1 = (long)g_pstAec->residue_real[k]*(long)temp_16_1;
          //nlp_out_real[k] = (short)(temp_32_1>>14);
          y_sub_real[k] = (short)(temp_32_1>>14);
          y_sub_imag[k] = 0;
        }
        else
        {
          temp_32_2 = sub_num-k;
  
          //temp_32_1 = (long)residue_real[k]*(long)nlp_gain[k];
          temp_32_1 = (long)g_pstAec->residue_real[k]*(long)temp_16_1;
          //nlp_out_real[k] = (short)(temp_32_1>>14);
          y_sub_real[k] = (short)(temp_32_1>>14); 
          y_sub_real[temp_32_2] = (short)(temp_32_1>>14);
  
          //temp_32_1 = (long)residue_imag[k]*(long)nlp_gain[k];
          temp_32_1 = (long)g_pstAec->residue_imag[k]*(long)temp_16_1;
          //nlp_out_imag[k] = (short)(temp_32_1>>14); 
          y_sub_imag[k] = (short)(temp_32_1>>14);
          y_sub_imag[temp_32_2] = -(short)(temp_32_1>>14);
        }
        }
        else
        {
		  temp_32_1 = ((long)(g_pstAec->far_end_nlp_count)<<4);

          if(k==0 || k==(sub_num_half-1))
          {
            y_sub_real[k] = g_pstAec->near_fifo_real[g_pstAec->fifo_pt][k];

			temp_32_1 = (long)y_sub_real[k]*temp_32_1;
			temp_32_1 = temp_32_1>>14;

			if(temp_32_1>0x7fff)
			{
				y_sub_real[k]=0x7fff;
			}
			else if(temp_32_1<-0x7fff)
			{
				y_sub_real[k]=-0x7fff;
			}
			else
			{
				y_sub_real[k]=temp_32_1;
			}

            y_sub_imag[k] = 0;
          }
          else
          {
            temp_32_2 = sub_num-k;
  
            y_sub_real[k] = g_pstAec->near_fifo_real[g_pstAec->fifo_pt][k];

			temp_32_1 = (long)y_sub_real[k]*temp_32_1;
			temp_32_1 = temp_32_1>>14;

			if(temp_32_1>0x7fff)
			{
				y_sub_real[k]=0x7fff;
			}
			else if(temp_32_1<-0x7fff)
			{
				y_sub_real[k]=-0x7fff;
			}
			else
			{
				y_sub_real[k]=temp_32_1;
			}	

            //y_sub_real[temp_32_2] = g_pstAec->near_fifo_real[g_pstAec->fifo_pt][k];
			y_sub_real[temp_32_2] = y_sub_real[k];

			temp_32_1 = ((long)(g_pstAec->far_end_nlp_count)<<4);
            y_sub_imag[k] = g_pstAec->near_fifo_imag[g_pstAec->fifo_pt][k];

			temp_32_1 = (long)y_sub_imag[k]*temp_32_1;
			temp_32_1 = temp_32_1>>14;

			if(temp_32_1>0x7fff)
			{
				y_sub_imag[k]=0x7fff;
			}
			else if(temp_32_1<-0x7fff)
			{
				y_sub_imag[k]=-0x7fff;
			}
			else
			{
				y_sub_imag[k]=temp_32_1;
			}	
            //y_sub_imag[temp_32_2] = -g_pstAec->near_fifo_imag[g_pstAec->fifo_pt][k];
			y_sub_imag[temp_32_2] = -y_sub_imag[k];
          }
        }
      }
  
      g_pstAec->fifo_pt = (g_pstAec->fifo_pt + sub_filter_max_len-1) & 0x0000001f;
  
       /*
      for(k=sub_num_half; k<sub_num; k++)
      {
        //temp_16u_1 = sub_num-k;
        temp_32_1 = sub_num-k;
        y_sub_real[k] = nlp_out_real[temp_32_1];
        //y_sub_real[k] =  y_sub_real[temp_32_1];
        y_sub_imag[k] = -nlp_out_imag[temp_32_1];
      }
      */
  
      _aec_ditFFT(sub_num, 1, 0, log_2_sub_num, y_sub_real, y_sub_imag); 
  
      for(k=0; k<sub_num; k++)
      {
        g_pstAec->fr_buf[g_pstAec->fr_buf_pt][k] = y_sub_real[k];
      }
      //fr_buf_pt = (fr_buf_pt+fr_buf_len-1)%(fr_buf_len);
      //fr_buf_pt = (fr_buf_pt+fr_buf_len-1);
      if(g_pstAec->fr_buf_pt> 0)
      {
        g_pstAec->fr_buf_pt = g_pstAec->fr_buf_pt - 1;
      }
      else
      {
        g_pstAec->fr_buf_pt = fr_buf_len - 1;
      }
  
      for(k=0;k<sub_num;k++)
      {
        //temp_16_1 = 0;
        temp_32_2 = 0;
        for(j=0; j<fr_poly_len; j++)
        {
          //temp_16u_1 = fr_buf_index[j];
          temp_16_2 = (short)((j<<1) + 1);
          //if(temp_16u_1>0)
          //{
            //temp_16_2 = (temp_16_2+fr_buf_pt)%(fr_buf_len);
            temp_16_2 = temp_16_2 + g_pstAec->fr_buf_pt;
            if(temp_16_2>=fr_buf_len)
            {
              temp_16_2 = temp_16_2 - fr_buf_len;
            }
            temp_32_1 = g_pstAec->fr_buf[temp_16_2][k]*fr_poly[j][k];
            //temp_16_1 = temp_16_1 + (short)(temp_32_1>>15);
            temp_32_2 = temp_32_2 + temp_32_1;
            //temp_16_1 = temp_16_1 + (short)(fr_buf[temp_16u_1][k]*fr_poly[j][k]>>15);
          //}
        }
  
        //temp_16_2 = (k+y_out_buf_pt) & 0x000f;
        temp_32_1 = (k+g_pstAec->y_out_buf_pt) & 0x0000000f;
        if(k==(sub_num-1))
        {
          //y_out_buf[sub_num-1] = temp_16_1;
          //y_out_buf[temp_16_2] = temp_16_1;
          //y_out_buf[temp_32_1] = temp_16_1;
          g_pstAec->y_out_buf[temp_32_1] = temp_32_2;
        }
        else
        {
          //y_out_buf[k] = temp_16_1 + y_out_buf[k+1];
          //y_out_buf[temp_16_2] = temp_16_1 + y_out_buf[temp_16_2];
          //y_out_buf[temp_32_1] = temp_16_1 + y_out_buf[temp_32_1];
          g_pstAec->y_out_buf[temp_32_1] = temp_32_2 + g_pstAec->y_out_buf[temp_32_1];
        }
      }
    }
    else
    {
      /*
      for(k=0; k<sub_num-1; k++)
      {
        y_out_buf[k] = y_out_buf[k+1];
      }
      y_out_buf[sub_num-1] = 0;
      */
      
      temp_32_3 = (sub_num-1+g_pstAec->y_out_buf_pt) & 0x000f;
      g_pstAec->y_out_buf[temp_32_3] = 0;
    }
  
    //yout = (short)(y_out_buf[0]*down_sample);
    //yout = (short)(y_out_buf[y_out_buf_pt]*down_sample);
    yout = (long)(g_pstAec->y_out_buf[g_pstAec->y_out_buf_pt]*down_sample);
  
    g_pstAec->y_out_buf_pt = (g_pstAec->y_out_buf_pt+1) & 0x000f;
    g_pstAec->hr_buf_pt = (g_pstAec->hr_buf_pt+hr_buf_len-1) % hr_buf_len;
  
    //fr_buf_index_pt = (fr_buf_index_pt+1) % down_sample;
    g_pstAec->fr_buf_index_pt = (g_pstAec->fr_buf_index_pt+1) & (0x0007);
  
    return(yout);
}


__attribute__((weak)) short hs_aec_process(short xin, short din)
{
  long xout;

  if(g_stAecInfo.u8AecDis)
    return din;
  
  xout = _aec_process(xin, din);
  xout *= g_stAecInfo.u8SwGain;
  xout >>= 15;
  
  return _aec_calEq(xout);
}

__attribute__((weak)) void hs_aec_init(void)
{
  int i,j; 

  if(g_pstAec)
  {
    hs_free(g_pstAec);
    g_pstAec = NULL;
  }

  g_pstAec = (hs_aecvar_t *)hs_malloc(sizeof(hs_aecvar_t), __MT_Z_GENERAL);
  if(!g_pstAec)
    return ;

  g_pstAecEq = (hs_aec_eq_t *)hs_malloc(sizeof(hs_aec_eq_t), __MT_Z_GENERAL);

  if(HS_CFG_OK != hs_cfg_getDataByIndex(HS_CFG_PROD_AUDIO_AEC, (uint8_t *)&g_stAecInfo, sizeof(hs_aec_var_t)))
  {
    g_stAecInfo.u8SwGain = 1;
    g_stAecInfo.u8AecDis = 0;
    
    g_stAecInfo.s16Mu       = 0x6666;  //0x6ccc; //0x6666; //0x4A3D;
    g_stAecInfo.s16Beta     = 16; //16;
    g_stAecInfo.s16FarAct1  = 0x280;
    g_stAecInfo.s16FarAct2  = 0x80;
    g_stAecInfo.s16PostGain = 0x25ff;

    g_stAecInfo.s16NlpInc   = 10;
    g_stAecInfo.s16AlphaEnergy = 0x7333;

    g_stAecInfo.s16EqB1A    = 0xc957;
    g_stAecInfo.s16EqB5A    = 0xdec1;
    g_stAecInfo.s16EqB1Gain = 0;
    g_stAecInfo.s16EqB5Gain = 0;
  }

  g_stAecInfo.u8SwGain = g_stAecInfo.u8SwGain == 0 ? 1 : g_stAecInfo.u8SwGain;

  g_pstAec->noise_cnt = 1;

  g_pstAec->lamda = 0x0000A3D7;
  g_pstAec->beta = g_stAecInfo.s16Beta;
  g_pstAec->mu = g_stAecInfo.s16Mu;
  g_pstAec->far_end_active_th = 32;
  g_pstAec->far_end_active_th_1 = g_stAecInfo.s16FarAct1;
  g_pstAec->far_end_nlp_count = 512;  //1024+512;
  g_pstAec->post_gain = g_stAecInfo.s16PostGain;
  g_pstAec->near_end_active_th = 16;
  g_pstAec->wup_dis_time = 0xc8;
  g_pstAec->wup_en_time = 0x28;
  g_pstAec->wup_dis_count_clear = 10;
  g_pstAec->wup_en_count_clear = 20;
  g_pstAec->wup_cnt_step = 8;
 
  g_pstAec->nlp_th = 0x1999;
  g_pstAec->nlp_inc = g_stAecInfo.s16NlpInc;
  g_pstAec->nlp_dec = 40;
  g_pstAec->nlp_low = 60;
  g_pstAec->alpha_erl = 0x7EB8;
  g_pstAec->alpha_energy = g_stAecInfo.s16AlphaEnergy;
  g_pstAec->alpha_energy_comp = 0x0ccd;

  g_pstAec->w_filter_len[0] = sub_filter_max_len;
  g_pstAec->w_filter_len[1] = sub_filter_max_len;
  g_pstAec->w_filter_len[2] = sub_filter_max_len;
  g_pstAec->w_filter_len[3] = sub_filter_max_len;
  g_pstAec->w_filter_len[4] = sub_filter_max_len;
  g_pstAec->w_filter_len[5] = sub_filter_max_len;
  g_pstAec->w_filter_len[6] = sub_filter_max_len;
  g_pstAec->w_filter_len[7] = sub_filter_max_len;
  g_pstAec->w_filter_len[8] = sub_filter_max_len;

  memset(g_pstAec->hr_buf_far, 0, hr_buf_len);
  memset(g_pstAec->hr_buf_near, 0, hr_buf_len);
  memset(g_pstAec->fr_buf, 0, fr_buf_len*sub_num);
  memset(g_pstAec->Enear, 0, sub_num_half);
  memset(g_pstAec->Efar,0 , sub_num_half);
  memset(g_pstAec->Efar_echo, 0, sub_num_half);
  memset(g_pstAec->Eres, 0, sub_num_half);
  memset(g_pstAec->far_energy, 0, sub_num_half);
  memset(g_pstAec->res_energy, 0, sub_num_half);
  memset(g_pstAec->wup_en_count, 0, sub_num_half);
  memset(g_pstAec->wup_dis_count, 0, sub_num_half);  
  memset(g_pstAec->erl, 0, sub_num_half);
  memset(g_pstAec->near_fifo_real, 0, filterMuxhalf);
  memset(g_pstAec->near_fifo_imag, 0, filterMuxhalf);
  memset(g_pstAec->far_fifo_real, 0, filterMuxhalf);
  memset(g_pstAec->far_fifo_imag, 0, filterMuxhalf);
  memset(g_pstAec->far_echo_fifo_real, 0, filterMuxhalf);
  memset(g_pstAec->far_echo_fifo_imag, 0, filterMuxhalf);
  memset(g_pstAec->w_filter_real, 0, filterMuxhalf);
  memset(g_pstAec->w_filter_imag, 0, filterMuxhalf);
  memset(g_pstAec->y_out_buf, 0, sub_num);

  for(i=0; i<sub_num_half; i++)
  {
    g_pstAec->w_update[i] = 0;
    g_pstAec->Pmin_far[i] = Pmax;
    g_pstAec->Pn_far[i]   = Pmax;
    g_pstAec->nlp_count[i]= nlp_max;
  }

  for(i=0; i<noise_winNo; i++)
    for(j=0; j<sub_num_half; j++)
      g_pstAec->PLmin_far[i][j] = Pmax;    

  _aec_eqInit();
}

__attribute__((weak)) void hs_aec_uninit(void)
{
  if(g_pstAec)
  {
    hs_free(g_pstAec);
    hs_free(g_pstAecEq);
    g_pstAec = NULL;
    g_pstAecEq = NULL;
  }
}



#endif

/** @} */
