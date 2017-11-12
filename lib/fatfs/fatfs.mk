# FATFS files.

FATFSPATH = $(LIBPATH)/fatfs

FATFSSRC = ${CHIBIOS}/os/various/fatfs_bindings/fatfs_diskio.c \
           ${CHIBIOS}/os/various/fatfs_bindings/fatfs_syscall.c 
           
LIBOPEN  += ${FATFSPATH}/src/ff.c \
           ${FATFSPATH}/src/option/unicode.c	\
           ${FATFSPATH}/fatfs.c

FATFSINC = ${FATFSPATH}		\
		   ${FATFSPATH}/src

