/*
    drvhal - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/audio/lib_audio.c
 * @brief   lib_audio file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */

#include "lib_ans.h"
#include "math.h"

#if HAL_USE_AUDIO

static const short hammingWin[FRAME_SIZE*2] = {
  0x0a3d, 0x0a42, 0x0a52, 0x0a6c, 0x0a90, 0x0abf, 0x0af8, 0x0b3b, 
  0x0b89, 0x0be1, 0x0c43, 0x0caf, 0x0d25, 0x0da5, 0x0e2e, 0x0ec2, 
  0x0f5f, 0x1005, 0x10b5, 0x116f, 0x1231, 0x12fc, 0x13d1, 0x14ad, 
  0x1593, 0x1681, 0x1777, 0x1875, 0x197c, 0x1a89, 0x1b9f, 0x1cbc, 
  0x1de0, 0x1f0a, 0x203c, 0x2174, 0x22b3, 0x23f7, 0x2541, 0x2691, 
  0x27e7, 0x2941, 0x2aa1, 0x2c05, 0x2d6e, 0x2edb, 0x304b, 0x31c0, 
  0x3338, 0x34b3, 0x3631, 0x37b1, 0x3934, 0x3ab9, 0x3c40, 0x3dc9, 
  0x3f53, 0x40dd, 0x4269, 0x43f5, 0x4581, 0x470d, 0x4899, 0x4a25, 
  0x4baf, 0x4d38, 0x4ec0, 0x5046, 0x51ca, 0x534c, 0x54cb, 0x5648, 
  0x57c1, 0x5937, 0x5aaa, 0x5c19, 0x5d83, 0x5eea, 0x604c, 0x61a9, 
  0x6301, 0x6454, 0x65a1, 0x66e8, 0x682a, 0x6965, 0x6a9a, 0x6bc8, 
  0x6cf0, 0x6e10, 0x6f29, 0x703b, 0x7145, 0x7247, 0x7341, 0x7433, 
  0x751d, 0x75ff, 0x76d7, 0x77a7, 0x786e, 0x792c, 0x79e0, 0x7a8b, 
  0x7b2d, 0x7bc6, 0x7c54, 0x7cd9, 0x7d54, 0x7dc5, 0x7e2c, 0x7e89, 
  0x7edb, 0x7f24, 0x7f62, 0x7f96, 0x7fc0, 0x7fdf, 0x7ff4, 0x7ffe
};

static const unsigned long log_10_db[64] = {
  0x80000000, 0x721482bf, 0x65ac8c2f, 0x5a9df7ab, 0x50c335d3, 0x47faccf0, 0x4026e73c, 0x392ced8d, 
  0x32f52cfe, 0x2d6a866f, 0x287a26c4, 0x241346f5, 0x2026f30f, 0x1ca7d767, 0x198a1357, 0x16c310e3, 
  0x144960c5, 0x12149a5f, 0x101d3f2d, 0x0e5ca14c, 0x0ccccccc, 0x0b687379, 0x0a2adad1, 0x090fcbf7, 
  0x08138561, 0x0732ae18, 0x066a4a52, 0x05b7b15a, 0x0518847f, 0x048aa70b, 0x040c3713, 0x039b8718, 
  0x0337184e, 0x02dd958a, 0x028dcebb, 0x0246b4e3, 0x0207567a, 0x01cedc3c, 0x019c8651, 0x016fa9ba, 
  0x0147ae14, 0x01240b8c, 0x01044914, 0x00e7facb, 0x00cec089, 0x00b8449c, 0x00a43aa1, 0x00925e89, 
  0x008273a6, 0x007443e7, 0x00679f1b, 0x005c5a4f, 0x00524f3b, 0x00495bc1, 0x00416179, 0x003a4549, 
  0x0033ef0c, 0x002e4939, 0x002940a1, 0x0024c42c, 0x0020c49b, 0x001d345a, 0x001a074e, 0x001732ad
};

static const short BANK_F[BANK_SIZE+1] = {1,2,5,9,12,15,19,23,27,32,37,43,50,58,66,77,89,103,129,130};

static const short w_real[BLOCK_SIZE/2] = {
  0x7fff, 0x7ff6, 0x7fd9, 0x7fa7, 0x7f62, 0x7f0a, 0x7e9d, 0x7e1e, //cos(2*pi/N)
  0x7d8a, 0x7ce4, 0x7c2a, 0x7b5d, 0x7a7d, 0x798a, 0x7885, 0x776c, 
  0x7642, 0x7505, 0x73b6, 0x7255, 0x70e3, 0x6f5f, 0x6dca, 0x6c24, 
  0x6a6e, 0x68a7, 0x66d0, 0x64e9, 0x62f2, 0x60ec, 0x5ed7, 0x5cb4, 
  0x5a82, 0x5843, 0x55f6, 0x539b, 0x5134, 0x4ec0, 0x4c40, 0x49b4, 
  0x471d, 0x447b, 0x41ce, 0x3f17, 0x3c57, 0x398d, 0x36ba, 0x33df, 
  0x30fc, 0x2e11, 0x2b1f, 0x2827, 0x2528, 0x2224, 0x1f1a, 0x1c0c, 
  0x18f9, 0x15e2, 0x12c8, 0x0fab, 0x0c8c, 0x096b, 0x0648, 0x0324, 
  0x0000, 0xfcdc, 0xf9b8, 0xf695, 0xf374, 0xf055, 0xed38, 0xea1e, 
  0xe707, 0xe3f4, 0xe0e6, 0xdddc, 0xdad8, 0xd7d9, 0xd4e1, 0xd1ef, 
  0xcf04, 0xcc21, 0xc946, 0xc673, 0xc3a9, 0xc0e9, 0xbe32, 0xbb85, 
  0xb8e3, 0xb64c, 0xb3c0, 0xb140, 0xaecc, 0xac65, 0xaa0a, 0xa7bd, 
  0xa57e, 0xa34c, 0xa129, 0x9f14, 0x9d0e, 0x9b17, 0x9930, 0x9759, 
  0x9592, 0x93dc, 0x9236, 0x90a1, 0x8f1d, 0x8dab, 0x8c4a, 0x8afb, 
  0x89be, 0x8894, 0x877b, 0x8676, 0x8583, 0x84a3, 0x83d6, 0x831c, 
  0x8276, 0x81e2, 0x8163, 0x80f6, 0x809e, 0x8059, 0x8027, 0x800a
};

