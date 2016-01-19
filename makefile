#################################################
# MAKEFILE For STM32Primer2 BARE-METAL 			#
# (c) 20091026 Nemui Trinomius					#
# http://nemuisan.blog.bai.ne.jp				#
#################################################

# Environment Dependent!!! This Environment assure under WINDOWS !!
# Throw path into YOUR environments
export PATH = %SYSTEMROOT%;$(TOOLDIR)/arm-gcc/bin;$(TOOLDIR)/bin;$(MAKEDIR);$(RIDEDIR)

# Environment Dependent!!!
# Toolchain prefix (i.e arm-none-eabi -> arm-none-eabi-gcc.exe)
TCHAIN  = arm-none-eabi
OCD		= openocd
RLINK	= Cortex_pgm

# Development Tools based on GNU Compiler Collection
DEVTOOL = LAUNCHPAD
#DEVTOOL = BLEEDING_EDGE
#DEVTOOL = YAGARTO
#DEVTOOL = DEVKITARM
#DEVTOOL = SOURCERY

# Check BuildTools
ifeq ($(DEVTOOL),LAUNCHPAD)
 TOOLDIR = C:/Devz/ARM/Launchpad
# NANOLIB = --specs=nano.specs
#  NANOLIB += -u _printf_float
#  NANOLIB += -u _scanf_float
 REMOVAL = rm
else ifeq ($(DEVTOOL),BLEEDING_EDGE)
 TOOLDIR = C:/Devz/ARM/Bleeding-edge
 REMOVAL = rm
else ifeq ($(DEVTOOL),YAGARTO)
 TOOLDIR = C:/Devz/ARM/Yagarto
 REMOVAL = rm
else ifeq ($(DEVTOOL),DEVKITARM)
 TOOLDIR = C:/Devz/ARM/devkitARM
 REMOVAL = rm
else ifeq ($(DEVTOOL),SOURCERY)
 TOOLDIR = C:/Devz/ARM/Sourcery
 REMOVAL = rm
else
 $(error SET BUILD-TOOLS AT FIRST!!)
endif


# Set UNIX-Like tools (Coreutils)
MAKEDIR = C:/Devz/Coreutils/bin
# Set RIDE Directory
RIDEDIR  = C:/Devz/ARM/Raisonance/Ride/bin

WSHELL  = cmd
MSGECHO = echo.exe
GDBDIR  = C:/Devz/ARM/insight/bin
INSIGHT = $(GDBDIR)/arm-none-eabi-insight
# Environment Dependent!!!

# OPTIMIZE definition
OPTIMIZE		= s
#OPTIMIZE		= 0

# GCC LTO Specific Option
ifneq ($(OPTIMIZE),0)
USE_LTO			= -flto-partition=none -fipa-sra
#USE_LTO			= -flto -fipa-sra
endif
# GCC Version Specific Options
ALIGNED_ACCESS	= -mno-unaligned-access
ARMV7M_BOOST    = -mslow-flash-data


# Apprication Version
APP_VER = Version42.00
# Basic definition
EVAL_BOARD    	= USE_STM32PRIMER2
MPU_MODEL		= STM32F10X_HD
SUBMODEL		= STM32F103VET6
HSE_CLOCK 		= 12000000
VECTOR_START  	= VECT_TAB_FLASH
PERIF_DRIVER    = USE_STDPERIPH_DRIVER
USE_TOUCH_SENCE =


# Display Drivers Definition
# Use Display Device? MUST See lcddrv.mk
include ./lcddrv.mk

# For JPEG Support
#USE_JPEG_LIB    = USE_TINYJPEG_LIB
#USE_JPEG_LIB    = USE_IJG_LIB

# Use Display Fonts?
#USE_FONTSIZE    = FONT8x8
USE_FONTSIZE    = FONT10x10
#USE_KANJI		= USE_KANJIFONT

