LIBPATH = $(CHIBIOS)/lib

LIBSRC = 
LIBASM = 
LIBINC = $(LIBPATH) 

LIBOPEN = 

include $(LIBPATH)/include/inc.mk
include $(LIBPATH)/fatfs/fatfs.mk
include $(LIBPATH)/shell/shell.mk
include $(LIBPATH)/mem/mem.mk
include $(LIBPATH)/oshal/oshal.mk
include $(LIBPATH)/config/config.mk
include $(LIBPATH)/player/player.mk
include $(LIBPATH)/recorder/recorder.mk
include $(LIBPATH)/speaker/speaker.mk
include $(LIBPATH)/drvhal/drvhal.mk
include $(LIBPATH)/user/user.mk

ifeq ($(BT),1)
include $(LIBPATH)/bt/bt.mk
endif

ifeq ($(BT),2)
include $(LIBPATH)/bt/bt_data.mk
endif

ifeq ($(BT),3)
include $(LIBPATH)/bt/bt_dongle.mk
endif