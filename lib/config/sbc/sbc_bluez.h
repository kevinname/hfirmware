/*
 *
 *  Bluetooth low-complexity, subband codec (SBC) library
 *
 *  Copyright (C) 2004-2009  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2004-2005  Henryk Ploetz <henryk@ploetzli.ch>
 *  Copyright (C) 2005-2006  Brad Midgley <bmidgley@xmission.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#pragma once
#ifndef __SBC_H
#define __SBC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
/* sampling frequency */
#define SBC_FREQ_16000		0x00
#define SBC_FREQ_32000		0x01
#define SBC_FREQ_44100		0x02
#define SBC_FREQ_48000		0x03

/* blocks */
#define SBC_BLK_4		0x00
#define SBC_BLK_8		0x01
#define SBC_BLK_12		0x02
#define SBC_BLK_16		0x03

/* channel mode */
#define SBC_MODE_MONO		0x00
#define SBC_MODE_DUAL_CHANNEL	0x01
#define SBC_MODE_STEREO		0x02
#define SBC_MODE_JOINT_STEREO	0x03

/* allocation method */
#ifndef SBC_AM_LOUDNESS
#define SBC_AM_LOUDNESS		0x00
#endif

#ifndef SBC_AM_SNR
#define SBC_AM_SNR		0x01
#endif

/* subbands */
#define SBC_SB_4		0x00
#define SBC_SB_8		0x01

/* Data endianess */
#define SBC_LE			0x00
#define SBC_BE			0x01

struct sbc_struct {
	unsigned long flags;

	uint8_t frequency;
	uint8_t blocks;
	uint8_t subbands;
	uint8_t mode;
	uint8_t allocation;
	uint8_t bitpool;
	uint8_t endian;

	void *priv;
};

typedef struct sbc_struct sbc_t;

sbc_t * sbc_bluez_open(uint8_t *hdr);
//int sbc_bluez_reinit(sbc_t *sbc, unsigned long flags);
//int sbc_parse(sbc_t *sbc, void *input, int input_len);
int16_t sbc_bluez_decode(sbc_t *sbc, const uint8_t *input, uint16_t input_len, void *output,
		uint16_t output_len, uint16_t *len);
int16_t sbc_bluez_encode(sbc_t *sbc, const uint8_t *input, uint16_t input_len, void *output,
		uint16_t output_len, uint16_t *written);
int sbc_get_frame_length(sbc_t *sbc);
int sbc_get_frame_duration(sbc_t *sbc);
uint16_t sbc_get_codesize(sbc_t *sbc);
void sbc_bluez_close(sbc_t *sbc);

#ifdef __cplusplus
}
#endif

#endif /* __SBC_H */
