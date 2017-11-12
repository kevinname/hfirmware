
#if PLAYER_INC_WMA && HS_USE_MP3

#include <string.h>
#include "libwma/asf.h"
#include "libwma/wmadec.h"
#include "lib.h"

#if 0
static const guid_t asf_guid_null =
{0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

/* top level object guids */
static const guid_t asf_guid_header =
{0x75B22630, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}};

static const guid_t asf_guid_index =
{0x33000890, 0xE5B1, 0x11CF, {0x89, 0xF4, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB}};
#endif

static const guid_t asf_guid_data =
{0x75B22636, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}};

/* header level object guids */
static const guid_t asf_guid_file_properties =
{0x8cabdca1, 0xa947, 0x11cf, {0x8E, 0xe4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}};

static const guid_t asf_guid_stream_properties =
{0xB7DC0791, 0xA9B7, 0x11CF, {0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}};

static const guid_t asf_guid_content_description =
{0x75B22633, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}};

static const guid_t asf_guid_extended_content_description =
{0xD2D0A440, 0xE307, 0x11D2, {0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50}};

static const guid_t asf_guid_content_encryption =
{0x2211b3fb, 0xbd23, 0x11d2, {0xb4, 0xb7, 0x00, 0xa0, 0xc9, 0x55, 0xfc, 0x6e}};

static const guid_t asf_guid_extended_content_encryption =
{0x298ae614, 0x2622, 0x4c17, {0xb9, 0x35, 0xda, 0xe0, 0x7e, 0xe9, 0x28, 0x9c}};

/* stream type guids */
static const guid_t asf_guid_stream_type_audio =
{0xF8699E40, 0x5B4D, 0x11CF, {0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}};

size_t stream_buffer_read(struct stream_buffer* stream, uint8_t* ptr, size_t len)
{
  hs_adec_t *pstAdec = &stream->dec->stAdec;
	size_t rest_bytes;

  if(len > stream->buffer_size || stream->eof == TRUE)
    return 0;

  stream->read_buf_ptr += len;
	if (stream->position + len > stream->length){
		rest_bytes = stream->length - stream->position;
		memcpy(ptr, &stream->buffer[stream->position], rest_bytes);
		ptr += rest_bytes;

		/* read more buffer */
		//f_read(stream->fd, &stream->buffer[0], stream->buffer_size, (UINT*)&stream->length);
		stream->length = pstAdec->pfnFetchData(pstAdec, &stream->buffer[0], stream->buffer_size);

		if (stream->length < len - rest_bytes){
			stream->position = stream->length;
			stream->eof = TRUE;
		}
		else{
			stream->position = len - rest_bytes;
    }

		memcpy(ptr, &stream->buffer[0], stream->position);
		return rest_bytes + stream->position;
	}

	memcpy(ptr, &stream->buffer[stream->position], len);
	stream->position += len;

	return len;
}

void stream_buffer_advance(struct stream_buffer* stream, size_t size)
{
  hs_adec_t *pstAdec = &stream->dec->stAdec;
  stream->read_buf_ptr += size;

	if (stream->position + size < stream->length)
  {
		stream->position += size;
		return;
	}

  stream->position = size - (stream->length - stream->position);
	//f_read(stream->fd, stream->buffer, stream->buffer_size, (UINT*)&stream->length);
	stream->length = pstAdec->pfnFetchData(pstAdec, &stream->buffer[0], stream->buffer_size);
	if (stream->length < stream->position)
    stream->eof = TRUE;
};

uint8_t *stream_buffer_request(struct stream_buffer* stream, size_t *r_size, size_t size)
{
  hs_adec_t *pstAdec = &stream->dec->stAdec;
	size_t rest_bytes, read_bytes;

	if (size > stream->buffer_size){
    return NULL; /* request size more than the buffer size */
  }

	if (stream->position + size < stream->length){
		*r_size = size;
		return &stream->buffer[stream->position];
	}

	/* read more data */
	rest_bytes = stream->length - stream->position;
	memmove(&stream->buffer[0], &stream->buffer[stream->position], rest_bytes);

	read_bytes = stream->buffer_size - rest_bytes;
	//read_bytes = (read_bytes / 512) * 512; /* align to 512 */

	//f_read(stream->fd, &stream->buffer[rest_bytes], read_bytes, (UINT*)&read_bytes);
	stream->length = pstAdec->pfnFetchData(pstAdec, &stream->buffer[0], stream->buffer_size);
	stream->position = 0;
	stream->length = read_bytes + rest_bytes;
	*r_size = size;
  
  if(stream->length != stream->buffer_size)
   return NULL;

	return &stream->buffer[0];
}

