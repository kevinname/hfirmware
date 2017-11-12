# List of all the btapp related files.
BTPATH = $(LIBPATH)/bt

LIBSRC += $(BTPATH)/libhal/bt_hc.c \
          $(BTPATH)/libhal/bt_vhci_usb.c \
          $(BTPATH)/libhal/bt_event.c \
          $(BTPATH)/libbt.c

LIBINC += ${BTPATH} \
          ${BTPATH}/libhal \
          ${BTPATH}/inc

