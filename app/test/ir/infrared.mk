# List of all the I2C related files.
TEST_HAL_PATH = ${TESTPATH}/ir

TESTSRC += ${TEST_HAL_PATH}/infrared_interface.c 
           
# Required include directories
TESTINC += ${TEST_HAL_PATH}