struct stream_buffer* stream_buffer_create(WMADecodeContext *s)
{
	struct stream_buffer* buffer;

  buffer = (struct stream_buffer*) hs_malloc(sizeof(struct stream_buffer), __MT_GENERAL);
	if (!buffer)
    return NULL;

  buffer->buffer = hs_malloc(buffer->buffer_size, __MT_GENERAL);
  if(!buffer->buffer)
  {
    hs_free(buffer);
    buffer = NULL;
  }

  buffer->eof          = FALSE;
  buffer->length       = 0;
  buffer->position     = 0;
  buffer->read_buf_ptr = 0;
  buffer->buffer_size  = STREAM_BUFFER_SIZE;
  buffer->dec          = s;

	return buffer;
}

void stream_buffer_close(struct stream_buffer* stream)
{
  if(stream->buffer)
	  hs_free(stream->buffer);

  hs_free(stream);
}

static int asf_guid_match(const guid_t *guid1, const guid_t *guid2)
{
  if((guid1->v1 != guid2->v1) || (guid1->v2 != guid2->v2) ||
       (guid1->v3 != guid2->v3) ||(memcmp(guid1->v4, guid2->v4, 8))){
    return 0;
  }

  return 1;
}

/* Read the 16 byte GUID from a file */
static void asf_readGUID(struct stream_buffer* stream, guid_t* guid)
{
  read_uint32le(stream, &guid->v1);
  read_uint16le(stream, &guid->v2);
  read_uint16le(stream, &guid->v3);
  stream_buffer_read(stream, guid->v4, 8);
}

static void asf_read_object_header(asf_object_t *obj, struct stream_buffer* stream)
{
  asf_readGUID(stream, &obj->guid);
  read_uint64le(stream, &obj->size);
  obj->datalen = 0;
}

/* Parse an integer from the extended content object - we always
convert to an int, regardless of native format.
*/
static int asf_intdecode(struct stream_buffer* stream, int type, int length)
{
  uint16_t tmp16;
  uint32_t tmp32;
  uint64_t tmp64;

  if (type==3) {
    read_uint32le(stream, &tmp32);
    stream_buffer_advance(stream,length - 4);
    return (int)tmp32;
  } else if (type==4) {
    read_uint64le(stream, &tmp64);
    stream_buffer_advance(stream,length - 8);
    return (int)tmp64;
  } else if (type == 5) {
    read_uint16le(stream, &tmp16);
    stream_buffer_advance(stream,length - 2);
    return (int)tmp16;
  }

  return 0;
}

#define MASK   0xC0 /* 11000000 */
#define COMP   0x80 /* 10x      */
static const unsigned char utf8comp[6] ={
  0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
};

/* Encode a UCS value as UTF-8 and return a pointer after this UTF-8 char. */
unsigned char* utf8encode(unsigned long ucs, unsigned char *utf8)
{
  int tail = 0;

  if (ucs > 0x7F){
    while (ucs >> (5*tail + 6)){
      tail++;
    }
  }

  *utf8++ = (ucs >> (6*tail)) | utf8comp[tail];
  while (tail--){
    *utf8++ = ((ucs >> (6*tail)) & (MASK ^ 0xFF)) | COMP;
  }

  return utf8;
}

