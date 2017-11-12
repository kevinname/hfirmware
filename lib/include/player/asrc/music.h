#ifndef __AUDIO_MUSIC_H__
#define __AUDIO_MUSIC_H__

#if HS_USE_MP3

#include "ao.h"
#include "adec.h"

#define PLAYER_MSG_SIZE         32

#define PLAYER_SIGNAL_WAKEUP    1
#define PLAYER_SIGNAL_WAIT      1

#define PLAYER_SIGNAL_THDOVER   0x80
#define PLAYER_SIGNAL_THDWAIT   0x80

#define NAME_MAX_NUM          (sizeof(TCHAR) * _MAX_LFN)

#define MUSIC_DIR0            ((TCHAR *)_T("0:/"))
#define MUSIC_DIR1            ((TCHAR *)_T("1:/"))

enum
{
  MUSIC_FUN_PLAYMODE      = 0x20,
  MUSIC_FUN_EQ            ,
  MUSIC_FF                ,
  MUSIC_REW               ,
  MUSIC_FFREW_STOP        ,

  MUSIC_FUN_NUM
};

enum 
{
  MUSIC_EQ_STANDARD       = 0,
  MUSIC_EQ_ROCK           ,
  MUSIC_EQ_POP            ,
  MUSIC_EQ_CLASSIC        ,
  MUSIC_EQ_JAZZ           ,
  MUSIC_EQ_FOLK           ,

  MUSIC_EQ_SELF           ,

  MUSIC_EQ_NUM
}; 

/* sort mode options */
enum 
{ 
  SORT_ALPHA = 0, 
  SORT_ALPHA_REVERSED, 
  SORT_DATE, 
  SORT_DATE_REVERSED, 
  SORT_TYPE,   
  SORT_TYPE_REVERSED 
}; 

/* repeat mode options */
enum
{
  REPEAT_ALL = 0,
  REPEAT_ONE,
  REPEAT_OFF,
  REPEAT_SHUFFLE,

  REPEAT_MODE_NUM,
};

typedef enum
{
  PLAY_PAUSE              = 0,
  PLAY_STOP               ,
  PLAY_START              ,
  PLAY_NEXT               ,
  PLAY_PREV               ,
  PLAY_CONTINUE           ,
  PLAY_VOLUME_CHANGED     ,
  PLAY_OVER               ,
}hs_musicmsg_t;

typedef enum
{
  MUSIC_STATUS_STOPED    = 0,
  MUSIC_STATUS_PLAYING   ,
  MUSIC_STATUS_PAUSE     ,
  MUSIC_STATUS_TERMINATE ,
}hs_musicstatus_t;

typedef enum
{
  MUSIC_DEVICE_NULL       = 0,
  MUSIC_DEVICE_SD         ,
  MUSIC_DEVICE_UDISK      ,
}hs_musicdevs_t;

typedef struct _file_info
{
  uint32_t time;
  uint32_t indices;
}hs_fileinfo_t;

typedef struct 
{
  uint8_t sortType;
  uint8_t playMode;
  int8_t  volume;         /* in dB */
  int8_t  volume_step;
  
  uint8_t musicPos;       /* current music index in playing */
  uint8_t eqIdx;          /* current eq index */ 

  uint8_t  ff_rew_step;
  uint8_t  dirLvl;
  uint16_t mode_map;

  uint32_t sd_offset;
  uint32_t udisk_offset;

  uint32_t sd_t;
  uint32_t udisk_t;

  uint8_t  sd_name[NAME_MAX_NUM];
  uint8_t  udisk_name[NAME_MAX_NUM];
}hs_musicpara_t;

typedef struct _playlist_info
{
  hs_fileinfo_t      *pstFileInfo;
  hs_musicpara_t      stCfg;
  hs_adec_t          *pstAdec;
  hs_ao_t            *pstAo;
  osTimerId           pstTimer;
  void               *pTimerArg;
  
  hs_musicstatus_t    eStatus;

  FIL                *pstFid;

  osMessageQDef_t     stMsgDef;
  osMessageQId        pstMsgId;
  osThreadId          pstThd;
  
  uint8_t             defaultEqNum; 
  uint8_t             saved;
  int                 max_playlist_size;  /* Max number of files in playlist*/
  TCHAR              *buffer;             /* buffer for in-ram playlists */
  int                 buffer_size;        /* size of buffer */
  int                 buffer_end_pos;     /* last position where buffer was written  */
  int                 amount;             /* number of tracks in the index */
  int                 current;

  uint16_t            sdNum;
  uint16_t            udiskNum;

  uint16_t            minIdx;
  uint16_t            maxIdx;
}hs_musicinfo_t;

hs_musicinfo_t *hs_music_create(hs_ao_t *pstAo);
void hs_music_destroy(hs_musicinfo_t *pstMusic);
void hs_music_start(hs_musicinfo_t *pstMusic);
void hs_music_stop(hs_musicinfo_t *pstMusic);
void hs_music_next(hs_musicinfo_t *pstMusic);
void hs_music_prev(hs_musicinfo_t *pstMusic);
void hs_music_volumeInc(hs_musicinfo_t *pstMusic);
void hs_music_volumeDec(hs_musicinfo_t *pstMusic);
void hs_music_funcSet(hs_musicinfo_t *pstMusic, uint32_t u32Fun);

void hs_music_volumeIncBig(hs_musicinfo_t *pstMusic);
void hs_music_volumeDecBig(hs_musicinfo_t *pstMusic);

__USED int hs_music_changeDev(uint8_t u8Msg);
__USED uint32_t hs_music_getTime(void);
__USED hs_musicstatus_t hs_music_getStatus(void);
__USED uint32_t hs_music_getIdx(void);
__USED void hs_music_setIdx(uint32_t u32Idx);
__USED hs_musicdevs_t hs_music_getCurDev(void);

__USED int hs_music_getEqIdx(void);
__USED int hs_music_getModeIdx(void);
__USED hs_musicinfo_t * hs_music_getHandle(void);



#endif

#endif
