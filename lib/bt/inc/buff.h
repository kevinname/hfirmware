/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    buff.h
Abstract:
	This file includes the definition of buffer management functions.
---------------------------------------------------------------------------*/
#ifndef BUFF_H
#define BUFF_H

struct BuffStru {
	WORD len; /* length of whole buffer, including data before off */
	WORD off;
	UCHAR data[1];
};

#ifdef CONFIG_MEMORY_LEAK_DETECT
struct BuffStru * DBG_BuffNew(WORD size, WORD off, char* filename, int line);
#define BuffNew(A,B) DBG_BuffNew(A,B, __FILE__, __LINE__)
#else
struct BuffStru * BuffNew(WORD size, WORD off);
#endif
void BuffFree(struct BuffStru *buf);
struct BuffStru *BuffCopy(struct BuffStru *in);

UCHAR BuffRes(struct BuffStru *buf, INT16 delta);
UCHAR CopyFromBuf(UCHAR *data, struct BuffStru *buf, INT16 len);
#define CopyToBuf(data,buf,len) CopyFromBuf(data,buf,-len)

#define HDLPTR(buf)		((UINT32 *)(&(((struct BuffStru *)buf)->data[0])))
#define HCICTRL(buf)	(&(((struct BuffStru *)buf)->data[4]))/* for HCI_RESERVE */

#define BUFLEN(buf)  	(((struct BuffStru *)buf)->len)
#define BUFDATA(buf)	((UINT8 *)(((struct BuffStru *)buf)->data + ((struct BuffStru *)buf)->off))
#define BUFEND(buf)		((UINT8 *)(((struct BuffStru *)buf)->data + ((struct BuffStru *)buf)->len))/* BUFDATA+DATASIZE, 1st invalid octet */
#define DATASIZE(buf) 	(BUFLEN((struct BuffStru *)buf) - ((struct BuffStru *)buf)->off)

#endif
