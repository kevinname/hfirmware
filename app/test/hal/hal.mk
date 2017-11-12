# List of all the ChibiOS/RT test files.

TEST_HAL_PATH = ${TESTPATH}/hal

TESTSRC += ${TEST_HAL_PATH}/cmd_adc.c \
           ${TEST_HAL_PATH}/cmd_i2c.c \
           ${TEST_HAL_PATH}/cmd_sd.c \
           ${TEST_HAL_PATH}/cmd_sf.c \
           ${TEST_HAL_PATH}/cmd_cpu.c \
           ${TEST_HAL_PATH}/cmd_avcodec.c \
           ${TEST_HAL_PATH}/cmd_rtc.c \
           ${TEST_HAL_PATH}/cmd_snd.c \
           ${TEST_HAL_PATH}/cmd_dma.c \
           ${TEST_HAL_PATH}/cmd_rf.c  \
           ${TEST_HAL_PATH}/cmd_usbhost.c  \
           ${TEST_HAL_PATH}/cmd_tim.c	\
           ${TEST_HAL_PATH}/cmd_fm.c	\
           ${TEST_HAL_PATH}/cmd_spi.c   \
           ${TEST_HAL_PATH}/cmd_gpio.c  \
           ${TEST_HAL_PATH}/cmd_wdt.c  \
           ${TEST_HAL_PATH}/cmd_btrf.c  
           
# Required include directories
TESTINC += ${TEST_HAL_PATH}