/* Decode a LE utf16 string from a disk buffer into a fixed-sized
   utf8 buffer.
*/
static void asf_utf16LEdecode(struct stream_buffer* stream,
                              uint16_t utf16bytes,
                              unsigned char **utf8,
                              int* utf8bytes)
{
  unsigned long ucs;
  int n;
  unsigned char utf16buf[256];
  unsigned char* utf16 = utf16buf;
  unsigned char* newutf8;

  n = stream_buffer_read(stream, utf16buf, MIN(sizeof(utf16buf), utf16bytes));
  utf16bytes -= n;

  while (n > 0) {
    /* Check for a surrogate pair */
    if (utf16[1] >= 0xD8 && utf16[1] < 0xE0) {
      if (n < 4) {
        /* Run out of utf16 bytes, read some more */
        utf16buf[0] = utf16[0];
        utf16buf[1] = utf16[1];

        n = stream_buffer_read(stream, utf16buf + 2, MIN(sizeof(utf16buf)-2, utf16bytes));
        utf16 = utf16buf;
        utf16bytes -= n;
        n += 2;
      }

      if (n < 4) {
        /* Truncated utf16 string, abort */
        break;
      }
      ucs = 0x10000 + ((utf16[0] << 10) | ((utf16[1] - 0xD8) << 18)
                       | utf16[2] | ((utf16[3] - 0xDC) << 8));
      utf16 += 4;
      n -= 4;
    } else {
      ucs = (utf16[0] | (utf16[1] << 8));
      utf16 += 2;
      n -= 2;
    }

    if (*utf8bytes > 6) {
      newutf8 = utf8encode(ucs, *utf8);
      *utf8bytes -= (newutf8 - *utf8);
      *utf8 += (newutf8 - *utf8);
    }

    /* We have run out of utf16 bytes, read more if available */
    if ((n == 0) && (utf16bytes > 0)) {
      n = stream_buffer_read(stream, utf16buf, MIN(sizeof(utf16buf), utf16bytes));
      utf16 = utf16buf;
      utf16bytes -= n;
    }
  }

  *utf8[0] = 0;
  --*utf8bytes;

  if (utf16bytes > 0) {
    /* Skip any remaining bytes */
    stream_buffer_advance(stream, utf16bytes);
  }
  return;
}