# Use FreeRTOS?
OS_SUPPORT		= BARE_METAL
#OS_SUPPORT		= USE_FREERTOS

# Synthesis makefile Defines
DEFZ = $(MPU_CLASS) $(MPU_MODEL) $(SUBMODEL) $(EVAL_BOARD) $(PERIF_DRIVER) $(VECTOR_START) \
	   $(USING_HOSTAGE) $(OS_SUPPORT) $(USE_EXT_SRAM) $(USE_EXT_SDRAM) $(USE_EXT_ROM)
# Defines if Display and misc Drivers
DEFZ += $(USE_DISPLAY) $(USE_FONTSIZE) $(USE_KANJI) $(USE_TOUCH_SENCE)  $(USE_XMSTN)	   \
        $(USE_JPEG_LIB) $(USE_PNG_LIB) $(USE_GIF_LIB) $(USE_AUDIO_LIB)  				   \
		$(USE_SOUND_MP3)  $(USE_SOUND_WAV)
SYNTHESIS_DEFS	= $(addprefix -D,$(DEFZ)) 							\
				 -DARM_MATH_CM3										\
				 -DPACK_STRUCT_END=__attribute\(\(packed\)\) 		\
				 -DALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\) \
				 -DMPU_SUBMODEL=\"$(SUBMODEL)\"						\
				 -DAPP_VERSION=\"$(APP_VER)\"						\
				 -DHSE_VALUE=$(HSE_CLOCK)UL

# TARGET definition
TARGET 		= main
TARGET_ELF  = $(TARGET).elf
TARGET_SREC = $(TARGET).s19
TARGET_HEX  = $(TARGET).hex
TARGET_BIN  = $(TARGET).bin
TARGET_LSS  = $(TARGET).lss
TARGET_SYM  = $(TARGET).sym

# define CMSIS LIBRARY PATH
FWLIB  			= ./lib/STM32F10x_StdPeriph_Driver
USBLIB 			= ./lib/STM32_USB-FS-Device_Driver
CMSISLIB 		= ./lib/CMSIS
CMSIS_DEVICE 	= $(CMSISLIB)/Device/ST/STM32F10x
CMSIS_CORE		= $(CMSISLIB)/Include


# include PATH
INCPATHS	 = 	./							\
				./inc						\
				$(FWLIB)/inc  				\
				$(USBLIB)/inc				\
				$(CMSIS_DEVICE)/Include		\
				$(CMSIS_CORE)				\
				$(LIBINCDIRS)
INCLUDES     = $(addprefix -I ,$(INCPATHS))

# Set library PATH
LIBPATHS     = $(FWLIB) $(USBLIB) $(CMSISLIB)
LIBRARY_DIRS = $(addprefix -L,$(LIBPATHS))
# if you use math-library, put "-lm" 
MATH_LIB	 =	-lm

# LinkerScript PATH
LINKER_PATH =  ./lib/linker
LINKER_DIRS = $(addprefix -L,$(LINKER_PATH)) 

# Object definition
OBJS 	 = $(CFILES:%.c=%.o) 
LIBOBJS  = $(LIBCFILES:%.c=%.o) $(SFILES:%.s=%.o)

# C code PATH
SOURCE  = ./src
CFILES = \
 $(SOURCE)/$(TARGET).c 				\
 $(SOURCE)/sysinit_primer2.c		\
 $(SOURCE)/systick.c				\
 $(SOURCE)/syscalls.c				\
 $(SOURCE)/stm32f10x_it.c			\
 $(SOURCE)/hw_config.c 				\
 $(SOURCE)/power.c 					\
 $(SOURCE)/adc_support.c 			\
 $(SOURCE)/rtc_support.c 			\
 $(SOURCE)/gps_support.c 			\
 $(SOURCE)/cdc_support.c 			\
 $(SOURCE)/msc_support.c 			\
 $(SOURCE)/uart_support_gps.c		\
 $(SOURCE)/xprintf.c

