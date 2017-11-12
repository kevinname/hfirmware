# List of all the Huntersun/hs6601 platform files.
ifeq ($(USE_SMART_BUILD),yes)
HALCONF := $(strip $(shell cat lib/include/halconf.h | egrep -e "define"))

PLATFORMSRC := $(CHIBIOS)/os/hal/ports/common/ANDESNx/nvic.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/HS6601/cpm_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/HS6601/hal_lld.c 
               
ifneq ($(findstring HAL_USE_ADC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/HS6601/adc_lld.c
endif
ifneq ($(findstring HAL_USE_EXT TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/HS6601/ext_lld_isr.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/ext_lld.c
endif
ifneq ($(findstring HAL_USE_CAN TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/can_lld.c
endif
ifneq ($(findstring HAL_USE_MAC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/mac_lld.c
endif
ifneq ($(findstring HAL_USE_SDC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/sdc_lld.c
endif
ifneq ($(findstring HAL_USE_DAC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/dac_lld.c
endif
ifneq ($(findstring HAL_USE_RF TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/rf_lld.c
endif
ifneq ($(findstring HAL_USE_PAL TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/pal_lld.c
endif
ifneq ($(findstring HAL_USE_I2C TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/i2c_lld.c
endif
ifneq ($(findstring HAL_USE_USB TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/usb_lld.c
endif
ifneq ($(findstring HAL_USE_USB_HOST_STORAGE TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/usb_h_lld.c
endif
ifneq ($(findstring HAL_USE_SF TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/sf_lld.c
endif
ifneq ($(findstring HAL_USE_RTC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/rtc_lld.c
endif
ifneq ($(findstring HAL_USE_I2S TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/i2s_lld.c
endif
ifneq ($(findstring HAL_USE_CODEC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/codec_lld.c
endif
ifneq ($(findstring HAL_USE_SPI TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/spi_lld.c
endif
ifneq ($(findstring HAL_USE_GPT TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/gpt_lld.c
endif
ifneq ($(findstring HAL_USE_ICU TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/icu_lld.c
endif
ifneq ($(findstring HAL_USE_PWM TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/pwm_lld.c
endif
ifneq ($(findstring HAL_USE_SERIAL TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/serial_lld.c
endif
ifneq ($(findstring HAL_USE_UART TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/uart_lld.c
endif
ifneq ($(findstring HS_DMA_REQUIRED TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/dma_lld.c
endif
ifneq ($(findstring HAL_USE_FM TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/Huntersun/lld/fm_lld.c
endif
ifneq ($(findstring HAL_USE_WDT TRUE,$(HALCONF)),)
PLATFORMSRC +=$(CHIBIOS)/os/hal/ports/Huntersun/lld/wdt_lld.c
endif
else
PLATFORMSRC := $(CHIBIOS)/os/hal/ports/common/ARMCMx/nvic.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/HS6601/Huntersun_dma.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/HS6601/hal_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/HS6601/adc_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/HS6601/ext_lld_isr.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/can_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/ext_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/mac_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/sdc_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/dac_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/pal_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/i2c_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/usb_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/usb_h_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/rtc_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/i2s_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/spi_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/gpt_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/icu_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/pwm_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/st_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/serial_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/uart_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/sf_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/dma_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/codec_lld.c \               
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/fm_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/i2s_lld.c \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld/rf_lld.c \
			   $(CHIBIOS)/os/hal/ports/Huntersun/lld/wdt_lld.c
endif

# Required include directories
PLATFORMINC := $(CHIBIOS)/os/hal/ports/common/ANDESNx \
               $(CHIBIOS)/os/hal/ports/Huntersun/HS6601 \
               $(CHIBIOS)/os/hal/ports/Huntersun/lld