static int asf_parse_header(struct stream_buffer* stream, struct id3_tag* id3,
                            asf_waveformatex_t* wfx)
{
  asf_object_t current;
  asf_object_t header;
  uint64_t datalen;
  int i;
  int fileprop = 0;
  uint64_t play_duration;
  uint16_t flags;
  uint32_t subobjects;
  uint8_t utf8buf[512];
  int id3buf_remaining = sizeof(id3->id3v2buf) + sizeof(id3->id3v1buf);
  unsigned char* id3buf = (unsigned char*)id3->id3v2buf;

  asf_read_object_header((asf_object_t *) &header, stream);

  //debug("header.size=%d\r\n",(int)header.size);
  if (header.size < 30) {
    /* invalid size for header object */
    return ASF_ERROR_OBJECT_SIZE;
  }

  read_uint32le(stream, &subobjects);

  /* Two reserved bytes - do we need to read them? */
  stream_buffer_advance(stream, 2);

  //debug("Read header - size=%d, subobjects=%d\r\n",(int)header.size, (int)subobjects);

  if (subobjects > 0) {
    header.datalen = header.size - 30;

    /* TODO: Check that we have datalen bytes left in the file */
    datalen = header.datalen;

    for (i=0; i<(int)subobjects; i++) {
      //debug("Parsing header object %d - datalen=%d\r\n",i,(int)datalen);
      if (datalen < 24) {
        //debug("not enough data for reading object\r\n");
        break;
      }

      asf_read_object_header(&current, stream);

      if (current.size > datalen || current.size < 24) {
        //debug("invalid object size - current.size=%d, datalen=%d\r\n",(int)current.size,(int)datalen);
        break;
      }

      if (asf_guid_match(&current.guid, &asf_guid_file_properties)) {
        if (current.size < 104)
          return ASF_ERROR_OBJECT_SIZE;

        if (fileprop) {
          /* multiple file properties objects not allowed */
          return ASF_ERROR_INVALID_OBJECT;
        }

        fileprop = 1;
        /* All we want is the play duration - uint64_t at offset 40 */
        stream_buffer_advance(stream, 40);

        read_uint64le(stream, &play_duration);
        id3->length = play_duration / 10000;

        //debug("****** length = %lums\r\n", id3->length);

        /* Read the packet size - uint32_t at offset 68 */
        stream_buffer_advance(stream, 20);
        read_uint32le(stream, &wfx->packet_size);

        /* Skip bytes remaining in object */
        stream_buffer_advance(stream, current.size - 24 - 72);
      }
      else if (asf_guid_match(&current.guid, &asf_guid_stream_properties)) {
        guid_t guid;
        uint32_t propdatalen;

        if (current.size < 78)
          return ASF_ERROR_OBJECT_SIZE;

        asf_readGUID(stream, &guid);

        stream_buffer_advance(stream, 24);
        read_uint32le(stream, &propdatalen);
        stream_buffer_advance(stream, 4);
        read_uint16le(stream, &flags);

        if (!asf_guid_match(&guid, &asf_guid_stream_type_audio)) {
          //debug("Found stream properties for non audio stream, skipping\r\n");
          stream_buffer_advance(stream,current.size - 24 - 50);
        }
        else if (wfx->audiostream == -1) {
          stream_buffer_advance(stream, 4);
          //debug("Found stream properties for audio stream %d\r\n",flags&0x7f);

          if (propdatalen < 18) {
            return ASF_ERROR_INVALID_LENGTH;
          }

          read_uint16le(stream, &wfx->codec_id);
          read_uint16le(stream, &wfx->channels);
          read_uint32le(stream, &wfx->rate);
          read_uint32le(stream, &wfx->bitrate);
          wfx->bitrate *= 8;
          read_uint16le(stream, &wfx->blockalign);
          read_uint16le(stream, &wfx->bitspersample);
          read_uint16le(stream, &wfx->datalen);

          /* Round bitrate to the nearest kbit */
          id3->bitrate = (wfx->bitrate + 500) / 1000;
          id3->frequency = wfx->rate;

          if (wfx->codec_id == ASF_CODEC_ID_WMAV1) {
            stream_buffer_read(stream, wfx->data, 4);
            stream_buffer_advance(stream, current.size - 24 - 72 - 4);
            wfx->audiostream = flags&0x7f;
          } else if (wfx->codec_id == ASF_CODEC_ID_WMAV2) {
            stream_buffer_read(stream, wfx->data, 6);
            stream_buffer_advance(stream,current.size - 24 - 72 - 6);
            wfx->audiostream = flags&0x7f;
          } else {
            //debug("Unsupported WMA codec (Pro, Lossless, Voice, etc)\r\n");
            stream_buffer_advance(stream,current.size - 24 - 72);
          }
        }
      }
      else if (asf_guid_match(&current.guid, &asf_guid_content_description)) {
        /* Object contains five 16-bit string lengths, followed by the five strings:
        title, artist, copyright, description, rating
        */
        uint16_t strlength[5];
        int i;

        //debug("Found GUID_CONTENT_DESCRIPTION - size=%d\r\n",(int)(current.size - 24));

        /* Read the 5 string lengths - number of bytes included trailing zero */
        for (i=0; i<5; i++) {
          read_uint16le(stream, &strlength[i]);
          //debug("strlength = %u\r\n",strlength[i]);
        }

        if (strlength[0] > 0) {  /* 0 - Title */
          id3->title = (char* )id3buf;
          asf_utf16LEdecode(stream, strlength[0], &id3buf, &id3buf_remaining);
        }

        if (strlength[1] > 0) {  /* 1 - Artist */
          id3->artist = (char* )id3buf;
          asf_utf16LEdecode(stream, strlength[1], &id3buf, &id3buf_remaining);
        }

        stream_buffer_advance(stream, strlength[2]); /* 2 - copyright */

        if (strlength[3] > 0) {  /* 3 - description */
          id3->comment = (char* )id3buf;
          asf_utf16LEdecode(stream, strlength[3], &id3buf, &id3buf_remaining);
        }

        stream_buffer_advance(stream, strlength[4]); /* 4 - rating */
      }
      else if (asf_guid_match(&current.guid, &asf_guid_extended_content_description)) {
        uint16_t count;
        int i;
        int bytesleft = current.size - 24;
        //debug("Found GUID_EXTENDED_CONTENT_DESCRIPTION\r\n");

        read_uint16le(stream, &count);
        bytesleft -= 2;
        //debug("extended metadata count = %u\r\n",count);

        for (i=0; i < count; i++) {
          uint16_t length, type;
          unsigned char* utf8 = utf8buf;
          int utf8length = 512;

          read_uint16le(stream, &length);
          asf_utf16LEdecode(stream, length, &utf8, &utf8length);
          bytesleft -= 2 + length;

          read_uint16le(stream, &type);
          read_uint16le(stream, &length);

          if (!strcmp("WM/TrackNumber", (char *)utf8buf)) {
            if (type == 0) {
              id3->track_string = (char *)id3buf;
              asf_utf16LEdecode(stream, length, &id3buf, &id3buf_remaining);
              id3->tracknum = atoi(id3->track_string);
            } else if ((type >=2) && (type <= 5)) {
              id3->tracknum = asf_intdecode(stream, type, length);
            } else {
              stream_buffer_advance(stream, length);
            }
          }
          else if ((!strcmp("WM/Genre", (char *)utf8buf)) && (type == 0)) {
            id3->genre_string = (char *)id3buf;
            asf_utf16LEdecode(stream, length, &id3buf, &id3buf_remaining);
          }
          else if ((!strcmp("WM/AlbumTitle", (char *)utf8buf)) && (type == 0)) {
            id3->album = (char *)id3buf;
            asf_utf16LEdecode(stream, length, &id3buf, &id3buf_remaining);
          }
          else if ((!strcmp("WM/AlbumArtist", (char *)utf8buf)) && (type == 0)) {
            id3->albumartist = (char *)id3buf;
            asf_utf16LEdecode(stream, length, &id3buf, &id3buf_remaining);
          }
          else if ((!strcmp("WM/Composer", (char *)utf8buf)) && (type == 0)) {
            id3->composer = (char *)id3buf;
            asf_utf16LEdecode(stream, length, &id3buf, &id3buf_remaining);
          }
          else if (!strcmp("WM/Year", (char *)utf8buf)) {
            if (type == 0) {
              id3->year_string = (char *)id3buf;
              asf_utf16LEdecode(stream, length, &id3buf, &id3buf_remaining);
              id3->year = atoi(id3->year_string);
            } else if ((type >=2) && (type <= 5)) {
              id3->year = asf_intdecode(stream, type, length);
            } else {
              stream_buffer_advance(stream, length);
            }
          }
          else if (!strncmp("replaygain_", (char *)utf8buf, 11)) {
            char* value = (char *)id3buf;
            int buf_len = id3buf_remaining;
            int len;
            asf_utf16LEdecode(stream, length, &id3buf, &id3buf_remaining);
            len = 0; // parse_replaygain(utf8buf, value, id3, value, buf_len);

            if (len == 0) {
              /* Don't need to keep the value */
              id3buf = (unsigned char*)value;
              id3buf_remaining = buf_len;
            }
          }
          else if (!strcmp("MusicBrainz/Track Id", (char *)utf8buf)) {
            id3->mb_track_id = (char *)id3buf;
            asf_utf16LEdecode(stream, length, &id3buf, &id3buf_remaining);
          }
          else {
            stream_buffer_advance(stream, length);
          }
          bytesleft -= 4 + length;
        }

        stream_buffer_advance(stream, bytesleft);
      } else if (asf_guid_match(&current.guid, &asf_guid_content_encryption)
                 || asf_guid_match(&current.guid, &asf_guid_extended_content_encryption)) {
      // debug("File is encrypted\r\n");
       return ASF_ERROR_ENCRYPTED;
     }
      else {
        //debug("Skipping %d bytes of object\r\n",(int)(current.size - 24));
        stream_buffer_advance(stream,current.size - 24);
      }

      //debug("Parsed object - size = %d\r\n",(int)current.size);
      datalen -= current.size;
    }

    if (i != (int)subobjects || datalen != 0) {
      //debug("header data doesn't match given subobject count\r\n");
      return ASF_ERROR_INVALID_VALUE;
    }

    //debug("%d subobjects read successfully\r\n", i);
  }

  //debug("header validated correctly\r\n");

  return 0;
}

