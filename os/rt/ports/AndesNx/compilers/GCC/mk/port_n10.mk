# List of the ChibiOS/RT ARMv7M generic port files.
PORTSRC += $(CHIBIOS)/os/rt/ports/AndesNx/chcore.c \
           $(CHIBIOS)/os/rt/ports/AndesNx/chcore_n10.c
          
PORTASM += $(CHIBIOS)/os/rt/ports/AndesNx/compilers/GCC/chcoreasm_n10.S

PORTINC = $(CHIBIOS)/os/rt/ports/AndesNx \
          $(CHIBIOS)/os/rt/ports/AndesNx/compilers/GCC