static const short w_imag[BLOCK_SIZE/2] = {
  0x0000, 0x0324, 0x0648, 0x096b, 0x0c8c, 0x0fab, 0x12c8, 0x15e2, //sin(2*pi/N)
  0x18f9, 0x1c0c, 0x1f1a, 0x2224, 0x2528, 0x2827, 0x2b1f, 0x2e11, 
  0x30fc, 0x33df, 0x36ba, 0x398d, 0x3c57, 0x3f17, 0x41ce, 0x447b, 
  0x471d, 0x49b4, 0x4c40, 0x4ec0, 0x5134, 0x539b, 0x55f6, 0x5843, 
  0x5a82, 0x5cb4, 0x5ed7, 0x60ec, 0x62f2, 0x64e9, 0x66d0, 0x68a7, 
  0x6a6e, 0x6c24, 0x6dca, 0x6f5f, 0x70e3, 0x7255, 0x73b6, 0x7505, 
  0x7642, 0x776c, 0x7885, 0x798a, 0x7a7d, 0x7b5d, 0x7c2a, 0x7ce4, 
  0x7d8a, 0x7e1e, 0x7e9d, 0x7f0a, 0x7f62, 0x7fa7, 0x7fd9, 0x7ff6, 
  0x7fff, 0x7ff6, 0x7fd9, 0x7fa7, 0x7f62, 0x7f0a, 0x7e9d, 0x7e1e, 
  0x7d8a, 0x7ce4, 0x7c2a, 0x7b5d, 0x7a7d, 0x798a, 0x7885, 0x776c, 
  0x7642, 0x7505, 0x73b6, 0x7255, 0x70e3, 0x6f5f, 0x6dca, 0x6c24, 
  0x6a6e, 0x68a7, 0x66d0, 0x64e9, 0x62f2, 0x60ec, 0x5ed7, 0x5cb4, 
  0x5a82, 0x5843, 0x55f6, 0x539b, 0x5134, 0x4ec0, 0x4c40, 0x49b4, 
  0x471d, 0x447b, 0x41ce, 0x3f17, 0x3c57, 0x398d, 0x36ba, 0x33df, 
  0x30fc, 0x2e11, 0x2b1f, 0x2827, 0x2528, 0x2224, 0x1f1a, 0x1c0c, 
  0x18f9, 0x15e2, 0x12c8, 0x0fab, 0x0c8c, 0x096b, 0x0648, 0x0324
};


static hs_ansvar_t *g_pstAns;

unsigned sqrt_16b(unsigned long x)
{

   unsigned long temp = 0;
   unsigned long v_bit = 15;
   unsigned long n = 0;
   unsigned long b = 0x8000;

   if (x <= 1)
       return x;

   do{
       temp = ((n << 1) + b) << (v_bit--);
       if (x >= temp)
       {
           n += b;
           x -= temp;
       }
   }while (b >>= 1);

   return n;
}

