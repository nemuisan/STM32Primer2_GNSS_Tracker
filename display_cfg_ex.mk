# This is the Part of Display Driver 

DISPLAY_INC = $(DISPLAY_LIB)/abstract/inc
DISPLAY_SRC = $(DISPLAY_LIB)/abstract/src
DISPLAY_BMP = $(DISPLAY_INC)/bmps

DISPLAY_DRV_SRC = $(DISPLAY_LIB)/drivers/src
DISPLAY_DRV_INC = $(DISPLAY_LIB)/drivers/inc

DISPLAY_MCU_SRC = $(DISPLAY_LIB)/mcu_depend/src
DISPLAY_MCU_INC = $(DISPLAY_LIB)/mcu_depend/inc


LIBINCDIRS += $(DISPLAY_INC)			\
			  $(DISPLAY_DRV_INC)		\
			  $(DISPLAY_MCU_INC)		\
			  $(DISPLAY_BMP)

CFILES += \
 $(DISPLAY_MCU_SRC)/display_if_basis.c	\
 $(DISPLAY_SRC)/display_if_support.c

# Set Abstract-layer of Display Driver
include $(DISPLAY_LIB)/display_drv.mk

# IJG JPEG Library
ifeq ($(USE_JPEG_LIB),USE_IJG_LIB)
ifeq ($(MPU_DENSITY),STM32F4XX)
SYNTHESIS_DEFS	+= -DDEFAULT_MAX_MEM=65535UL
else
SYNTHESIS_DEFS	+= -DDEFAULT_MAX_MEM=32768UL
endif
JPEGLIB = $(DISPLAY_LIB)/jpeg/libjpeg
LIBINCDIRS += $(JPEGLIB)
include $(DISPLAY_LIB)/jpeg/jpeglib.mk
CFILES +=\
 $(JPEGLIB)/jidctflt.c
endif

# Chan's TINY JPEG Library
ifeq ($(USE_JPEG_LIB),USE_TINYJPEG_LIB)
JPEGLIB = $(DISPLAY_LIB)/jpeg/tjpgd
LIBINCDIRS += $(JPEGLIB)
CFILES += \
 $(JPEGLIB)/tjpgd.c
endif

# PNG Library
ifeq ($(USE_PNG_LIB),USE_LIBPNG)
PNGLIB = $(DISPLAY_LIB)/png
LIBINCDIRS += $(PNGLIB)/libpng \
              $(PNGLIB)/zlib
include $(PNGLIB)/pnglib.mk
endif

# GIF Library
ifeq ($(USE_GIF_LIB),USE_GIFLIB)
GIFLIB = $(DISPLAY_LIB)/gif
LIBINCDIRS += $(GIFLIB)/giflib
include $(GIFLIB)/giflib.mk
endif

# Display Driver Touch Sence
ifeq ($(USE_TOUCH_SENCE),USE_ADS7843)
CFILES += \
 $(DISPLAY_MCU_SRC)/touch_if_basis.c	\
 $(DISPLAY_SRC)/touch_if.c
endif
