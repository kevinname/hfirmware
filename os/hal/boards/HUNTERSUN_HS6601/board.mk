# List of all the board related files.

ifeq ($(BOARD), huntersun)
BOARDSRC = ${CHIBIOS}/os/hal/boards/HUNTERSUN_HS6601/board.c
BOARDINC = ${CHIBIOS}/os/hal/boards/HUNTERSUN_HS6601
else
BOARDSRC = ${CHIBIOS}/os/hal/boards/HUNTERSUN_HS6601/$(BOARD)/board.c
BOARDINC = ${CHIBIOS}/os/hal/boards/HUNTERSUN_HS6601/$(BOARD)
endif
