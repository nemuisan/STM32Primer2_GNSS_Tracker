#################################################
# MAKEFILE For STM32Primer2 BARE-METAL 			#
# (c) 20091026 Nemui Trinomius					#
# http://nemuisan.blog.bai.ne.jp				#
#################################################

# Environment dependent!!! This environment assume under WINDOWS OS!!!
# Throw path into YOUR environments
export PATH = %SYSTEMROOT%;$(TOOLDIR)/bin;$(OCDIR);$(DBGDIR);$(MAKEDIR);$(RIDEDIR)

# Toolchain Prefix (i.e arm-none-eabi -> arm-none-eabi-gcc.exe)
TCHAIN  = arm-none-eabi

# OpenOCD/FlashTool Prefix
OCD		= openocd
RLINK	= Cortex_pgm

# Development Tools Based on GNU Compiler Collection
DEVTOOL = LAUNCHPAD
#DEVTOOL = BLEEDING_EDGE

# Check Build Tools
ifeq ($(DEVTOOL),LAUNCHPAD)
 TOOLDIR = C:/Devz/ARM/Launchpad
 NANOLIB = --specs=nano.specs
 # NANOLIB += -u _printf_float
 # NANOLIB += -u _scanf_float
 REMOVAL = rm
else ifeq ($(DEVTOOL),BLEEDING_EDGE)
 TOOLDIR = C:/Devz/ARM/Bleeding-edge
 REMOVAL = rm
else
 $(error SET BUILD-TOOLS AT FIRST!!)
endif

# Set UNIX-Like Tools(CoreUtils) Directory
MAKEDIR = C:/Devz/Coreutils/bin


# Set RIDE Directory
RIDEDIR  = C:/Devz/ARM/Raisonance/Ride/bin

# Set Shell Definitions
WSHELL  = cmd
MSGECHO = echo.exe
#MSGECHO = /bin/echo

# Set Debugger Directory
#DBGDIR  = C:/Devz/ARM/CodeLite
#DBG_BIN = codelite.exe
#DBG_CMD = ./lib/codelite_dbg/codelite_dbg.workspace
# Environment Dependent!!!



# GDB DEBUG BUILD Setting
# If set to 1,FORCE to change OPTIMIZE into "-O0 -g -ggdb".
DEBUG_MODE		= 0

# OPTIMIZE Definition
OPTIMIZE		= s

# Force to Optimize OFF in Debug Build
ifeq ($(DEBUG_MODE),1)
 OPTIMIZE		= 0 -g -ggdb
# OPTIMIZE		+= -gdwarf-4
# OPTIMIZE		= g -g -ggdb
endif


# GCC LTO Specific Option(for test...usually broken)
ifneq ($(OPTIMIZE),0)
#USE_LTO			= -flto -fuse-linker-plugin -ffat-lto-objects -flto-partition=none
endif

# GCC Version Specific Options
ALIGNED_ACCESS	= -mno-unaligned-access
ARMV7M_BOOST    = -mslow-flash-data


# Semihosting Definition
#USING_HOSTAGE   = USE_SEMIHOSTING
ifeq ($(USING_HOSTAGE),USE_SEMIHOSTING)
SEMIHOST_LIB = --specs=rdimon.specs -lrdimon
else
START_LIB    = -nostartfiles
endif

# Program Version
APP_VER = 113.00


# Board and MPU Definitions
EVAL_BOARD    	= USE_STM32PRIMER2
MPU_MODEL		= STM32F10X_HD
SUBMODEL		= STM32F103VET6
HSE_CLOCK 		= 12000000
VECTOR_START  	= VECT_TAB_FLASH
PERIF_DRIVER    = USE_STDPERIPH_DRIVER
USE_TOUCH_SENCE =


# Display Drivers Definition
# Use Display Device? MUST See lcddrv.mk
# MUST put before DEFZ
include ./lcddrv.mk

# For JPEG Support(select one of those)
#USE_JPEG_LIB    = USE_IJG_LIB
#USE_JPEG_LIB    = USE_TINYJPEG_LIB

# Display Fonts Definition
# MUST put before DEFZ
#USE_FONTSIZE    = FONT8x8
USE_FONTSIZE    = FONT10x10
#USE_KANJI		= USE_KANJIFONT