#/*----- Display library PATH -----*/	
DISPLAY_LIB	= ./lib/display
ifneq ($(USE_DISPLAY),)
include ./display_cfg_ex.mk
endif

#/*----- FONX2 Driver library PATH -----*/	
FONTX2_LIB	= ./lib/FONTX2
ifneq ($(USE_FONTSIZE),)
include $(FONTX2_LIB)/fontx2_drv.mk
endif

#/*----- xMSTN Display library PATH -----*/	
xMSTN_LIB	= ./lib/xMSTN
ifneq ($(USE_XMSTN),)
include $(xMSTN_LIB)/xmstn_drv.mk
endif

#/*----- FatFs library PATH -----*/	
FATFS = ./lib/ff
LIBINCDIRS += $(FATFS)
CFILES += \
 $(FATFS)/ff.c 							\
 $(FATFS)/sdio_stm32f1.c 				\
 $(FATFS)/ff_rtc_if.c 					\
 $(FATFS)/option/syscall.c				\
 $(FATFS)/option/unicode.c
SFILES += $(FATFS)/up_memcpy.s

#/*----- USB-Function library PATH -----*/
USBFUNC = ./lib/USB_Functions
LIBINCDIRS += $(USBFUNC)
CFILES += \
 $(USBFUNC)/usb_pwr.c 					\
 $(USBFUNC)/usb_istr.c 					\
 $(USBFUNC)/usb_conf.c 					\
 $(USBFUNC)/usb_prop.c
 
#/*----- USB-CDC library PATH -----*/
USBCDC =  $(USBFUNC)/cdc
LIBINCDIRS += $(USBCDC)/inc
CFILES += \
 $(USBCDC)/src/usb_cdc_desc.c 			\
 $(USBCDC)/src/usb_cdc_prop.c 			\
 $(USBCDC)/src/usb_cdc_endp.c 				

#/*----- USB-MSC library PATH -----*/
USBMSC =  $(USBFUNC)/msc
LIBINCDIRS += $(USBMSC)/inc
CFILES += \
 $(USBMSC)/src/usb_msc_mass_mal.c 		\
 $(USBMSC)/src/usb_msc_memory.c 		\
 $(USBMSC)/src/usb_msc_scsi_data.c 		\
 $(USBMSC)/src/usb_msc_scsi.c			\
 $(USBMSC)/src/usb_msc_bot.c 			\
 $(USBMSC)/src/usb_msc_desc.c 			\
 $(USBMSC)/src/usb_msc_prop.c 			


#/*----- STARTUP code PATH -----*/
STARTUP_DIR = $(CMSIS_DEVICE)/Source/Templates/gcc_ride7
ifeq ($(MPU_MODEL),STM32F10X_HD)
SFILES += \
	$(STARTUP_DIR)/startup_stm32f10x_hd.s
else
    $(error U MUST SELECT STM32F1_HD Device!!)
endif

#/*----- STM32 library PATH -----*/
LIBCFILES = \
 $(FWLIB)/src/misc.c \
 $(FWLIB)/src/stm32f10x_flash.c 	\
 $(FWLIB)/src/stm32f10x_gpio.c 		\
 $(FWLIB)/src/stm32f10x_fsmc.c 		\
 $(FWLIB)/src/stm32f10x_rcc.c 		\
 $(FWLIB)/src/stm32f10x_adc.c 		\
 $(FWLIB)/src/stm32f10x_dma.c 		\
 $(FWLIB)/src/stm32f10x_tim.c 		\
 $(FWLIB)/src/stm32f10x_rtc.c 		\
 $(FWLIB)/src/stm32f10x_sdio.c 		\
 $(FWLIB)/src/stm32f10x_usart.c 	\
 $(FWLIB)/src/stm32f10x_bkp.c 		\
 $(FWLIB)/src/stm32f10x_pwr.c 		\
 $(FWLIB)/src/stm32f10x_iwdg.c 		\
 $(USBLIB)/src/usb_core.c 			\
 $(USBLIB)/src/usb_init.c 			\
 $(USBLIB)/src/usb_int.c 			\
 $(USBLIB)/src/usb_mem.c 			\
 $(USBLIB)/src/usb_regs.c 			\
 $(USBLIB)/src/usb_sil.c
 
