TESTPATH = $(CHIBIOS)/app/test

# List of all the ChibiOS/RT test files.
TESTSRC = $(TESTPATH)/test.c

# Required include directories
TESTINC = $(TESTPATH)

ifeq ($(TEST_RT),1)
include $(TESTPATH)/rt/rt.mk
endif

ifeq ($(TEST_HAL),1)
include $(TESTPATH)/hal/hal.mk
endif

ifeq ($(TEST_LIB),1)
include $(TESTPATH)/lib/lib.mk
endif

ifeq ($(TEST_USB_RF),1)
TEST_USB_RF_PATH = $(TESTPATH)/hal/TestMAC6200
include $(TEST_USB_RF_PATH)/usb_rf.mk
TESTINC += $(TEST_USB_RF_PATH)
endif