bool get_asf_metadata(struct stream_buffer* stream, struct id3_tag* id3)
{
  int res;
  asf_object_t obj;
  asf_waveformatex_t* wfx;

	wfx = (asf_waveformatex_t*) hs_malloc(sizeof(asf_waveformatex_t), __MT_GENERAL);
	if (wfx == NULL) return FALSE;

  wfx->audiostream = -1;

  res = asf_parse_header(stream, id3, wfx);
  if (res < 0) {
    //debug("ASF: parsing error - %d\r\n",res);
    return FALSE;
  }

  if (wfx->audiostream == -1) {
    //debug("ASF: No WMA streams found\r\n");
    hs_free(wfx);
    return FALSE;
  }

  asf_read_object_header(&obj, stream);

  if (!asf_guid_match(&obj.guid, &asf_guid_data)) {
    //debug("ASF: No data object found\r\n");
    hs_free(wfx);
    return FALSE;
  }

  /* Store the current file position - no need to parse the header
  again in the codec.  The +26 skips the rest of the data object
  header.
  */
  id3->first_frame_offset = 26;
	/* set wfx to user data */
	id3->user_data = (uint32_t)wfx;

  stream_buffer_advance(stream, 26);
  return TRUE;
}


/* Read an unaligned 32-bit little endian long from buffer. */
static unsigned long get_long_le(void* buf)
{
  unsigned char* p = (unsigned char*) buf;

  return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

/* Read an unaligned 16-bit little endian short from buffer. */
static unsigned short get_short_le(void* buf)
{
  unsigned char* p = (unsigned char*) buf;

  return p[0] | (p[1] << 8);
}

#define GETLEN2b(bits) (((bits) == 0x03) ? 4 : bits)

#define GETVALUE2b(bits, data) \
  (((bits) != 0x03) ? ((bits) != 0x02) ? ((bits) != 0x01) ? \
    0 : *(data) : get_short_le(data) : get_long_le(data))

static int asf_read_packet(WMADecodeContext* s,
                           uint8_t** audiobuf,
                           int* audiobufsize,
                           int* packetlength,
                           asf_waveformatex_t* wfx)
{
  uint8_t tmp8, packet_flags, packet_property;
  int stream_id;
  int ec_length, opaque_data, ec_length_type;
  int datalen;
  uint8_t data[18];
  uint8_t* datap;
  uint32_t length;
  uint32_t padding_length;
  //uint32_t send_time;
  //uint16_t duration;
  uint16_t payload_count;
  int payload_length_type;
  uint32_t payload_hdrlen;
  int payload_datalen;
  int multiple;
  uint32_t replicated_length;
  uint32_t bytesread = 0;
  uint8_t* buf;
  size_t bufsize;
  int i;
  /*debug("Reading new packet at %d bytes ", (int)ci->curpos);*/

  if (stream_buffer_read(s->stream, &tmp8, 1) == 0) {
    return ASF_ERROR_EOF;
  }
  bytesread++;

  /* TODO: We need a better way to detect endofstream */
  if (tmp8 != 0x82) {
    //debug("Read failed:  packet did not sync\r\n");
    return -1;
  }

  if (tmp8 & 0x80) {
    ec_length = tmp8 & 0x0f;
    opaque_data = (tmp8 >> 4) & 0x01;
    ec_length_type = (tmp8 >> 5) & 0x03;

    if (ec_length_type != 0x00 || opaque_data != 0 || ec_length != 0x02) {
      //debug("incorrect error correction flags\r\n");
      return ASF_ERROR_INVALID_VALUE;
    }

    /* Skip ec_data */
    stream_buffer_advance(s->stream, ec_length);
    bytesread += ec_length;
  } else {
    ec_length = 0;
  }

  if (stream_buffer_read(s->stream, &packet_flags, 1) == 0) { return ASF_ERROR_EOF; }
  if (stream_buffer_read(s->stream, &packet_property, 1) == 0) { return ASF_ERROR_EOF; }
  bytesread += 2;

  datalen = GETLEN2b((packet_flags >> 1) & 0x03) +
    GETLEN2b((packet_flags >> 3) & 0x03) +
      GETLEN2b((packet_flags >> 5) & 0x03) + 6;

  if (stream_buffer_read(s->stream, data, datalen) == 0) {
    return ASF_ERROR_EOF;
  }

  bytesread += datalen;

  datap = data;
  length = GETVALUE2b((packet_flags >> 5) & 0x03, datap);
  datap += GETLEN2b((packet_flags >> 5) & 0x03);
  /* sequence value is not used */
  GETVALUE2b((packet_flags >> 1) & 0x03, datap);
  datap += GETLEN2b((packet_flags >> 1) & 0x03);
  padding_length = GETVALUE2b((packet_flags >> 3) & 0x03, datap);
  datap += GETLEN2b((packet_flags >> 3) & 0x03);
  //send_time = get_long_le(datap);
  datap += 4;
  //duration = get_short_le(datap);
  datap += 2;
  /*debug("and duration %d ms\r\n", duration);*/

  /* this is really idiotic, packet length can (and often will) be
  * undefined and we just have to use the header packet size as the size
  * value */
  if (!((packet_flags >> 5) & 0x03)) {
    length = wfx->packet_size;
  }

  /* this is also really idiotic, if packet length is smaller than packet
  * size, we need to manually add the additional bytes into padding length
  */
  if (length < wfx->packet_size) {
    padding_length += wfx->packet_size - length;
    length = wfx->packet_size;
  }

  if (length > wfx->packet_size) {
    //debug("packet with too big length value\r\n");
    return ASF_ERROR_INVALID_LENGTH;
  }

  /* check if we have multiple payloads */
  if (packet_flags & 0x01) {
    if (stream_buffer_read(s->stream, &tmp8, 1) == 0) {
      return ASF_ERROR_EOF;
    }
    payload_count = tmp8 & 0x3f;
    payload_length_type = (tmp8 >> 6) & 0x03;
    bytesread++;
  } else {
    payload_count = 1;
    payload_length_type = 0x02; /* not used */
  }

  if (length < bytesread) {
    //debug("header exceeded packet size, invalid file - length=%d, bytesread=%d\r\n",(int)length,(int)bytesread);
    /* FIXME: should this be checked earlier? */
    return ASF_ERROR_INVALID_LENGTH;
  }


  /* We now parse the individual payloads, and move all payloads
  belonging to our audio stream to a contiguous block, starting at
  the location of the first payload.
  */

  *audiobuf = NULL;
  *audiobufsize = 0;
  *packetlength = length - bytesread;

  if((buf = stream_buffer_request(s->stream, &bufsize, length)) == NULL){
    return ASF_ERROR_IO;
  }

  datap = buf;

  #define ASF_MAX_REQUEST (1L<<15) /* 32KB */
  if (bufsize != length && length >= ASF_MAX_REQUEST) {
    /* This should only happen with packets larger than 32KB (the
    guard buffer size).  All the streams I've seen have
    relatively small packets less than about 8KB), but I don't
    know what is expected.
    */
    //debug("Could not read packet (requested %d bytes, received %d), aborting\r\n",
    //      (int)length,(int)bufsize);
    return -1;
  }

  for (i=0; i<payload_count; i++) {
    stream_id = datap[0]&0x7f;
    datap++;
    bytesread++;

    payload_hdrlen = GETLEN2b(packet_property & 0x03)
        + GETLEN2b((packet_property >> 2) & 0x03)
        + GETLEN2b((packet_property >> 4) & 0x03);

    //debug("payload_hdrlen = %d\r\n",payload_hdrlen);
    if (payload_hdrlen > sizeof(data)) {
      //debug("Unexpectedly long datalen in data - %d\r\n",datalen);
      return ASF_ERROR_OUTOFMEM;
    }

    bytesread += payload_hdrlen;
    datap += GETLEN2b((packet_property >> 4) & 0x03);
    datap += GETLEN2b((packet_property >> 2) & 0x03);
    replicated_length = GETVALUE2b(packet_property & 0x03, datap);
    datap += GETLEN2b(packet_property & 0x03);

    /* TODO: Validate replicated_length */
    /* TODO: Is the content of this important for us? */
    datap += replicated_length;
    bytesread += replicated_length;

    multiple = packet_flags & 0x01;


    if (multiple) {
      int x;

      x = GETLEN2b(payload_length_type);

      if (x != 2) {
        /* in multiple payloads datalen should be a word */
        return ASF_ERROR_INVALID_VALUE;
      }

      payload_datalen = GETVALUE2b(payload_length_type, datap);
      datap += x;
      bytesread += x;
    } else {
      payload_datalen = length - bytesread - padding_length;
    }

    if (replicated_length==1)
      datap++;

    if (stream_id == wfx->audiostream)
    {
      if (*audiobuf == NULL) {
        /* The first payload can stay where it is */
        *audiobuf = datap;
        *audiobufsize = payload_datalen;
      } else {
        /* The second and subsequent payloads in this packet
        that belong to the audio stream need to be moved to be
        contiguous with the first payload.
        */
        memmove(*audiobuf + *audiobufsize, datap, payload_datalen);
        *audiobufsize += payload_datalen;
      }
    }
    datap += payload_datalen;
    bytesread += payload_datalen;
  }

  if (*audiobuf != NULL)
    return 1;
  else
    return 0;
}

static void _adec_wmaReinit(hs_adec_t *pstAdec)
{
  WMADecodeContext *wma_decoder = (WMADecodeContext *)pstAdec->decoder;
  
  wma_decoder->stream->eof = FALSE;
  wma_decoder->stream->length = 0;
  wma_decoder->stream->position = 0;
  wma_decoder->stream->read_buf_ptr = 0;
  wma_decoder->stream->buffer_size = STREAM_BUFFER_SIZE;
}

static int _adec_wmaSkipMetaData(hs_adec_t *pstAdec)
{
  WMADecodeContext *wma_decoder = (WMADecodeContext *)pstAdec->decoder;
  uint32_t flag = 0;
  
  /* get meta information */
  if(get_asf_metadata(wma_decoder->stream, &wma_decoder->id3) != FALSE)
  {
    wma_decoder->wfx = (asf_waveformatex_t*)wma_decoder->id3.user_data;
    wma_decode_init(wma_decoder);

     /* set sample rate */
    if (wma_decoder->sample_rate != (int)pstAdec->pstAo->stI2sCfg.sample_rate)
    {
      pstAdec->pstAo->stI2sCfg.sample_rate = (hs_i2s_sample_t)wma_decoder->sample_rate;
      audioSetPlaySample(pstAdec->pstAo->stI2sCfg.sample_rate);
    }

    if(wma_decoder->nb_channels == 1 && pstAdec->pstAo->stI2sCfg.i2s_mode != I2S_PCMMODE_MONO)
    {
      pstAdec->pstAo->stI2sCfg.i2s_mode = I2S_PCMMODE_MONO;

      flag = 1;
    }
    else if(wma_decoder->nb_channels == 2 && pstAdec->pstAo->stI2sCfg.i2s_mode != I2S_PCMMODE_STEREO)
    {
      pstAdec->pstAo->stI2sCfg.i2s_mode = I2S_PCMMODE_STEREO;
      
      flag = 1;
    }

    if(flag == 1)
    {
      hs_ao_stop(pstAdec->pstAo);
      hs_ao_start(pstAdec->pstAo);
    }  

    return ADEC_OK;
  }

  stream_buffer_close(wma_decoder->stream);
  wma_decoder->stream = NULL;
  return ADEC_ERR;
}

static void _adec_wmaDelete(hs_adec_t *pstAdec)
{
  WMADecodeContext* s = (WMADecodeContext *)pstAdec->decoder;

  if(s->stream)
    stream_buffer_close(s->stream);

  if(s->wfx)
    hs_free(s->wfx);

  if(s->decoded)
    hs_free(s->decoded);

  hs_free(s);
  pstAdec->decoder = NULL;
}

static int _adec_wmaRun(hs_adec_t *pstAdec)
{
  WMADecodeContext* s = (WMADecodeContext*)pstAdec->decoder;
  int audiobufsize, wmares, res, packetlength, i;
  uint8_t* audiobuf;

  res = asf_read_packet(s, &audiobuf, &audiobufsize, &packetlength, s->wfx);
  if (res < 0)
    return ADEC_ERR;
  
  if (res > 0) 
  {
    wma_decode_superframe_init(s, audiobuf, audiobufsize);
    
    for (i=0; i < s->nb_frames; i++)
    {
      wmares = wma_decode_superframe_frame(s, (int32_t *)s->decoded, audiobuf, audiobufsize);
      if (wmares > 0)
        pstAdec->pfnCarryAway((uint8_t *)s->decoded, (uint32_t)wmares * 4);
      else if (wmares < 0)
        return ADEC_ERR;
    }
  }  

  stream_buffer_advance(s->stream, packetlength);
  return ADEC_OK;
}

hs_adec_t *hs_adec_wmaCreate(void)
{
  WMADecodeContext *wma_decoder;
  hs_adec_t *pstAdec = NULL;

  wma_decoder = hs_malloc(sizeof(WMADecodeContext), __MT_GENERAL);
  if(!wma_decoder)
    return NULL;

  wma_decoder->decoded = hs_malloc(BLOCK_MAX_SIZE * MAX_CHANNELS, __MT_GENERAL);
  if(!wma_decoder->decoded)
  {
    hs_free(wma_decoder);
    return NULL;
  }

  wma_decoder->stream = stream_buffer_create(wma_decoder);
  if(!wma_decoder->stream)
  {
    hs_free(wma_decoder->decoded);
    hs_free(wma_decoder);
    return NULL;
  }

  pstAdec = &wma_decoder->stAdec;
      
  memcpy(pstAdec->flag, "wma", sizeof("wma"));
  pstAdec->pfnReinit  = _adec_wmaReinit;
  pstAdec->pfnRun     = _adec_wmaRun;
  pstAdec->pfnDestroy = _adec_wmaDelete;
  pstAdec->pfnSkip    = _adec_wmaSkipMetaData;
  pstAdec->decoder    = wma_decoder;

  return pstAdec;
}

#endif


