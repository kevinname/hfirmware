# List of all the btapp related files.
BTPATH = $(LIBPATH)/bt

LIBSRC += $(BTPATH)/libbt.c \
					$(BTPATH)/libhal/bt_event.c \
					$(BTPATH)/libhal/bt_config.c \
					$(BTPATH)/libhal/bt_control.c

LIBINC += ${BTPATH} \
          ${BTPATH}/libhal \
          ${BTPATH}/inc