void _ans_ditFFT(short N, short inverse, short m,long *yout_real, long *yout_imag)
{
	short	LH;
	short nm;
	short i, j, k;
	long t;

	short le;
	short L;
	short B;
	short p;
	short ip;

	long t_real;
	long t_imag;
	short w_imag_t;
	int64_t temp;

	LH = N>>1;
	nm = N - 2;                   
	j = LH;

	for(i=1;i<=nm;i++)
	{
		if(i<j)
		{
			t = yout_real[j];
			yout_real[j] = yout_real[i];
			yout_real[i] = t;
			if(inverse==1)
			{
				t = yout_imag[j];
				yout_imag[j] = yout_imag[i];
				yout_imag[i] = t;
			}
		}
		k = LH;
		while(j>=k)
		{
			j=j-k;
			k=k/2;
		}
		j=j+k;
	}

	for (L=0;L<=m-1;L++)
	{
		le = 1<<(L+1);
		B = le>>1;

		for(j=0;j<=(B-1);j++)
		{
			p=(1<<(m-L-1))*j;

			if(inverse==1)
			{
				w_imag_t = w_imag[p];
			}
			else
			{
				w_imag_t = -w_imag[p];
			}
 
			for (i=j;i<=N-1;i=i+le)
			{
				ip = i+B;

				temp = (int64_t)yout_real[ip]*(int64_t)w_real[p];
				temp = temp - (int64_t)yout_imag[ip]*(int64_t)w_imag_t;
				t_real = (long)(temp>>15);

				temp = (int64_t)yout_real[ip]*(int64_t)w_imag_t;
				temp = temp + (int64_t)yout_imag[ip]*(int64_t)w_real[p];
				t_imag = (long)(temp>>15);

		
				yout_real[ip]=yout_real[i]-t_real;
				yout_imag[ip]=yout_imag[i]-t_imag;
				yout_real[i]=yout_real[i]+t_real;
				yout_imag[i]=yout_imag[i]+t_imag;
			}
		}
	}
}
void hs_ans_process(short *xin, short *yout)
{
	short i, j, k, h;
	unsigned short temp_16u;
	unsigned long  temp_32u;
	unsigned long long temp_64u;
	long temp_32;
	int64_t temp_64;

	//////	move raw data into buffer
	for(i=0;i<FRAME_SIZE;i++)
	{
		g_pstAns->micIn[g_pstAns->mic_in_frame_index][i] = xin[i];
	}
	//////	move raw data into buffer end

	//////	add window at the buffer data
	temp_16u=(g_pstAns->mic_in_frame_index + 3)%4;
	for(i=0; i<FRAME_SIZE;i++)
	{
		g_pstAns->micWin[i]=(long)g_pstAns->micIn[temp_16u][i]*(long)hammingWin[i];
	}
	
	temp_16u=(g_pstAns->mic_in_frame_index + 2)%4;
	for(i=0; i<FRAME_SIZE;i++)
	{
		g_pstAns->micWin[FRAME_SIZE+i]=(long)g_pstAns->micIn[temp_16u][i]*(long)hammingWin[FRAME_SIZE+i];
	}

	temp_16u=(g_pstAns->mic_in_frame_index + 1)%4;
	for(i=0; i<FRAME_SIZE;i++)
	{
		g_pstAns->micWin[(FRAME_SIZE<<1)+i]=(long)g_pstAns->micIn[temp_16u][i]*(long)hammingWin[(FRAME_SIZE<<1)-i-1];
	}

	for(i=0; i<FRAME_SIZE;i++)
	{
		g_pstAns->micWin[FRAME_SIZE*3+i]=(long)g_pstAns->micIn[g_pstAns->mic_in_frame_index][i]*(long)hammingWin[FRAME_SIZE-i-1];
	}
	//////	add window at the buffer end

	////////	debug
	//fprintf( pTest_1, "micwin:\n");
	//for(i=0; i<BLOCK_SIZE; i++)
	//{
	//	fprintf( pTest_1, "0x%08x, ", micWin[i]);
	//	if(i%8==7)
	//	{
	//		fprintf( pTest_1, "\n");
	//	}
	//}
	//fprintf( pTest_1, "\n");
	//////////	debug end

	//////	fft analysis
	for(i=0; i<BLOCK_SIZE; i++)
	{
		g_pstAns->micWin_f_real[i] = (long) (g_pstAns->micWin[i]>>LOG_2_BLOCK);
		g_pstAns->micWin_f_imag[i] = 0;
	}
  
	_ans_ditFFT(BLOCK_SIZE, 0, LOG_2_BLOCK, g_pstAns->micWin_f_real, g_pstAns->micWin_f_imag); 
  
	for(i=0;i<(BLOCK_SIZE/2+1);i++)
	{
		temp_64 = (int64_t)g_pstAns->micWin_f_real[i]*(int64_t)g_pstAns->micWin_f_real[i];
		g_pstAns->micWin_f_amp[i]=(unsigned long)(temp_64>>28);
		temp_64 = (int64_t)g_pstAns->micWin_f_imag[i]*(int64_t)g_pstAns->micWin_f_imag[i];
		g_pstAns->micWin_f_amp[i] = g_pstAns->micWin_f_amp[i]+(unsigned long)(temp_64>>28);
	}
	//////	fft analysis end

	////// compute smooth power
	for(i=0; i<BANK_SIZE; i++)
	{
		temp_64u = 0;
		for(j=(BANK_F[i]-1);j<(BANK_F[i+1]-1);j++)
		{
			temp_64u = (unsigned long long)g_pstAns->micWin_f_amp[j]+temp_64u;
		}
		g_pstAns->pr[i] = (unsigned long)(temp_64u/(BANK_F[i+1]-BANK_F[i]));
	}
	for(i=0; i<BANK_SIZE; i++)
	{
		g_pstAns->pn[i] = g_pstAns->pn_fifo_pre[i];
		g_pstAns->px[i] = g_pstAns->px_fifo_pre[i];
	}
	if(g_pstAns->first_frame!=0)
	{
		g_pstAns->px_sum = 0;
		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->px_sum = g_pstAns->px_sum + (g_pstAns->px[i]>>4);
		}
		g_pstAns->pr_sum = 0;
		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->pr_sum = g_pstAns->pr_sum + (g_pstAns->pr[i]>>4);
		}
		if(g_pstAns->pr_sum==0)
		{
			temp_64u = MAX_16U;
		}
		else
		{
			temp_64u = (((unsigned long long)g_pstAns->px_sum)<<15)/g_pstAns->pr_sum;
		}
		if(temp_64u>MAX_16U)
		{
			temp_16u = MAX_16U;
		}
		else
		{
			temp_16u = (unsigned short)temp_64u;
		}

		if(temp_16u>=MAX_15U)
		{
			temp_32u = (unsigned long)(temp_16u-MAX_15U)*(unsigned long)(temp_16u-MAX_15U);
		}
		else
		{
			temp_32u = (unsigned long)(MAX_15U-temp_16u)*(unsigned long)(MAX_15U-temp_16u);
		}
		temp_16u = (unsigned short)(temp_32u>>15);
		if(temp_16u>=MAX_15U)
		{
			temp_16u = MAX_16U;
		}
		else
		{
			temp_16u = MAX_15U + temp_16u;
		}
					
		g_pstAns->alpha_ce = (unsigned short)((((unsigned long)g_pstAns->alpha_max)<<15)/temp_16u);
		if(g_pstAns->alpha_ce<g_pstAns->cmax1)
		{
			temp_16u=g_pstAns->cmax1;
		}
		else
		{
			temp_16u=g_pstAns->alpha_ce;
		}
		temp_32u = (unsigned long)(g_pstAns->cmax*g_pstAns->alpha_c);
		temp_32u = temp_32u + (unsigned long)(temp_16u*(0x10000-(unsigned long)g_pstAns->cmax));
		g_pstAns->alpha_c = (unsigned short)(temp_32u>>16);	
		for(i=0;i<BANK_SIZE;i++)
		{
			if(g_pstAns->pn[i]==0)
			{
				g_pstAns->SNR_temp[i] = 0xC000;	//Q14
			}
			else
			{
				temp_64u=(((unsigned long long)g_pstAns->px[i])<<14)/g_pstAns->pn[i];
				if(temp_64u>0xC000)
				{
					g_pstAns->SNR_temp[i] = 0xC000;
				}
				else
				{
					g_pstAns->SNR_temp[i] = (unsigned short)temp_64u;
				}
			}
			if(g_pstAns->SNR_temp[i]>=0x4000)
			{
				temp_32u = ((unsigned long)g_pstAns->SNR_temp[i]-0x4000)*((unsigned long)g_pstAns->SNR_temp[i]-0x4000);
			}
			else
			{
				temp_32u = (0x4000-(unsigned long)g_pstAns->SNR_temp[i])*(0x4000-(unsigned long)g_pstAns->SNR_temp[i]);
			}
			temp_16u = (unsigned short)(temp_32u>>15);
			temp_16u = temp_16u + 0x2000;
			g_pstAns->alpha[i] = (unsigned short)((((unsigned long)g_pstAns->alpha_c)<<13)/temp_16u);
			if(g_pstAns->alpha[i]<g_pstAns->alpha_min)
			{
				g_pstAns->alpha[i]=g_pstAns->alpha_min;
			}

			temp_64u = (unsigned long long)g_pstAns->px[i]*(unsigned long long)g_pstAns->alpha[i];
			temp_64u = temp_64u + (unsigned long long)(0x10000-g_pstAns->alpha[i])*(unsigned long long)g_pstAns->pr[i];
			g_pstAns->px[i]=(unsigned long)(temp_64u>>16);
		}

	}
	else
	{
		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->px[i]=g_pstAns->pr[i];
			g_pstAns->pn[i]=g_pstAns->pr[i];
			g_pstAns->px_mean[i]=(unsigned short)(g_pstAns->pr[i]>>16);
			temp_64u = (unsigned long long)g_pstAns->pr[i]*(unsigned long long)g_pstAns->pr[i];
			g_pstAns->px_square[i] = (unsigned long)(temp_64u>>32);
		}
	}
	////// compute smooth power end

	////// compute bias correction
	for(i=0;i<BANK_SIZE;i++)
	{
		temp_32u = (unsigned long)g_pstAns->alpha[i]*(unsigned long)g_pstAns->alpha[i];
		temp_16u = (unsigned short)(temp_32u>>16);
		if(temp_16u>P8_16U)
		{
			temp_16u=P8_16U;
		}
		g_pstAns->beta_q[i]=temp_16u;
		temp_32u = (unsigned long)g_pstAns->px_mean[i]*(unsigned long)g_pstAns->beta_q[i];
		temp_32u = temp_32u + (g_pstAns->px[i]>>16)*(0x10000-(unsigned long)g_pstAns->beta_q[i]);
		g_pstAns->px_mean[i]=(unsigned short)(temp_32u>>16);

		temp_64u = (unsigned long long)g_pstAns->px[i]*(unsigned long long)g_pstAns->px[i];
		temp_64u = temp_64u>>32;
		temp_64u = temp_64u*(unsigned long long)(0x10000-g_pstAns->beta_q[i]);
		temp_64u = temp_64u + (unsigned long long)g_pstAns->px_square[i]*(unsigned long long)g_pstAns->beta_q[i];
		g_pstAns->px_square[i]=(unsigned long)(temp_64u>>16);

		temp_32u = (unsigned long)g_pstAns->px_mean[i]*(unsigned long)g_pstAns->px_mean[i];
		if(temp_32u>g_pstAns->px_square[i])
		{
			g_pstAns->px_var[i] = temp_32u-g_pstAns->px_square[i];
		}
		else
		{
			g_pstAns->px_var[i] = g_pstAns->px_square[i]-temp_32u;
		}
		temp_16u=sqrt_16b(g_pstAns->px_var[i]);
		if(g_pstAns->pn[i]==0)
		{
			g_pstAns->Q_eq[i]=0x4000;
		}
		else
		{	
			temp_64u=(((unsigned long long)temp_16u)<<24)/g_pstAns->pn[i];
			if(temp_64u>0xffff)
			{
				temp_16u = 0xffff;
			}
			else
			{
				temp_16u = (unsigned short)temp_64u;
			}
			temp_32u = (unsigned long)temp_16u*(unsigned long)temp_16u;
			temp_32u = temp_32u>>2;
			if(temp_32u>0x4000)
			{
				g_pstAns->Q_eq[i]=0x4000;
			}
			else
			{
				g_pstAns->Q_eq[i]=(unsigned short)temp_32u;
			}
		}
		if(g_pstAns->Q_eq[i]==0)
		{
			g_pstAns->Q_eq_recip = MAX_32U;
		}
		else
		{
			g_pstAns->Q_eq_recip = (0x80000000)/g_pstAns->Q_eq[i];
		}
		temp_64u = ((g_pstAns->Q_eq_recip-(((unsigned long)g_pstAns->Md)<<1))<<8)/(0x10000-g_pstAns->Md);
		if(temp_64u>0xffff)
		{
			g_pstAns->Q_eq_e[i]=0xffff;
		}
		else
		{
			g_pstAns->Q_eq_e[i]=(unsigned short)temp_64u;
		}
		temp_64u = (((unsigned long long)(g_pstAns->Q_eq_recip-(((unsigned long)g_pstAns->Md_sub)<<1)))<<8)/(0x10000-g_pstAns->Md_sub);
		if(temp_64u>0xffff)
		{
			g_pstAns->Q_eq_e_sub[i]=0xffff;
		}
		else
		{
			g_pstAns->Q_eq_e_sub[i]=(unsigned short)temp_64u;
		}
		if(g_pstAns->Q_eq_e[i]==0)
		{
			g_pstAns->Bmin[i]=0xFFFF;
		}
		else
		{
			g_pstAns->Bmin[i] = (unsigned short)((unsigned long)((D-1)<<17)/g_pstAns->Q_eq_e[i]);
			g_pstAns->Bmin[i] = g_pstAns->Bmin[i]+0x0100;
		}
		if(g_pstAns->Q_eq_e_sub[i]==0)
		{
			g_pstAns->Bmin_sub[i]=0xFFFF;
		}
		else
		{
			g_pstAns->Bmin_sub[i] = (unsigned short)((unsigned long)((V-1)<<17)/g_pstAns->Q_eq_e_sub[i]);
			g_pstAns->Bmin_sub[i] = g_pstAns->Bmin_sub[i]+0x0100;
		}
	}
	temp_32u=0;
	for(i=1;i<BANK_SIZE-1;i++)
	{
		temp_32u = temp_32u + g_pstAns->Q_eq[i];
	}
	g_pstAns->Q_1=(unsigned short)(temp_32u/(BANK_SIZE-2));
	g_pstAns->Q_1_sqrt=sqrt_16b((((unsigned long)g_pstAns->Q_1)<<17));
	g_pstAns->Bc = (unsigned short)((g_pstAns->alpha_v*g_pstAns->Q_1_sqrt)>>16);
	g_pstAns->Bc = g_pstAns->Bc + 0x2000;
	////// compute bias correction end

	////// noise power estimation
	for(i=0;i<BANK_SIZE;i++)
	{
		g_pstAns->k_mode[i]=0;
	}

	for(i=0;i<BANK_SIZE;i++)
	{
		if(g_pstAns->first_frame<V)
		{
			g_pstAns->pmin_act[i] = g_pstAns->px[i];
			g_pstAns->pmin_act_sub[i] = g_pstAns->px[i];
		}
		else
		{
			temp_32u=(unsigned long)(g_pstAns->Bmin[i]*g_pstAns->Bc);
			temp_64u=(unsigned long long)g_pstAns->px[i]*(unsigned long long)temp_32u;
			temp_32u=(unsigned long)(temp_64u>>21);
			if(temp_32u<g_pstAns->pmin_act[i])
			{
				g_pstAns->pmin_act[i]=temp_32u;
				temp_32u=(unsigned long)(g_pstAns->Bmin_sub[i]*g_pstAns->Bc);
				temp_64u=(unsigned long long)g_pstAns->px[i]*(unsigned long long)temp_32u;
				temp_32u=(unsigned long)(temp_64u>>21);
				g_pstAns->pmin_act_sub[i]=temp_32u;
				g_pstAns->k_mode[i]=1;
			}
		}
	}

	if(g_pstAns->subwc==V)
	{
		for(i=0;i<BANK_SIZE;i++)
		{
			if(g_pstAns->k_mode[i]==1)
			{
				g_pstAns->lmin_flag[i]=0;
			}
		}
		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->pmin_sub[g_pstAns->pmin_sub_index][i]=g_pstAns->pmin_act[i];
		}
		g_pstAns->pmin_sub_index = (g_pstAns->pmin_sub_index+1)%U;
		for(i=0;i<BANK_SIZE;i++)
		{
			temp_32u=MAX_32U;
			for(j=0;j<U;j++)
			{
				if(temp_32u>g_pstAns->pmin_sub[j][i])
				{
					temp_32u=g_pstAns->pmin_sub[j][i];
				}
			}
			g_pstAns->pmin[i]=temp_32u;
		}

		if(g_pstAns->Q_1<0x03D7)	//0.03
		{
			g_pstAns->noise_slope_max = 0x0800;
		}
		else if(g_pstAns->Q_1<0x0666) //0.05
		{
			g_pstAns->noise_slope_max = 0x0400;
		}
		else if(g_pstAns->Q_1<0x07AE)	//0.06
		{
			g_pstAns->noise_slope_max = 0x0200;
		}
		else
		{
			g_pstAns->noise_slope_max = 0x0133;
		}
		for(i=0;i<BANK_SIZE;i++)
		{
			temp_64u=(unsigned long long)g_pstAns->noise_slope_max*(unsigned long long)g_pstAns->pmin[i];
			temp_64u=temp_64u>>8;
			if(temp_64u>MAX_32U)
			{
				temp_64u=MAX_32U;
			}
			temp_32u=(unsigned long)temp_64u;
			if((g_pstAns->lmin_flag[i]==1)&&(g_pstAns->pmin_act_sub[i]<temp_32u)&&(g_pstAns->pmin_act_sub[i]>g_pstAns->pmin[i]))
			{
				g_pstAns->pmin[i]=g_pstAns->pmin_act_sub[i];
				for(j=0;j<U;j++)
				{
					g_pstAns->pmin_sub[j][i]=g_pstAns->pmin_act_sub[i];
				}
			}
		}

		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->noise_increase_flag = 1;
			for(j=0;j<(U-1);j++)
			{
				k=(g_pstAns->pmin_sub_index+j)%U;
				h=(k+1)%U;
				if(g_pstAns->pmin_sub[k][i]>g_pstAns->pmin_sub[h][i])
				{
					g_pstAns->noise_increase_flag = 0;
				}
			}
			if(g_pstAns->noise_increase_flag==1)
			{
				g_pstAns->pmin[i]=g_pstAns->pmin_act_sub[i];
				for(j=0;j<U;j++)
				{
					g_pstAns->pmin_sub[j][i]=g_pstAns->pmin_act_sub[i];
				}
			}
		}

		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->lmin_flag[i]=0;
			g_pstAns->pmin_act[i]=MAX_32U;
		}
		g_pstAns->subwc = 1;
	}
	else
	{
		if(g_pstAns->subwc>1)
		{
			for(i=0;i<BANK_SIZE;i++)
			{
				if(g_pstAns->k_mode[i]==1)
				{
					g_pstAns->lmin_flag[i]=1;
				}
			}
		}
		g_pstAns->subwc = g_pstAns->subwc+1;
	}

	for(i=0;i<BANK_SIZE;i++)
	{
		if(g_pstAns->first_frame<V)
		{
			g_pstAns->pmin[i]=g_pstAns->px[i];
		}
		if(g_pstAns->pmin_act_sub[i]<g_pstAns->pmin[i])
		{
			g_pstAns->pn[i]=g_pstAns->pmin_act_sub[i];
		}
		else
		{
			g_pstAns->pn[i]=g_pstAns->pmin[i];
		}
		g_pstAns->pmin[i]=g_pstAns->pn[i];
	}
	////// noise power estimation end

	////////	debug
	//fprintf( pTest_1, "px:\n");
	//for(i=0; i<BANK_SIZE; i++)
	//{
	//	fprintf( pTest_1, "0x%08x, ", px[i]);
	//	if(i%8==7)
	//	{
	//		fprintf( pTest_1, "\n");
	//	}
	//}
	//fprintf( pTest_1, "\n");
	//////////	debug end


	//////	fifo operation
	if(g_pstAns->first_frame<(FIFO_DEPTH-1))
	{
		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->SNR_post_1[i]=0;
		}
	}
	else
	{
		for(i=0;i<BANK_SIZE;i++)
		{
			if(g_pstAns->pn[i]==0)
			{
				g_pstAns->SNR_post_1[i] = 0x8000;	//Q15
			}
			else
			{
				temp_64u=(((unsigned long long)g_pstAns->px[i])<<15)/g_pstAns->pn[i];
				if(temp_64u>0x8000)
				{
					g_pstAns->SNR_post_1[i] = 0x8000;
				}
				else
				{
					g_pstAns->SNR_post_1[i] = (unsigned short)temp_64u;
				}
			}
		}
	}
	for(i=0;i<BANK_SIZE;i++)
	{
		if(g_pstAns->SNR_post_1[i]<g_pstAns->snr_post_th)
		{
			g_pstAns->I_1[i]=1;
		}
		else
		{
			g_pstAns->I_1[i]=0;
		}
	}
	k=0;
	for(i=0;i<BANK_SIZE;i++)
	{
		if(g_pstAns->I_1[i]==1)
		{
			k=k+1;
			if(k>1)	break;
		}
	}
	if(k<=1)
	{
		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->pn_fifo_pre[i]=g_pstAns->px[i];
		}
	}
	else
	{
		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->pn_fifo_pre[i]=g_pstAns->pn[i];
		}
	}
	for(i=0;i<BANK_SIZE;i++)
	{
		g_pstAns->px_fifo_pre[i]=g_pstAns->px[i];
	}

	if(FIFO_DEPTH>1)
	{
		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->pn_fifo[g_pstAns->fifo_index][i]=g_pstAns->pn[i];
			g_pstAns->px_fifo[g_pstAns->fifo_index][i]=g_pstAns->px[i];
		}
		for(i=0; i<(BLOCK_SIZE/2+1); i++)
		{
			g_pstAns->micWin_f_real_fifo[g_pstAns->fifo_index][i]=g_pstAns->micWin_f_real[i];
			g_pstAns->micWin_f_imag_fifo[g_pstAns->fifo_index][i]=g_pstAns->micWin_f_imag[i];
		}

		g_pstAns->fifo_index = (g_pstAns->fifo_index + FIFO_DEPTH - 1) % FIFO_DEPTH;
		
		if(g_pstAns->first_frame>=(FIFO_DEPTH-1))
		{
			for(i=0;i<BANK_SIZE;i++)
			{
				temp_64u=0;
				for(j=0;j<FIFO_DEPTH;j++)
				{
					temp_64u = temp_64u + g_pstAns->pn_fifo[j][i];
				}
				g_pstAns->pn[i]=(unsigned long)(temp_64u/FIFO_DEPTH);
				g_pstAns->px[i]=g_pstAns->px_fifo[g_pstAns->fifo_index][i];
			}
			for(i=0; i<(BLOCK_SIZE/2+1); i++)
			{
				g_pstAns->micWin_f_real[i] = g_pstAns->micWin_f_real_fifo[g_pstAns->fifo_index][i];
				g_pstAns->micWin_f_imag[i] = g_pstAns->micWin_f_imag_fifo[g_pstAns->fifo_index][i];
			}
		}
	}
	//////	fifo operation end

	//////	spectral gain computation
	if(g_pstAns->first_frame<(FIFO_DEPTH-1))
	{
		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->SNR_post[i]=0;
			g_pstAns->SNR_post_pre[i]=0;
		}
	}
	else
	{
		if(g_pstAns->first_frame>=FIFO_DEPTH)
		{
			for(i=0;i<BANK_SIZE;i++)
			{
				g_pstAns->SNR_post_pre[i]=g_pstAns->SNR_post[i];
			}
		}
		for(i=0;i<BANK_SIZE;i++)
		{
			if(g_pstAns->pn[i]==0)
			{
				g_pstAns->SNR_post[i] = MAX_32U;	//Q24
			}
			else
			{
				temp_64u=(((unsigned long long)g_pstAns->px[i])<<24)/g_pstAns->pn[i];
				if(temp_64u>MAX_32U)
				{
					g_pstAns->SNR_post[i] = MAX_32U;
				}
				else
				{
					g_pstAns->SNR_post[i] = (unsigned long)temp_64u;
				}
			}
		}
		if(g_pstAns->first_frame==(FIFO_DEPTH-1))
		{
			for(i=0;i<BANK_SIZE;i++)
			{
				g_pstAns->SNR_post_pre[i]=g_pstAns->SNR_post[i];
			}
		}	
	}

	for(i=0;i<BANK_SIZE;i++)
	{
		if(g_pstAns->SNR_post[i]<=0x1000000)
		{
			g_pstAns->SNR_pri[i]=0;
		}
		else
		{
			temp_64u=(unsigned long long)(0x10000-(unsigned long)(g_pstAns->alpha_snr_pri))*(unsigned long long)(g_pstAns->SNR_post[i]-0x1000000);
			g_pstAns->SNR_pri[i]=(unsigned long)(temp_64u>>16);
		}
		temp_64u=(unsigned long long)g_pstAns->alpha_snr_pri*(unsigned long long)g_pstAns->SNR_post_pre[i];
		temp_32u=(unsigned long)(temp_64u>>16);
		temp_64u=(unsigned long long)g_pstAns->gain_nb[i]*(unsigned long long)g_pstAns->gain_nb[i];
		temp_64u=(unsigned long long)((temp_64u>>31)*(unsigned long long)temp_32u);
		g_pstAns->SNR_pri[i]=(unsigned long)(temp_64u>>31)+g_pstAns->SNR_pri[i];
	}
	for(i=0;i<BANK_SIZE;i++)
	{
		g_pstAns->gain_nb_pre[i]=g_pstAns->gain_nb[i];
	}
	for(i=0;i<BANK_SIZE;i++)
	{
		if(g_pstAns->SNR_pri[i]>0xfeffffff)
		{
			temp_32u = MAX_32U;
		}
		else
		{
			temp_32u = (g_pstAns->SNR_pri[i]+0x1000000);
		}
		g_pstAns->gain_nb[i]=(unsigned long)((((unsigned long long)g_pstAns->SNR_pri[i])<<31)/temp_32u);
	}

	temp_64u=0;
	for(i=1;i<(BANK_SIZE-1);i++)
	{
		temp_64u = temp_64u+g_pstAns->px[i];
	}
	g_pstAns->px_avg=(unsigned long)(temp_64u/(BANK_SIZE-2));
	g_pstAns->gain_nb_min_pre = g_pstAns->gain_nb_min;
	g_pstAns->px_avg_sqrt=sqrt_16b(g_pstAns->px_avg);
	if(g_pstAns->px_avg_sqrt==0)
	{
		g_pstAns->gain_nb_min = 0x80000000;
	}
	else
	{
		temp_64u=(((unsigned long long)g_pstAns->noise_min_thd)<<31)/g_pstAns->px_avg_sqrt;
		temp_64u=temp_64u>>LOG_2_BLOCK;
		if(temp_64u>0x80000000)
		{
			g_pstAns->gain_nb_min = 0x80000000;
		}
		else
		{
			g_pstAns->gain_nb_min = (unsigned long)temp_64u;
		}
	}

	temp_64u = (unsigned long long)g_pstAns->gain_nb_min*(unsigned long long)(0x10000-g_pstAns->alpha_gain_nb_min);
	temp_64u = temp_64u + (unsigned long long)g_pstAns->gain_nb_min_pre*(unsigned long long)g_pstAns->alpha_gain_nb_min;
	g_pstAns->gain_nb_min = (unsigned long)(temp_64u>>16);

	for(i=0;i<BANK_SIZE;i++)
	{
		if(g_pstAns->gain_nb[i]<g_pstAns->gain_nb_min)
		{
			g_pstAns->gain_nb[i]=g_pstAns->gain_nb_min;
		}
	}

	if(g_pstAns->first_frame<(FIFO_DEPTH-1))
	{
		for(i=0;i<BANK_SIZE;i++)
		{
			g_pstAns->gain_nb[i]=0x80000000;
		}
	}

	for(i=0;i<BANK_SIZE;i++)
	{
		k = 0;
		h = INIT_LOG_SEARCH;
		for(j=0;j<LOG_DB_BITS; j++)
		{
			if(g_pstAns->gain_nb[i]<log_10_db[k+h-1])
			{
				k=k+h;
			}
			h=h>>1;
		}
		g_pstAns->gain_nb_db[i]=k;
		if(g_pstAns->gain_nb_db[i]>g_pstAns->gain_max_db)
		{
			g_pstAns->gain_nb_db[i]=g_pstAns->gain_max_db;
		}
	}

	temp_16u=(g_pstAns->alpha_gain_max-g_pstAns->alpha_gain_min);
	temp_16u=temp_16u/g_pstAns->gain_max_db;
	for(i=0;i<BANK_SIZE;i++)
	{
		temp_32u=(unsigned long)temp_16u*(unsigned long)g_pstAns->gain_nb_db[i];
		g_pstAns->alpha_gain[i]=g_pstAns->alpha_gain_min+((unsigned short)temp_32u);
	}

	if(g_pstAns->first_frame>=(FIFO_DEPTH-1))
	{
		for(i=0; i<BANK_SIZE; i++)
		{
			temp_64u=(unsigned long long)(0x10000-g_pstAns->alpha_gain[i])*(unsigned long long)g_pstAns->gain_nb[i];
			temp_64u=temp_64u>>16;
			g_pstAns->gain_nb[i]=(unsigned long)temp_64u;
			temp_64u=(unsigned long long)g_pstAns->gain_nb_pre[i]*(unsigned long long)g_pstAns->alpha_gain[i];
			temp_64u=temp_64u>>16;
			g_pstAns->gain_nb[i]=(unsigned long)(temp_64u)+g_pstAns->gain_nb[i];
		}
	}
	for(i=0;i<BANK_SIZE;i++)
	{
		for(j=BANK_F[i];j<BANK_F[i+1];j++)
		{
			g_pstAns->gain[j-1]=g_pstAns->gain_nb[i];
		}
	}
	//////	spectral gain computation end

	//////	ifft
	if(g_pstAns->first_frame<(FIFO_DEPTH-1))
	{
		for(i=0;i<(BLOCK_SIZE/2+1);i++)
		{
			g_pstAns->micWin_f_real[i]=0;
			g_pstAns->micWin_f_imag[i]=0;
		}
	}

	for(i=0;i<(BLOCK_SIZE/2+1);i++)
	{
		temp_64=(int64_t)g_pstAns->micWin_f_real[i]*(int64_t)g_pstAns->gain[i];
		temp_64=temp_64>>30;
		g_pstAns->micWin_f_real[i]=(long)temp_64;
		temp_64=(int64_t)g_pstAns->micWin_f_imag[i]*(int64_t)g_pstAns->gain[i];
		temp_64=temp_64>>30;
		g_pstAns->micWin_f_imag[i]=(long)temp_64;
	}
	for(i=1; i<(BLOCK_SIZE/2); i++)
	{
		g_pstAns->micWin_f_real[BLOCK_SIZE-i] = g_pstAns->micWin_f_real[i];
		g_pstAns->micWin_f_imag[BLOCK_SIZE-i] = -g_pstAns->micWin_f_imag[i];
	}


	_ans_ditFFT(BLOCK_SIZE, 1, LOG_2_BLOCK, g_pstAns->micWin_f_real, g_pstAns->micWin_f_imag); 
	//////	ifft end

	//////	overlap add
	if(g_pstAns->first_frame<=(FIFO_DEPTH-1))
	{
		for(i=0; i<(FRAME_SIZE<<2); i++)
		{
			temp_32 = (long)(((short)(g_pstAns->micWin_f_real[i]>>16)))*(long)OL_SCALE;
			temp_32 = temp_32>>16;
			g_pstAns->youtBuffer[i] = (short)(temp_32);
		}
	}
	else
	{
		for(i=0; i<(FRAME_SIZE<<2); i++)
		{
			temp_32 = (long)(((short)(g_pstAns->micWin_f_real[i]>>16)))*(long)OL_SCALE;
			temp_32 = temp_32>>16;
			if(i<((FRAME_SIZE<<2)-FRAME_SIZE))
			{
				g_pstAns->youtBuffer[i] = g_pstAns->youtBuffer[i+FRAME_SIZE]+(short)temp_32;
			}
			else
			{
				g_pstAns->youtBuffer[i] = (short)(temp_32);
			}
		}
	}
	//////	overlap add end

	for(i=0;i<FRAME_SIZE;i++)
	{
		yout[i]	= g_pstAns->youtBuffer[i];
	}

	g_pstAns->mic_in_frame_index = (g_pstAns->mic_in_frame_index + 3)%4;

	if(g_pstAns->first_frame<V)
	{
		g_pstAns->first_frame = g_pstAns->first_frame+1;
	}
}

