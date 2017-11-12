##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

HOPT ?=s
RIFLASH ?=0
PROTYPE ?=0
DEBUG ?=1
BOARD ?=huntersun
TEST_HAL ?=0
BT ?=1
RIFLASH ?=1

NO_EX9 ?=y 
NO_IFC ?=y
# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT =   #-fomit-frame-pointer -falign-functions=16
endif

ifeq ($(DEBUG),1)
  USE_OPT	+= -O$(HOPT) -g3 -fno-schedule-insns #--coverage
else
  USE_OPT	+= -O$(HOPT) -g3 -fomit-frame-pointer -falign-functions=16
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = 
endif

# Enable this if you want link time optimizations (LTO)
ifeq ($(USE_LTO),)
  USE_LTO = yes
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x400
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

# Enables the use of FPU on Cortex-M4 (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = no
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
ifeq ($(DEBUG),1)
PROJECT = hs6601D
else
PROJECT = hs6601
endif

# Imported source files and paths
CHIBIOS = .
RELEASE_LIB = $(CHIBIOS)/../release/demo/lib
RELEASE_OS  = $(CHIBIOS)/../release/demo/os/rt/ports/AndesNx

include $(CHIBIOS)/app/app.mk
include $(CHIBIOS)/lib/lib.mk
# Startup files.
include $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC/mk/startup_hs6601.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/Huntersun/HS6601/platform.mk
include $(CHIBIOS)/os/hal/boards/HUNTERSUN_HS6601/board.mk
include $(CHIBIOS)/os/hal/osal/rt/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/rt/ports/AndesNx/compilers/GCC/mk/port_n10.mk
include $(CHIBIOS)/os/rt/ports/AndesNx/cmsis_os/cmsis_os.mk

# Test File

ifeq ($(TEST),1)
DDEFS += -DTEST_ENABLE

ifeq ($(TEST_RT),1)
DDEFS += -DTEST_RT_ENABLE
endif

ifeq ($(TEST_HAL),1)
DDEFS += -DTEST_HAL_ENABLE
endif

ifeq ($(TEST_LIB),1)
DDEFS += -DTEST_LIB_ENABLE
endif

include $(CHIBIOS)/app/test/test.mk

ifeq ($(UTEST),1)
DDEFS += -DHS_UNIT_TEST
endif
endif


ifeq ($(BT),0)
else
DDEFS += -DHS_USE_BT=$(BT)
endif #ifeq ($(BT),1)

ifeq ($(RIFLASH),1)
DDEFS   += -DRUN_IN_FLASH
DADEFS  += -DRUN_IN_FLASH
endif

DDEFS   += -DHS_PRODUCT_TYPE=$(PROTYPE)

# Define linker script file here
ifeq ($(RIFLASH),1)

ifeq ($(PROTYPE),1)
LDSCRIPT= $(STARTUPLD)/hs6601_hp.ld
SAGSCRIPT= $(STARTUPLD)/hs6601_hp.sag
else
LDSCRIPT= $(STARTUPLD)/hs6601.ld
ifeq ($(BT),1)
SAGSCRIPT= $(STARTUPLD)/hs6601.sag
else
SAGSCRIPT= $(STARTUPLD)/hs6601_dm.sag
endif
endif

else ifeq ($(RIFLASH),2)
LDSCRIPT= $(STARTUPLD)/hs6601_ram.ld
SAGSCRIPT= $(STARTUPLD)/hs6601_ram.sag
else
LDSCRIPT= $(STARTUPLD)/hs6601_rom.ld
SAGSCRIPT= $(STARTUPLD)/hs6601_rom.sag
endif

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(STARTUPSRC) \
       $(HALSRC) \
       $(PORTSRC) \
       $(PLATFORMSRC) \
       $(KERNSRC) \
       $(OSALSRC) \
       $(BOARDSRC) \
       $(TESTSRC) \
       $(LWSRC) \
       $(CMSISRTOSSRC) \
       $(FATFSSRC) \
       $(LIBSRC) \
       $(LIBOPEN) \
       $(APPSRC)

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC =

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC = $(STARTUPASM) $(PORTASM) $(OSALASM) $(LIBASM)

INCDIR = $(STARTUPINC) $(KERNINC) $(PORTINC) $(OSALINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) $(TESTINC) \
         $(LWINC) $(FATFSINC) $(CMSISRTOSINC) $(APPINC) \
         ${LIBINC}

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#
TRGT = nds32le-elf-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)as
AR   = $(TRGT)ar r
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

ifeq ($(HOST),linux)
SAG  = /opt/Andestech/BSPv404/utils/nds_ldsag
else
SAG  = nds_ldsag.exe
endif

# ARM-specific options here
AOPT =

# THUMB-specific options here
#TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra #-Wundef -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra -Wundef

#
# Compiler settings
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
#UDEFS = -DSTDOUT_SD=SD1 -DSTDIN_SD=SD1 -D'SVN_REV="$(shell svnversion -n .)"'

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR = $(CHIBIOS)/lib/bt

# List all user libraries here
ifeq ($(BT),1)
ULIBS = -lbthost_cl -lhc_nds #-laec_nds -lns_nds
endif

ifeq ($(BT),2)
ULIBS = -lbthost_pos -lhc_nds_data #-laec_nds -lns_nds
endif

ifeq ($(BT),3)
ULIBS = -lhc_nds_data
endif

ULIBS += -lm

#
# End of user defines
##############################################################################

RULESPATH = $(CHIBIOS)/os/common/ports/AndesNx/compilers/GCC
include $(RULESPATH)/rules.mk
