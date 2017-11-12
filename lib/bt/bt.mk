# List of all the btapp related files.
BTPATH = $(LIBPATH)/bt

LIBSRC += $(BTPATH)/libhal/bt_hc.c \
          $(BTPATH)/libhal/bt_host_cl.c \
          $(BTPATH)/libhal/bt_vhci_cl.c \
          $(BTPATH)/libhal/bt_config.c \
          $(BTPATH)/libhal/bt_event.c \
          $(BTPATH)/libhal/bt_hcidump.c \
          $(BTPATH)/libhal/bt_host_wrapper.c \
          $(BTPATH)/libbt.c \
          $(BTPATH)/src/app_gap.c \
          $(BTPATH)/src/app_fsm.c \
          $(BTPATH)/src/app_a2dp.c \
          $(BTPATH)/src/app_avrcp.c \
          $(BTPATH)/src/app_hfp.c \
          $(BTPATH)/src/app_spp.c \
          $(BTPATH)/src/app_hid.c \
          $(BTPATH)/src/app_ble.c \
          $(BTPATH)/src/app_btled.c

LIBINC += ${BTPATH} \
          ${BTPATH}/libhal \
          ${BTPATH}/src \
          ${BTPATH}/inc

#include $(BTPATH)/host/bt_rfcomm.mk

