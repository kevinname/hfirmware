# List of all the led related files.
USERLEDPATH = $(USERPATH)/$(BOARD)/led

LIBINC  += ${USERLEDPATH}  \
           ${USERLEDPATH}/rhythm \
           ${USERLEDPATH}/screen

# screen led
LIBOPEN += $(USERLEDPATH)/screen/led_disp.c \
           $(USERLEDPATH)/screen/led_engine.c
