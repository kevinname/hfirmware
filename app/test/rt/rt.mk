# List of all the ChibiOS/RT test files.

TEST_RT_PATH = ${TESTPATH}/rt

TESTSRC += ${TEST_RT_PATH}/test_rt.c \
          ${TEST_RT_PATH}/testthd.c \
          ${TEST_RT_PATH}/testsem.c \
          ${TEST_RT_PATH}/testmtx.c \
          ${TEST_RT_PATH}/testmsg.c \
          ${TEST_RT_PATH}/testmbox.c \
          ${TEST_RT_PATH}/testevt.c \
          ${TEST_RT_PATH}/testheap.c \
          ${TEST_RT_PATH}/testpools.c \
          ${TEST_RT_PATH}/testdyn.c \
          ${TEST_RT_PATH}/testqueues.c \
          ${TEST_RT_PATH}/testsys.c \
          ${TEST_RT_PATH}/testbmk.c

# Required include directories
TESTINC += ${TEST_RT_PATH}
