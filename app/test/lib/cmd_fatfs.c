#include <string.h>
#include <stdlib.h> 
#include "lib.h"

void cmd_isMount(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if(argc != 0) {
    chprintf(chp, "Usage: mount\r\n");
  }

  if(hs_fatfs_isMount(FATFS_MEMDEV_SD))
    chprintf(chp, "SD-Card mount fatfs ok!\r\n");
  else
    chprintf(chp, "SD-Card fatfs is not mount!\r\n");

  if(hs_fatfs_isMount(FATFS_MEMDEV_UDISK))
    chprintf(chp, "USB-Disk mount fatfs ok!\r\n");
  else
    chprintf(chp, "USB-Disk fatfs is not mount!\r\n");
}

void cmd_fsLs(BaseSequentialStream *chp, int argc, char *argv[]) {

  TCHAR *pu8Path;
  DIR dir;
  FILINFO fileInfo;
  #if _LFN_UNICODE
  int i;
  #endif

  (void)argv;
  if(argc != 0) {
    chprintf(chp, "Usage: ls\r\n");
  }

  if((!hs_fatfs_isMount(FATFS_MEMDEV_SD))
    && (!hs_fatfs_isMount(FATFS_MEMDEV_UDISK))) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  pu8Path = (TCHAR *)hs_malloc(sizeof(TCHAR) * 256, __MT_Z_GENERAL);  
  if(pu8Path == NULL) {
    chprintf(chp, "memory is not enough!\r\n");
    return;
  }

  fileInfo.lfsize = sizeof(TCHAR) * 256;
  fileInfo.lfname = (TCHAR *)hs_malloc(sizeof(TCHAR) * 256, __MT_Z_GENERAL);  
  if(fileInfo.lfname == NULL) {
    chprintf(chp, "memory is not enough!\r\n");
    hs_free(pu8Path);
    return;
  }  

  if(FR_OK != f_getcwd(pu8Path, 256)) {
    chprintf(chp, "get current path error!\r\n");
    hs_free(pu8Path);
    hs_free(fileInfo.lfname);
    return;
  }

  if(FR_OK != f_opendir(&dir, pu8Path)) {
    hs_free(pu8Path);
    hs_free(fileInfo.lfname);
    chprintf(chp, "open dir error!\r\n");
    return;
  }

  chprintf(chp, "\r\n name                                 size\r\n");

  while(FR_OK == f_readdir(&dir, &fileInfo)) {

    if(fileInfo.fname[0] == 0)
      break;

    if(fileInfo.lfname[0] != 0) {

      #if _LFN_UNICODE
      for(i=0; ; i++) {
        if(fileInfo.lfname[i] == 0)
          break;
        
        chprintf(chp, "%s", &fileInfo.lfname[i]);
      }
      #else
      chprintf(chp, "%s", fileInfo.lfname);
      #endif
    }
    else {

      #if _LFN_UNICODE
      for(i=0; ; i++) {
        if(fileInfo.fname[i] == 0)
          break;
        
        chprintf(chp, "%s", &fileInfo.fname[i]);
      }
      #else
      chprintf(chp, "%s", fileInfo.fname);
      #endif
    }

    #if _LFN_UNICODE
    for(i=32-i; i > 0; i--)
      chprintf(chp, " ");
    #endif
    
    chprintf(chp, "%10d\r\n", fileInfo.fsize);
  }

  f_closedir(&dir);
  hs_free(pu8Path);
  hs_free(fileInfo.lfname);

  chprintf(chp, "\r\n");
}

void cmd_fsPwd(BaseSequentialStream *chp, int argc, char *argv[]) {

  TCHAR *pu8Path;
  #if _LFN_UNICODE
  uint32_t i;
  #endif

  (void)argv;
  if(argc != 0) {
    chprintf(chp, "Usage: Pwd\r\n");
  }

  if((!hs_fatfs_isMount(FATFS_MEMDEV_SD))
    && (!hs_fatfs_isMount(FATFS_MEMDEV_UDISK))) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  pu8Path = (TCHAR *)hs_malloc(sizeof(TCHAR) * 256, __MT_Z_GENERAL);  
  if(pu8Path == NULL) {
    chprintf(chp, "memory is not enough!\r\n");
    return;
  }

  if(FR_OK != f_getcwd(pu8Path, 256)) {
    chprintf(chp, "get current path error!\r\n");
    hs_free(pu8Path);
    return;
  }

  chprintf(chp, "\r\nCurrent Path:\r\n\t");

  #if _LFN_UNICODE
  i = 0;
  while(0 != pu8Path[i]) {

    chprintf(chp, "%s", &pu8Path[i]);
    i++;
  }
  #else
  chprintf(chp, "%s", pu8Path);
  #endif

  hs_free(pu8Path);
  chprintf(chp, "\r\n");
}


