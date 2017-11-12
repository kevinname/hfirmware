# List of all the ChibiOS/RT test files.

TEST_LIB_PATH = ${TESTPATH}/lib

TESTSRC += ${TEST_LIB_PATH}/cmd_mem.c \
           ${TEST_LIB_PATH}/cmd_vtime.c \
           ${TEST_LIB_PATH}/cmd_message.c \
           ${TEST_LIB_PATH}/cmd_sem.c \
           ${TEST_LIB_PATH}/cmd_fatfs.c \
           ${TEST_LIB_PATH}/cmd_drvhal.c \
           ${TEST_LIB_PATH}/cmd_config.c
#           ${TEST_LIB_PATH}/bt_rfcomm_client.c \
#           ${TEST_LIB_PATH}/bt_rfcomm_server.c \
#           ${TEST_LIB_PATH}/cmd_rfcomm.c

# Required include directories
TESTINC += ${TEST_LIB_PATH}
