# Select TFT-LCD Driver Each One!

USE_DISPLAY		= USE_ST7732_TFT

# Fool Proof
ifeq ($(USE_DISPLAY),)
 $(error U MUST SELECT TFT-LCD DRIVER FROM lcddrv.mk!!)
endif
