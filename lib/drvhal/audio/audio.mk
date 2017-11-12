# List of all the codec related files.
AUDIOPATH = $(DRVPATH)/audio

LIBSRC += $(AUDIOPATH)/lib_codec.c	\
          $(AUDIOPATH)/lib_audio.c  \
          $(AUDIOPATH)/lib_aec.c  \
          $(AUDIOPATH)/lib_ans.c

LIBOPEN += $(AUDIOPATH)/audio_rtythmCfg.c

LIBINC += ${CODECPATH}
