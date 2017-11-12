# List of all the usb related files.
USERLEDPATH = $(USERPATH)/$(BOARD)/led

LIBINC  += ${USERLEDPATH}  \
           ${USERLEDPATH}/rgb  \
           ${USERLEDPATH}/rhythm  \
           ${USERLEDPATH}/screen

# rhythm & rgb led
LIBOPEN += ${USERLEDPATH}/rhythm/led_rhythm.c

# screen led
LIBOPEN += $(USERLEDPATH)/screen/led_disp.c \
           $(USERLEDPATH)/screen/led_engine.c
