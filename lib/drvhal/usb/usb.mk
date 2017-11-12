# List of all the usb related files.
USBPATH = $(DRVPATH)/usb

LIBOPEN += $(USBPATH)/usbdev.c \
          $(USBPATH)/usbsubdev_serial.c \
          $(USBPATH)/usbsubdev_audio.c \
          $(USBPATH)/usbsubdev_storage.c
		  
LIBINC += ${USBPATH}