void cmd_fsCd(BaseSequentialStream *chp, int argc, char *argv[]) {

  TCHAR *pu8Path;
  uint32_t i, j;

  if(argc != 1) {
    chprintf(chp, "Usage: cd dir\r\n");
  }

  if((!hs_fatfs_isMount(FATFS_MEMDEV_SD))
    && (!hs_fatfs_isMount(FATFS_MEMDEV_UDISK))) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  pu8Path = (TCHAR *)hs_malloc(sizeof(TCHAR) * 256, __MT_Z_GENERAL);  
  if(pu8Path == NULL) {
    chprintf(chp, "memory is not enough!\r\n");
    return;
  }

  if(FR_OK != f_getcwd(pu8Path, 256)) {
    chprintf(chp, "get current path error!\r\n");
    hs_free(pu8Path);
    return;
  }

  i = 0;
  while(0 != pu8Path[i]) {
    i++;
  }
  pu8Path[i++] = _T('/');

  for(j=0; j<strlen(argv[0]); j++)
    pu8Path[i++] = argv[0][j];

  pu8Path[i] = 0;

  if(FR_OK != f_chdir(pu8Path)) {
    chprintf(chp, "change current path error!\r\n");
    hs_free(pu8Path);
    return;
  }

  hs_free(pu8Path);
  chprintf(chp, "\r\n");
}

void cmd_fsCdDrive(BaseSequentialStream *chp, int argc, char *argv[]) {

  TCHAR pPath[5];

  if(argc != 1) {
    chprintf(chp, "Usage: cdd drive\r\n");
  }

  if((!hs_fatfs_isMount(FATFS_MEMDEV_SD))
    && (!hs_fatfs_isMount(FATFS_MEMDEV_UDISK))) {
    chprintf(chp, "fatfs is not mount!\r\n");
    return ;
  }

  memset(pPath, 0, sizeof(TCHAR) * 5);
  if(strlen(argv[0]) > 2) {
    chprintf(chp, "drive %s is error!\r\n", argv[0]);
    return ;
  }

  pPath[0] = argv[0][0];
  pPath[1] = argv[0][1];
    
  if(FR_OK != f_chdrive(pPath)) {
    chprintf(chp, "enter the drive %s error!\r\n", argv[0]);
  }

  chprintf(chp, "\r\n");
  return ;
}

void cmd_fsMkdir(BaseSequentialStream *chp, int argc, char *argv[]) {

  TCHAR *pu8Path;
  uint32_t i, j;

  if(argc != 1) {
    chprintf(chp, "Usage: mkdir dir\r\n");
  }

  if((!hs_fatfs_isMount(FATFS_MEMDEV_SD))
    && (!hs_fatfs_isMount(FATFS_MEMDEV_UDISK))) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  pu8Path = (TCHAR *)hs_malloc(sizeof(TCHAR) * 256, __MT_Z_GENERAL);  
  if(pu8Path == NULL) {
    chprintf(chp, "memory is not enough!\r\n");
    return;
  }

  if(FR_OK != f_getcwd(pu8Path, 256)) {
    chprintf(chp, "get current path error!\r\n");
    hs_free(pu8Path);
    return;
  }

  i = 0;
  while(0 != pu8Path[i]) {
    i++;
  }
  pu8Path[i++] = _T('/');

  for(j=0; j<strlen(argv[0]); j++)
    pu8Path[i++] = argv[0][j];

  pu8Path[i] = 0;

  if(FR_OK != f_mkdir(pu8Path)) {
    chprintf(chp, "mkdir error!\r\n");
    hs_free(pu8Path);
    return;
  }

  hs_free(pu8Path);
  chprintf(chp, "create dir \"%s\" ok!\r\n", argv[0]);
}

