# ARM ANDES-Mx common makefile scripts and rules.

##############################################################################
# Processing options coming from the upper Makefile.
#

VERSION := $(shell $(CC) --version | grep ^$(CC) | sed 's/^.* //g')
GCC_VERSION := $(shell echo $(VERSION)| sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/' )

# GCC version before 4.8.2 doesn't support -mcmodel
ifneq ($(shell expr `echo $(GCC_VERSION)` \< 40802 ),1)	
CMODEL := -mcmodel=large
endif

# Compiler options
OPT = $(USE_OPT) 
COPT = $(USE_COPT)
CPPOPT = $(USE_CPPOPT)

# Garbage collection
ifeq ($(USE_LINK_GC),yes)
  OPT += -ffunction-sections -fdata-sections -fno-common
  LDOPT := ,--gc-sections #,--print-gc-sections
else
  LDOPT := ,--no-gc-sections
endif

# Linker extra options
ifneq ($(USE_LDOPT),)
  LDOPT := $(LDOPT),$(USE_LDOPT)
endif

# Link time optimizations
ifeq ($(USE_LTO),yes)
  OPT += -flto
endif

# FPU-related options
ifeq ($(USE_FPU),)
  USE_FPU = no
endif
ifneq ($(USE_FPU),no)
  OPT += -mfloat-abi=$(USE_FPU) -mfpu=fpv4-sp-d16 -fsingle-precision-constant
  DDEFS += -DANDES_USE_FPU=TRUE
  DADEFS += -DANDES_USE_FPU=TRUE
else
  DDEFS += -DANDES_USE_FPU=FALSE
  DADEFS += -DANDES_USE_FPU=FALSE
endif

# Add `-fno-delete-null-pointer-checks` flag if the compiler supports it.
# GCC assumes that programs cannot safely dereference null pointers, 
# and that no code or data element resides there.
# However, 0x0 is the vector table memory location, so the test must not be removed.
ifeq ($(shell $(CC) -fno-delete-null-pointer-checks -E - 2>/dev/null >/dev/null </dev/null ; echo $$?),0)
OPT += -fno-delete-null-pointer-checks
endif

ifeq ($(shell echo | $(CC) -E -dM - | grep __NDS32_EXT_IFC__ > /dev/null && echo IFC),IFC)
ifeq ($(NO_IFC),y)
OPT += -mno-ifc -DCONFIG_NO_NDS32_EXT_IFC
endif
else
ifneq ($(NO_IFC),)
$(error this toolchain do not support IFC extension)
endif
endif

ifeq ($(shell echo | $(CC) -E -dM - | grep __NDS32_EXT_EX9__ > /dev/null && echo EX9),EX9)
ifeq ($(NO_EX9),y)
OPT += -mno-ex9 -DCONFIG_NO_NDS32_EXT_EX9
endif
else
ifneq ($(NO_EX9),)
$(error this toolchain do not support EX9 extension)
endif
endif

# Process stack size
#ifeq ($(USE_PROCESS_STACKSIZE),)
#  LDOPT := $(LDOPT),--defsym=__process_stack_size__=0x400
#else
#  LDOPT := $(LDOPT),--defsym=__process_stack_size__=$(USE_PROCESS_STACKSIZE)
#endif

# Exceptions stack size
#ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
#  LDOPT := $(LDOPT),--defsym=__main_stack_size__=0x400
#else
#  LDOPT := $(LDOPT),--defsym=__main_stack_size__=$(USE_EXCEPTIONS_STACKSIZE)
#endif

# Output directory and files
ifeq ($(BUILDDIR),)
  BUILDDIR = .build
endif
ifeq ($(BUILDDIR),.)
  BUILDDIR = .build
endif

OUTFILES = $(BUILDDIR)/$(PROJECT).elf \
           $(BUILDDIR)/$(PROJECT).hex \
           $(BUILDDIR)/$(PROJECT).bin \
           $(BUILDDIR)/$(PROJECT).dmp \
           $(BUILDDIR)/$(PROJECT).s

ifdef SREC
OUTFILES += $(BUILDDIR)/$(PROJECT).srec
endif

# Source files groups and paths
ifeq ($(USE_THUMB),yes)
  TCSRC += $(CSRC)
  TCPPSRC += $(CPPSRC)
else
  ACSRC += $(CSRC)
  ACPPSRC += $(CPPSRC)
endif
ASRC	  = $(ACSRC)$(ACPPSRC)
TSRC	  = $(TCSRC)$(TCPPSRC)
SRCPATHS  = $(sort $(dir $(ASMXSRC)) $(dir $(ASMSRC)) $(dir $(ASRC)) $(dir $(TSRC)))

# Various directories
OBJDIR    = $(BUILDDIR)/obj
LSTDIR    = $(BUILDDIR)/lst

# Object files groups
ACOBJS    = $(addprefix $(OBJDIR)/, $(notdir $(ACSRC:.c=.o)))
ACPPOBJS  = $(addprefix $(OBJDIR)/, $(notdir $(ACPPSRC:.cpp=.o)))
TCOBJS    = $(addprefix $(OBJDIR)/, $(notdir $(TCSRC:.c=.o)))
TCPPOBJS  = $(addprefix $(OBJDIR)/, $(notdir $(TCPPSRC:.cpp=.o)))
ASMOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(ASMSRC:.S=.o)))
ASMXOBJS  = $(addprefix $(OBJDIR)/, $(notdir $(ASMXSRC:.S=.o)))
OBJS	  = $(ASMXOBJS) $(ASMOBJS) $(ACOBJS) $(TCOBJS) $(ACPPOBJS) $(TCPPOBJS)

LIBCOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(LIBSRC:.c=.o)))
LIBSOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(LIBASM:.S=.o)))
LIBOBJS    = $(LIBCOBJS) $(LIBSOBJS)

OSPORTCLIB = $(addprefix $(OBJDIR)/, $(notdir $(PORTSRC:.c=.o)))
OSPORTALIB = $(addprefix $(OBJDIR)/, $(notdir $(PORTASM:.S=.o)))
CMSISLIB   = $(addprefix $(OBJDIR)/, $(notdir $(CMSISRTOSSRC:.c=.o)))
OSLIBOBJS  = $(OSPORTCLIB) $(OSPORTALIB) $(CMSISLIB)

# Paths
IINCDIR   = $(patsubst %,-I%,$(INCDIR) $(DINCDIR) $(UINCDIR))
LLIBDIR   = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

# Macros
DEFS      = $(DDEFS) $(UDEFS)
ADEFS 	  = $(DADEFS) $(UADEFS)

# Libs
LIBS      = $(DLIBS) $(ULIBS)

# Various settings
MCFLAGS   = $(CMODEL) #-mcpu=$(MCU)
ODFLAGS	  = -x --syms
ASFLAGS   = $(MCFLAGS) -Wa,-amhls=$(LSTDIR)/$(notdir $(<:.s=.lst)) $(ADEFS)
ASXFLAGS  = $(MCFLAGS) -Wa,-amhls=$(LSTDIR)/$(notdir $(<:.S=.lst)) $(ADEFS)
CFLAGS    = $(MCFLAGS) $(OPT) $(COPT) $(CWARN) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.c=.lst)) $(DEFS)
CPPFLAGS  = $(MCFLAGS) $(OPT) $(CPPOPT) $(CPPWARN) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.cpp=.lst)) $(DEFS)
LDFLAGS   = $(MCFLAGS) $(OPT) -nostartfiles $(LLIBDIR) -Wl,-Map=$(BUILDDIR)/$(PROJECT).map,--cref,--no-warn-mismatch,--library-path=$(RULESPATH),-T$(LDSCRIPT)$(LDOPT)

