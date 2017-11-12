# List of all the usb related files.
DRVPATH = $(LIBPATH)/drvhal

LIBSRC += $(DRVPATH)/drvhal.c

LIBINC += ${DRVPATH}

include $(DRVPATH)/rtc/rtc.mk
include $(DRVPATH)/i2c/i2c.mk
#include $(DRVPATH)/infrared/infrared.mk
include $(DRVPATH)/usb/usb.mk
include $(DRVPATH)/pad/pad.mk
include $(DRVPATH)/pmu/pmu.mk
include $(DRVPATH)/pwm/pwm.mk
include $(DRVPATH)/adc/adc.mk
include $(DRVPATH)/sd/sd.mk
include $(DRVPATH)/audio/audio.mk

