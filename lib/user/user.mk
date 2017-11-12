# List of all the usb related files.
USERPATH = $(LIBPATH)/user

LIBINC += ${USERPATH}
LIBINC += ${USERPATH}/$(BOARD)

include $(USERPATH)/$(BOARD)/$(BOARD).mk