void hs_ans_init(void)
{
  uint32_t i, j;
  
  if(g_pstAns)
  {
    hs_free(g_pstAns);
    g_pstAns = NULL;
  }

  g_pstAns = (hs_ansvar_t *)hs_malloc(sizeof(hs_ansvar_t), __MT_Z_GENERAL);
  if(!g_pstAns)
    return ;

  for (i=0;i<4;i++)
	{
		for(j=0;j<FRAME_SIZE;j++)
		{
			g_pstAns->micIn[i][j]=0;
		}
	}

	g_pstAns->mic_in_frame_index = 0;

	for(i=0;i<BLOCK_SIZE;i++)
	{
		g_pstAns->youtBuffer[i]   = 0;
		g_pstAns->micWin[i]       = 0;
	}

	g_pstAns->fifo_index = 0;

	for(i=0;i<BANK_SIZE;i++)
	{
		g_pstAns->pmin_act[i]     = MAX_32U;
		g_pstAns->pmin_act_sub[i] = MAX_32U;
		g_pstAns->pmin[i]         = MAX_32U;
		g_pstAns->gain_nb[i]      = 0x8000;
	}

	g_pstAns->first_frame = 0;
	g_pstAns->subwc = 1;

	for(i=0;i<U;i++)
	{
		for(j=0;j<BANK_SIZE;j++)
		{
			g_pstAns->pmin_sub[i][j]= MAX_32U;
		}
	}
	g_pstAns->pmin_sub_index = 0;

	for(i=0;i<FIFO_SIZE;i++)
	{
		for(j=0;j<(BLOCK_SIZE/2+1);j++)
		{
			g_pstAns->micWin_f_real_fifo[i][j]=0;
			g_pstAns->micWin_f_imag_fifo[i][j]=0;
		}
		for(j=0;j<BANK_SIZE;j++)
		{
			g_pstAns->pn_fifo[i][j]=0;
			g_pstAns->px_fifo[i][j]=0;
		}
	}

#if 1
	g_pstAns->alpha_max         = 0xF5C3;
	g_pstAns->cmax1             = 0xB333;
	g_pstAns->cmax              = 0xB333;
	g_pstAns->alpha_c           = 0xFFFF;
	g_pstAns->alpha_min         = 0x4CCD;
	g_pstAns->Md                = 0xCC16;
	g_pstAns->Md_sub            = 0x9F42;
	//alpha_v = 0x87AE;
	g_pstAns->alpha_v           = 0xc000;
	g_pstAns->snr_post_th       = 0x6666;
	g_pstAns->alpha_snr_pri     = 0xF5C3;
	//gain_nb_min = 0x6666666;
	g_pstAns->gain_nb_min       = 0x666;  
	g_pstAns->gain_max_db       = 0x002c;
	g_pstAns->alpha_gain_max    = 0xFD71;
	g_pstAns->alpha_gain_min    = 0x8000;
	g_pstAns->alpha_gain_nb_min = 0xE666;

  // ( 10 ^ (db / 20) ) * 2 ^ 16
	//g_pstAns->noise_min_thd     = 0x170;  //-45db 
	g_pstAns->noise_min_thd     = 0x818;  //-30db 
#else
	g_pstAns->alpha_max         = cfgp->alpha_max;
	g_pstAns->cmax1             = cfgp->cmax1;
	g_pstAns->cmax              = cfgp->cmax;
	g_pstAns->alpha_c           = cfgp->alpha_c;
	g_pstAns->alpha_min         = cfgp->alpha_min;
	g_pstAns->Md                = cfgp->Md;
	g_pstAns->Md_sub            = cfgp->Md_sub;
	g_pstAns->alpha_v           = cfgp->alpha_v;
	g_pstAns->snr_post_th       = cfgp->snr_post_th;
	g_pstAns->alpha_snr_pri     = cfgp->alpha_snr_pri;
	g_pstAns->gain_nb_min       = cfgp->gain_nb_min;
	g_pstAns->noise_min_thd     = cfgp->noise_min_thd;
	g_pstAns->gain_max_db       = cfgp->gain_max_db;
	g_pstAns->alpha_gain_max    = cfgp->alpha_gain_max;
	g_pstAns->alpha_gain_min    = cfgp->alpha_gain_min;
	g_pstAns->alpha_gain_nb_min = cfgp->alpha_gain_nb_min;
#endif
}

void hs_ans_uninit(void)
{
  if(g_pstAns)
  {
    hs_free(g_pstAns);
    g_pstAns = NULL;
  }
}



#endif

/** @} */