# Generate dependency information
ASFLAGS  += -MD -MP -MF .dep/$(@F).d
CFLAGS   += -MD -MP -MF .dep/$(@F).d
CPPFLAGS += -MD -MP -MF .dep/$(@F).d

# Paths where to search for sources
VPATH     = $(SRCPATHS)

#
# Makefile rules
#

all: PRE_MAKE_ALL_RULE_HOOK $(OBJS) $(OUTFILES) POST_MAKE_ALL_RULE_HOOK

PRE_MAKE_ALL_RULE_HOOK:

POST_MAKE_ALL_RULE_HOOK:
	@echo 
	@echo --------  Done  --------

$(OBJS): | $(BUILDDIR) $(OBJDIR) $(LSTDIR)


$(BUILDDIR):
ifneq ($(USE_VERBOSE_COMPILE),yes)
	@echo Compiler Options
endif
	@mkdir -p $(BUILDDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(LSTDIR):
	@mkdir -p $(LSTDIR)

$(LDSCRIPT): $(SAGSCRIPT)
	@$(SAG) $(SAGSCRIPT) -o $(LDSCRIPT)

$(ACPPOBJS) : $(OBJDIR)/%.o : %.cpp Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CPPC) -c $(CPPFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CPPC) -c $(CPPFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
endif

$(TCPPOBJS) : $(OBJDIR)/%.o : %.cpp Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CPPC) -c $(CPPFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CPPC) -c $(CPPFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
endif

$(ACOBJS) : $(OBJDIR)/%.o : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CC) -c $(CFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
endif

$(TCOBJS) : $(OBJDIR)/%.o : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) $(TOPT) $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CC) -c $(CFLAGS) $(TOPT) $(IINCDIR) $< -o $@
endif

$(ASMOBJS) : $(OBJDIR)/%.o : %.S Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(ASFLAGS) $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CC) -c  $(IINCDIR) $(CFLAGS) $(ASFLAGS) $< -o $@
endif

$(ASMXOBJS) : $(OBJDIR)/%.o : %.S Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(ASXFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CC) -c $(ASXFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
endif

$(BUILDDIR)/$(PROJECT).elf: $(OBJS) $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
	$(AR) $(RELEASE_LIB)/libhs.a $(LIBOBJS)
	$(AR) $(RELEASE_OS)/libos.a $(OSLIBOBJS)
else
	@echo Linking $@ 
	@$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
	@$(AR) $(RELEASE_LIB)/libhs.a $(LIBOBJS)
	@$(AR) $(RELEASE_OS)/libos.a $(OSLIBOBJS)
endif

%.hex: %.elf $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(HEX) $< $@
else
	@echo Creating $@
	@$(HEX) $< $@
endif

%.bin: %.elf $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(BIN) $< $@
else
	@echo Creating $@
	@$(BIN) $< $@
endif

%.srec: %.elf $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(SREC) $< $@
else
	@echo Creating $@
	@$(SREC) $< $@
endif

%.dmp: %.elf $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(OD) $(ODFLAGS) $< > $@
	$(SZ) $<
else
	@echo Creating $@
	@$(OD) $(ODFLAGS) $< > $@
	@echo
	@$(SZ) $<
endif

%.list: %.elf $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(OD) -S $< > $@
else
	@echo Creating $@
	@$(OD) -S $< > $@
endif

%.s: %.elf $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(OD) -D $< > $@
else
	@echo Creating $@
	@$(OD) -D $< > $@
endif

#lib: $(OBJS) $(BUILDDIR)/lib$(PROJECT).a

#$(BUILDDIR)/lib$(PROJECT).a: $(OBJS)
#	@$(AR) -r $@ $^
#	@echo
#	@echo Done

clean:
	@echo Cleaning
	-rm -fR .dep $(BUILDDIR) $(LDSCRIPT)
	@echo
	@echo Done

#
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
