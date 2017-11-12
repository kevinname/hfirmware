# List of all the ChibiOS/RT test files.

TEST_USB_RF_PATH = ${TESTPATH}/hal/TestMAC6200

TESTSRC += ${TEST_USB_RF_PATH}/C8051F_USB.c \
           ${TEST_USB_RF_PATH}/HS6200.c \
           ${TEST_USB_RF_PATH}/HS6200_Analog_Test.c \
           ${TEST_USB_RF_PATH}/HS6200_Debug.c \
		   ${TEST_USB_RF_PATH}/HS6200_test.c \
           ${TEST_USB_RF_PATH}/HS6200_test_sys.c \
           ${TEST_USB_RF_PATH}/nRF24L01_X.c \


# Required include directories
TESTINC += ${TESTPATH}
