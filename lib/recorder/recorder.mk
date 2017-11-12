# List of all the audio player related files.
RECPATH = $(LIBPATH)/recorder

LIBSRC += $(RECPATH)/ai/ai.c \
          $(RECPATH)/file/rec_wr.c \
          $(RECPATH)/recorder.c
          

LIBINC += ${RECPATH} \
          ${RECPATH}/ai \
          ${RECPATH}/file