#/*----- STM32 Debug library -----*/
ifeq ($(OPTIMIZE),0)
CFILES += \
 ./lib/IOView/stm32f10x_io_view.c
else
endif


# TOOLCHAIN SETTING
CC 			= $(TCHAIN)-gcc
CPP 		= $(TCHAIN)-g++
OBJCOPY 	= $(TCHAIN)-objcopy
OBJDUMP 	= $(TCHAIN)-objdump
SIZE 		= $(TCHAIN)-size
AR 			= $(TCHAIN)-ar
LD 			= $(TCHAIN)-gcc
NM 			= $(TCHAIN)-nm
REMOVE		= $(REMOVAL) -f
REMOVEDIR 	= $(REMOVAL) -rf

# C and ASM FLAGS
CFLAGS  = -MD -mcpu=cortex-m3 -mtune=cortex-m3 -mfix-cortex-m3-ldrd
CFLAGS += -mthumb -mlittle-endian $(ALIGNED_ACCESS) $(ARMV7M_BOOST)
CFLAGS += -mno-sched-prolog -msoft-float
CFLAGS += -std=gnu99
CFLAGS += -gdwarf-2 -O$(OPTIMIZE) $(USE_LTO) $(NANOLIB)
CFLAGS += -fno-strict-aliasing -fsigned-char
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fno-schedule-insns2
CFLAGS += --param max-inline-insns-single=1000
CFLAGS += -fno-common -fno-hosted
CFLAGS += -Wall -Wno-array-bounds
#CFLAGS += -Wdouble-promotion
#CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
CFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<) 
CFLAGS += $(SYNTHESIS_DEFS)  

# Linker FLAGS
LDFLAGS  = -mcpu=cortex-m3 -mthumb -mfix-cortex-m3-ldrd
LDFLAGS += -u g_pfnVectors -Wl,-static -Wl,--gc-sections -nostartfiles
LDFLAGS += -Wl,-Map=$(TARGET).map
LDFLAGS += $(LIBRARY_DIRS) $(LINKER_DIRS) $(MATH_LIB) 
LDFLAGS +=-T$(LINKER_PATH)/$(SUBMODEL).ld

# Object Copy and dfu generation FLAGS
OBJCPFLAGS = -O
OBJDUMPFLAGS = -h -S -C
DFU	  = hex2dfu
DFLAGS = -w


# Build Object
all: gccversion clean build buildinform sizeafter
build: $(TARGET_ELF) $(TARGET_LSS) $(TARGET_SYM) $(TARGET_HEX) $(TARGET_SREC) $(TARGET_BIN)

.SUFFIXES: .o .c .s   

$(TARGET_LSS): $(TARGET_ELF)
	@$(MSGECHO)
	@$(MSGECHO) Disassemble: $@
	$(OBJDUMP) $(OBJDUMPFLAGS) $< > $@ 
$(TARGET_SYM): $(TARGET_ELF)
	@$(MSGECHO)
	@$(MSGECHO) Symbol: $@
	$(NM) -n $< > $@
$(TARGET).hex: $(TARGET).elf
	@$(MSGECHO)
	@$(MSGECHO) Objcopy: $@
	$(OBJCOPY) $(OBJCPFLAGS) ihex $^ $@    
$(TARGET).s19: $(TARGET).elf
	@$(MSGECHO)
	@$(MSGECHO) Objcopy: $@
	$(OBJCOPY) $(OBJCPFLAGS) srec $^ $@ 
$(TARGET).bin: $(TARGET).elf
	@$(MSGECHO)
	@$(MSGECHO) Objcopy: $@
	$(OBJCOPY) $(OBJCPFLAGS) binary $< $@ 