# Use FreeRTOS?
OS_SUPPORT		= BARE_METAL
#OS_SUPPORT		= USE_FREERTOS


# Synthesis makefile Defines
DEFZ = $(MPU_CLASS) $(MPU_MODEL) $(SUBMODEL) $(EVAL_BOARD) $(PERIF_DRIVER) $(VECTOR_START) \
	   $(USING_HOSTAGE) $(OS_SUPPORT) $(USE_EXT_SRAM) $(USE_EXT_SDRAM) $(USE_EXT_ROM)	   \
	   $(USE_SYSCLK_FREQ)
# Defines if Display and misc Drivers
DEFZ += $(USE_DISPLAY) $(USE_FONTSIZE) $(USE_KANJI) $(USE_TOUCH_SENCE)  $(USE_XMSTN)	   \
        $(USE_JPEG_LIB) $(USE_PNG_LIB) $(USE_GIF_LIB) $(USE_AUDIO_LIB)  				   \
		$(USE_SOUND_MP3) $(USE_SOUND_AAC) $(USE_SOUND_WAV)
SYNTHESIS_DEFS	= $(addprefix -D,$(DEFZ)) \
				 -DPACK_STRUCT_END=__attribute\(\(packed\)\) \
				 -DALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\) \
				 -DMPU_SUBMODEL=\"$(SUBMODEL)\" \
				 -DAPP_VERSION=\"$(APP_VER)\" \
				 -DHSE_VALUE=$(HSE_CLOCK)UL
# Exchange Uppercase into Lowercase
MPU_MODEL_LOWER = $(strip $(shell echo $(MPU_MODEL) | tr A-Z a-z))


# TARGET Definition
TARGET 		= main
TARGET_ELF  = $(TARGET).elf
TARGET_SREC = $(TARGET).s19
TARGET_HEX  = $(TARGET).hex
TARGET_BIN  = $(TARGET).bin
TARGET_LSS  = $(TARGET).lss
TARGET_SYM  = $(TARGET).sym

# Define CMSIS LIBRARY PATH
CMSISLIB 		= ./lib/CMSIS
CMSIS_DEVICE 	= $(CMSISLIB)/Device/ST/STM32F10x
CMSIS_CORE		= $(CMSISLIB)/Core
# Define SPL and USB LIBRARY PATH
FWLIB  			= ./lib/STM32F10x_StdPeriph_Driver
USBLIB 			= ./lib/STM32_USB-FS-Device_Driver


# Include PATH
INCPATHS	 = 	./							\
				./inc						\
				$(FWLIB)/inc  				\
				$(USBLIB)/inc				\
				$(CMSIS_DEVICE)/Include		\
				$(CMSIS_CORE)/Include		\
				$(LIBINCDIRS)
INCLUDES     = $(addprefix -I ,$(INCPATHS))

# Set library PATH
LIBPATHS     = $(FWLIB) $(USBLIB)
LIBRARY_DIRS = $(addprefix -L,$(LIBPATHS))
# If you use math-library, put "-lm"
MATH_LIB	 =	-lm

# LinkerScript PATH
LINKER_PATH =  ./lib/linker
LINKER_DIRS = $(addprefix -L,$(LINKER_PATH))

# Object Definition
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
 $(SOURCE)/pwr_support.c 			\
 $(SOURCE)/adc_support.c 			\
 $(SOURCE)/rtc_support.c 			\
 $(SOURCE)/gnss_support.c 			\
 $(SOURCE)/cdc_support.c 			\
 $(SOURCE)/msc_support.c 			\
 $(SOURCE)/uart_support_gps.c		\
 $(SOURCE)/xprintf.c

#/*----- Display library PATH -----*/
DISPLAY_LIB	= ./lib/display
ifneq ($(USE_DISPLAY),)
include ./display_cfg_ex.mk
endif

#/*----- FONTX2 Driver library PATH -----*/
FONTX2_LIB	= ./lib/FONTX2
ifneq ($(USE_FONTSIZE),)
include $(FONTX2_LIB)/fontx2_drv.mk
endif

