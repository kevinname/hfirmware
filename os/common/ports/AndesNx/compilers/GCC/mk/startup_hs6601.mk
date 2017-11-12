# List of the ChibiOS generic hs6601 startup and CMSIS files.
STARTUPSRC = $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/cpu/cache.c \
             $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/cpu/irq.c  \
             $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/sw/context.c \
             $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/cpu/tick_timer.c 
          
STARTUPASM = $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/vector_32ivic.S \
             $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/start.S

STARTUPINC = $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC \
             $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/cpu \
             $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/sw \
             $(CHIBIOS)/os/common/ports/AndesNx/devices/HS6601 \
             $(CHIBIOS)/os/ext/CMSIS/include \
             $(CHIBIOS)/os/ext/CMSIS/huntersun

PORTSRC +=   $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/crt.c
PORTASM +=   $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/sw/scontext.S

STARTUPLD  = $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/ld