$(TARGET).dfu: $(TARGET).hex
	@$(MSGECHO)
	@$(MSGECHO) Make STM32 dfu: $@
	$(DFU) $(DFLAGS) $< $@
	@$(MSGECHO)
$(TARGET).elf: $(OBJS) $(SUBMODEL)_lib.a
	@$(MSGECHO) Link: $@
	$(LD) $(CFLAGS) $(LDFLAGS) $^ -o $@
	@$(MSGECHO)

$(SUBMODEL)_lib.a: $(LIBOBJS)
	@$(MSGECHO) Archive: $@
	$(AR) cr $@ $(LIBOBJS)    
	@$(MSGECHO)
.c.o:
	@$(MSGECHO) Compile: $<
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@
	@$(MSGECHO)
.s.o:
	@$(MSGECHO) Assemble: $<
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@
	@$(MSGECHO)

# Object Size Informations
sizeafter:
	@$(MSGECHO) 
	@$(MSGECHO) Built Object Informations:
	@$(MSGECHO) === Total Binary Size ===
	@$(SIZE) $(TARGET).hex
	@$(MSGECHO) === Verbose ELF Size ===
	@$(SIZE) $(TARGET).elf
	@$(SIZE) -A -x $(TARGET).elf

# Display compiler version information.
gccversion : 
	@$(CC) --version
	@$(MSGECHO) 

buildinform :
	@$(MSGECHO) 
	@$(MSGECHO) 
	@$(MSGECHO) Built Informations:
	@$(MSGECHO) USING_SYSTEM = $(OS_SUPPORT)
	@$(MSGECHO) USING_DISPLAY = $(USE_DISPLAY)
	@$(MSGECHO) USING_DEVBOARD = $(EVAL_BOARD)

# Flash and Debug Program
debug :

program :
	$(RLINK) T$(SUBMODEL) E P$(TARGET).hex S


# Drop files into dust-shoot
.PHONY clean:
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).bin
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(SUBMODEL)_lib.a
	$(REMOVE) $(wildcard *_lib.a)
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).s19
	$(REMOVE) $(TARGET).a90
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lnk
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(TARGET).dfu
	$(REMOVE) $(wildcard *.stackdump)
	$(REMOVE) $(OBJS)
	$(REMOVE) $(AOBJ)
	$(REMOVE) $(LIBOBJS)
	$(REMOVE) $(LST)
	$(REMOVE) $(CFILES:.c=.lst)
	$(REMOVE) $(CFILES:.c=.d)
	$(REMOVE) $(LIBCFILES:.c=.lst)
	$(REMOVE) $(LIBCFILES:.c=.d)
	$(REMOVE) $(SFILES:.s=.lst)
	$(REMOVE) $(wildcard ./lib/IOView/*.d)
	$(REMOVE) $(wildcard ./lib/IOView/*.lst)
	$(REMOVE) $(wildcard ./lib/IOView/*.o)
	$(REMOVE) $(wildcard $(DISPLAY_DRV_SRC)/*.d)
	$(REMOVE) $(wildcard $(DISPLAY_DRV_SRC)/*.lst)
	$(REMOVE) $(wildcard $(DISPLAY_DRV_SRC)/*.o)
	$(REMOVE) $(wildcard $(FATFS)/*.d)
	$(REMOVE) $(wildcard $(FATFS)/*.lst)
	$(REMOVE) $(wildcard $(FATFS)/*.o)
	$(REMOVE) $(wildcard $(CMSIS_DEVICE)/*.d)
	$(REMOVE) $(wildcard $(CMSIS_DEVICE)/*.lst)
	$(REMOVE) $(wildcard $(CMSIS_DEVICE)/*.o)
	$(REMOVEDIR) .dep
	@$(MSGECHO)

# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex bin lss sym clean clean_list program
