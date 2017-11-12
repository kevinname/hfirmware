# List of all the audio player related files.
PLAYERPATH = $(LIBPATH)/player

LIBSRC += $(PLAYERPATH)/adec/libmp3/real/bitstream.c \
          $(PLAYERPATH)/adec/libmp3/real/buffers.c \
          $(PLAYERPATH)/adec/libmp3/real/dct32.c \
          $(PLAYERPATH)/adec/libmp3/real/dequant.c \
          $(PLAYERPATH)/adec/libmp3/real/dqchan.c \
          $(PLAYERPATH)/adec/libmp3/real/huffman.c \
          $(PLAYERPATH)/adec/libmp3/real/hufftabs.c \
          $(PLAYERPATH)/adec/libmp3/real/imdct.c \
          $(PLAYERPATH)/adec/libmp3/real/scalfact.c \
          $(PLAYERPATH)/adec/libmp3/real/stproc.c \
          $(PLAYERPATH)/adec/libmp3/real/subband.c \
          $(PLAYERPATH)/adec/libmp3/real/trigtabs.c \
          $(PLAYERPATH)/adec/libmp3/real/polyphase.c \
          $(PLAYERPATH)/adec/libmp3/mp3dec.c \
          $(PLAYERPATH)/adec/libmp3/mp3tabs.c \
          $(PLAYERPATH)/adec/adec.c \
          $(PLAYERPATH)/adec/mp3.c \
          $(PLAYERPATH)/adec/wav.c \
          $(PLAYERPATH)/adec/libwma/mdct.c \
          $(PLAYERPATH)/adec/libwma/mdct2.c \
          $(PLAYERPATH)/adec/libwma/wmabitstream.c \
          $(PLAYERPATH)/adec/libwma/wmadeci.c \
          $(PLAYERPATH)/adec/libwma/wmafixed.c \
          $(PLAYERPATH)/adec/wma.c \
          $(PLAYERPATH)/ao/ao.c \
          $(PLAYERPATH)/asrc/afm.c \
          $(PLAYERPATH)/asrc/linein.c \
          $(PLAYERPATH)/asrc/music.c \
          $(PLAYERPATH)/asrc/usbaudio.c \
          $(PLAYERPATH)/player.c

LIBINC += ${PLAYERPATH}/adec/ \
          ${PLAYERPATH}/adec/libmp3 \
          ${PLAYERPATH}/adec/libmp3/pub \
          ${PLAYERPATH}/adec/libmp3/real \
          $(PLAYERPATH)/adec/libwma \
          $(PLAYERPATH)/ao \
          $(PLAYERPATH)/asrc \
          $(PLAYERPATH)/

#LIBASM += $(PLAYERPATH)/adec/libmp3/real/asmpoly_nds32.S

LIBOPEN += $(PLAYERPATH)/asrc/music_eqDefault.c