void cmd_fsCat(BaseSequentialStream *chp, int argc, char *argv[]) {

  TCHAR *pu8Path;
  uint32_t i, j, base;
  UINT len;
  uint8_t *tmpPtr;
  FIL* fp;

  if((argc != 1) && (argc != 2)) {
    chprintf(chp, "Usage: cat [-s/-b/>] file\r\n");
  }

  if((!hs_fatfs_isMount(FATFS_MEMDEV_SD))
    && (!hs_fatfs_isMount(FATFS_MEMDEV_UDISK))) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  pu8Path = (TCHAR *)hs_malloc(4 * 1024, __MT_Z_DMA);  
  if(pu8Path == NULL) {
    chprintf(chp, "memory is not enough!\r\n");
    return;
  }

  fp = (FIL *)hs_malloc(sizeof(FIL), __MT_Z_DMA);  
  if(fp == NULL) {
    chprintf(chp, "memory is not enough!\r\n");
    hs_free(pu8Path);
    return;
  }

  if(FR_OK != f_getcwd(pu8Path, 256)) {
    chprintf(chp, "get current path error!\r\n");
    hs_free(pu8Path);
    hs_free(fp);
    return;
  }

  i = 0;
  while(0 != pu8Path[i]) {
    i++;
  }
  pu8Path[i++] = _T('/');

  tmpPtr = argc == 1 ? (uint8_t *)argv[0] : (uint8_t *)argv[1];
  for(j=0; j<strlen((char *)tmpPtr); j++)
    pu8Path[i++] = tmpPtr[j];

  pu8Path[i] = 0;

  if(FR_OK != f_open(fp, pu8Path, FA_READ)) {
    chprintf(chp, "open file error!\r\n");
    hs_free(pu8Path);
    hs_free(fp);
    return;
  }

  tmpPtr = (uint8_t *)pu8Path;
  if(argc == 2) {
    if(0 == strcmp(argv[0], "-s")) {
      while(FR_OK == f_read(fp, tmpPtr, 4096, &len)) {
        if(len == 0)
          break;
        
        for(i=0; i<len; i++)
          chprintf(chp, "%c", tmpPtr[i]);
      };
    }
    else if(0 == strcmp(argv[0], "-b")) {
      base = 0;
      while(FR_OK == f_read(fp, tmpPtr, 4096, &len)) {
        if(len == 0)
          break;
        
        for(i=0; i<len; i++) {
          if(i%16 == 0)
            chprintf(chp, "\r\n%08X:\t", base + i);
          
          chprintf(chp, "%02X ", tmpPtr[i]);
        }

        base += len;
      };
    }
  }
  else {
    while(FR_OK == f_read(fp, tmpPtr, 4096, &len)) {
      if(len == 0)
        break;
      
      for(i=0; i<len; i++)
        chprintf(chp, "%c", tmpPtr[i]);
    };
  }

  f_close(fp);
  hs_free(pu8Path);
  hs_free(fp);
  chprintf(chp, "\r\n");
}


void cmd_fsDel(BaseSequentialStream *chp, int argc, char *argv[]) {

  TCHAR *pu8Path;
  uint32_t i, j;
  uint8_t *tmpPtr;

  if(argc != 1) {
    chprintf(chp, "Usage: del file\r\n");
  }

  if((!hs_fatfs_isMount(FATFS_MEMDEV_SD))
    && (!hs_fatfs_isMount(FATFS_MEMDEV_UDISK))) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  pu8Path = (TCHAR *)hs_malloc(sizeof(TCHAR) * 256, __MT_Z_GENERAL);  
  if(pu8Path == NULL) {
    chprintf(chp, "memory is not enough!\r\n");
    return;
  }

  if(FR_OK != f_getcwd(pu8Path, 256)) {
    chprintf(chp, "get current path error!\r\n");
    hs_free(pu8Path);
    return;
  }

  i = 0;
  while(0 != pu8Path[i]) {
    i++;
  }
  pu8Path[i++] = _T('/');

  tmpPtr = (uint8_t *)argv[0];
  for(j=0; j<strlen(argv[0]); j++)
    pu8Path[i++] = tmpPtr[j];

  pu8Path[i] = 0;

  if(FR_OK != f_unlink(pu8Path)) {
    chprintf(chp, "delete file %s error!\r\n", argv[0]);
  }
  
  hs_free(pu8Path);
  chprintf(chp, "\r\n");
}

void cmd_fsFormat(BaseSequentialStream *chp, int argc, char *argv[]) {

 (void)argv;

  if(argc != 0) {
    chprintf(chp, "Usage: format\r\n");
  }

  if((!hs_fatfs_isMount(FATFS_MEMDEV_SD))
    && (!hs_fatfs_isMount(FATFS_MEMDEV_UDISK))) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  chprintf(chp, "Formating...");

  if(FR_OK != f_mkfs(_T("0:"),0,0))
    chprintf(chp, " error!");
  else
    chprintf(chp, " ok!");
  
  chprintf(chp, "\r\n");
}

