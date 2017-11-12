#ifndef __WMA_H__
#define __WMA_H__

#if PLAYER_INC_WMA && HS_USE_MP3

#include "adec.h"
#include "libwma/wmadec.h"

enum asf_error_e {
  ASF_ERROR_INTERNAL       = -1,  /* incorrect input to API calls */
  ASF_ERROR_OUTOFMEM       = -2,  /* some malloc inside program failed */
  ASF_ERROR_EOF            = -3,  /* unexpected end of file */
  ASF_ERROR_IO             = -4,  /* error reading or writing to file */
  ASF_ERROR_INVALID_LENGTH = -5,  /* length value conflict in input data */
  ASF_ERROR_INVALID_VALUE  = -6,  /* other value conflict in input data */
  ASF_ERROR_INVALID_OBJECT = -7,  /* ASF object missing or in wrong place */
  ASF_ERROR_OBJECT_SIZE    = -8,  /* invalid ASF object size (too small) */
  ASF_ERROR_SEEKABLE       = -9,  /* file not seekable */
  ASF_ERROR_SEEK           = -10, /* file is seekable but seeking failed */
  ASF_ERROR_ENCRYPTED      = -11  /* file is encrypted */
};

#ifndef MIN
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

/* always little endian */
#define read_uint16le(stream,buf) stream_buffer_read((stream), (uint8_t*)(buf), 2)
#define read_uint32le(stream,buf) stream_buffer_read((stream), (uint8_t*)(buf), 4)
#define read_uint64le(stream,buf) stream_buffer_read((stream), (uint8_t*)(buf), 8)

/* TODO: Just read the GUIDs into a 16-byte array, and use memcmp to compare */
struct guid_s {
  uint32_t v1;
  uint16_t v2;
  uint16_t v3;
  uint8_t  v4[8];
};
typedef struct guid_s guid_t;

struct asf_object_s {
  guid_t       guid;
  uint64_t     size;
  uint64_t     datalen;
};
typedef struct asf_object_s asf_object_t;

hs_adec_t *hs_adec_wmaCreate(void);
#endif
#endif
