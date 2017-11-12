
CFGPATH = $(LIBPATH)/config

LIBSRC += ${CFGPATH}/cfg_main.c \
          ${CFGPATH}/base/cfg_hal.c \
          ${CFGPATH}/base/cfg_cachemem.c \
          ${CFGPATH}/system/cfg_button.c \
          ${CFGPATH}/system/cfg_led.c \
          ${CFGPATH}/system/cfg_sys.c \
          ${CFGPATH}/sbc/sbc_bluez.c \
	      ${CFGPATH}/tone/cfg_audio.c \
          ${CFGPATH}/tone/cfg_sbc.c \
          ${CFGPATH}/tone/cfg_tone.c \
          ${CFGPATH}/tone/cfg_ring.c

# Required include directories
LIBINC += ${CFGPATH} \
          ${CFGPATH}/base \
          ${CFGPATH}/system \
          ${CFGPATH}/sbc \
          ${CFGPATH}/tone

LIBOPEN += ${CFGPATH}/tone/cfg_defaultTone.c