#/*----- FatFs library PATH -----*/
FATFS = ./lib/ff
LIBINCDIRS += $(FATFS)
CFILES += \
 $(FATFS)/ff.c 						\
 $(FATFS)/ffunicode.c				\
 $(FATFS)/ffsystem.c				\
 $(FATFS)/ff_rtc_if.c				\
 $(FATFS)/sdio_stm32f1.c

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
STARTUP_DIR = $(CMSIS_DEVICE)/Source/Templates/gcc
ifeq ($(OS_SUPPORT),USE_FREERTOS)
SFILES += \
	$(SOURCE)/startup_$(MPU_MODEL_LOWER)_rtos.s
else
SFILES += \
	$(STARTUP_DIR)/startup_$(MPU_MODEL_LOWER).s
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

#/*----- Debugging I/O view library -----*/
ifeq ($(DEBUG_MODE),1)
CFILES += \
 ./lib/IOView/stm32f10x_io_view.c
endif


# Toolchain Settings
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
CFLAGS += -O$(OPTIMIZE) $(USE_LTO) $(NANOLIB) $(SEMIHOST_LIB)
CFLAGS += -fno-strict-aliasing -fsigned-char
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fno-schedule-insns2 -fipa-sra
CFLAGS += --param max-inline-insns-single=1000
CFLAGS += -fno-common -fno-hosted
#CFLAGS += -fanalyzer
CFLAGS += -Wall
CFLAGS += -Wdouble-promotion -Woverflow
CFLAGS += -Wstrict-prototypes -Wredundant-decls -Wreturn-type
CFLAGS += -Wshadow -Wunused
CFLAGS += -Wextra -Wfloat-equal
CFLAGS += -Wno-unused-but-set-variable
CFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<)
CFLAGS += $(SYNTHESIS_DEFS)

# Linker FLAGS
LDFLAGS  = -mcpu=cortex-m3 -mthumb -mfix-cortex-m3-ldrd
LDFLAGS += -u g_pfnVectors -Wl,-static -Wl,--gc-sections $(START_LIB)
LDFLAGS += -Wl,-Map=$(TARGET).map
LDFLAGS += -Wl,--print-memory-usage
LDFLAGS += $(LIBRARY_DIRS) $(LINKER_DIRS) $(MATH_LIB)
LDFLAGS +=-T$(LINKER_PATH)/$(SUBMODEL).ld

# Object Copy Relation FLAGS
OBJCPFLAGS = -O
OBJDUMPFLAGS = -h -S -C


# Build Objects
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

# Display Compiler Version Informations
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
# RLINK DOES NOT SUPPORT ANY OSS-DEBUG!!
program :
	$(RLINK) T$(SUBMODEL) E P$(TARGET).hex S


# Drop Files into Dust-Shoot
.PHONY clean:
	@$(MSGECHO) Cleaning Objects:
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).bin
	$(REMOVE) $(TARGET).obj
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
	$(REMOVE) $(wildcard $(DISPLAY_SRC)/*.d)
	$(REMOVE) $(wildcard $(DISPLAY_SRC)/*.lst)
	$(REMOVE) $(wildcard $(DISPLAY_SRC)/*.o)
	$(REMOVE) $(wildcard $(DISPLAY_DRV_SRC)/*.d)
	$(REMOVE) $(wildcard $(DISPLAY_DRV_SRC)/*.lst)
	$(REMOVE) $(wildcard $(DISPLAY_DRV_SRC)/*.o)
	$(REMOVE) $(wildcard $(DISPLAY_MCU_SRC)/*.d)
	$(REMOVE) $(wildcard $(DISPLAY_MCU_SRC)/*.lst)
	$(REMOVE) $(wildcard $(DISPLAY_MCU_SRC)/*.o)
	$(REMOVE) $(wildcard $(FATFS)/*.d)
	$(REMOVE) $(wildcard $(FATFS)/*.lst)
	$(REMOVE) $(wildcard $(FATFS)/*.o)
	$(REMOVE) $(wildcard $(CMSIS_DEVICE)/*.d)
	$(REMOVE) $(wildcard $(CMSIS_DEVICE)/*.lst)
	$(REMOVE) $(wildcard $(CMSIS_DEVICE)/*.o)
	$(REMOVEDIR) $(wildcard ./lib/codelite_dbg/.codelite)
	$(REMOVEDIR) .dep
	@$(MSGECHO)

# Listing of PHONY Targets
.PHONY : all build clean begin finish end sizebefore sizeafter \
		 gccversion buildinform elf hex bin lss sym \
		 build_target clean_list program debug
