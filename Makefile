# Name of the binaries.
PROJ_NAME=mems

######################################################################
#                         SETUP SOURCES                              #
######################################################################


# This is the directory containing the firmware package,
# the unzipped folder downloaded from here:
# http://www.st.com/web/en/catalog/tools/PF257904
STM_DIR=/Users/chamod/Documents/stm32/STM32Cube_FW_F4_V1.6.0/Drivers

# This is where the source files are located,
# which are not in the current directory
# (the sources of the standard peripheral library, which we use)
# see also "info:/make/Selective Search" in Konqueror
STM_SRC = $(STM_DIR)/STM32F4xx_HAL_Driver/Src
STM_SRC += $(STM_DIR)/BSP/STM32F429I-Discovery
STM_SRC += $(STM_DIR)/../Utilities/Log
STM_SRC += $(STM_DIR)/BSP/Components/stmpe811
STM_SRC += $(STM_DIR)/BSP/Components/ili9341
STM_SRC += $(STM_DIR)/BSP/Components/l3gd20
# Tell make to look in that folder if it cannot find a source
# in the current directory
vpath %.c $(STM_SRC)

# My source file
SRCS   = Src/main.c
SRCS  += Src/*.c
# Contains initialisation code and must be compiled into
# our project. This file is in the current directory and
# was writen by ST.
SRCS  += Src/system_stm32f4xx.c
# These source files implement the functions we use.
# make finds them by searching the vpath defined above.
#SRCS  += stm32f4xx_rcc.c 
SRCS += stm32f4xx_hal_uart.c
SRCS += stm32f4xx_hal_gpio.c
SRCS += stm32f4xx_hal_i2c.c
SRCS += stm32f4xx_hal_spi.c
SRCS += stm32f4xx_hal_dma.c
SRCS += stm32f4xx_hal_rtc.c
SRCS += stm32f429i_discovery_gyroscope.c
SRCS += stm32f429i_discovery_lcd.c
SRCS += stm32f4xx_hal_dma2d.c
SRCS += stm32f4xx_hal_rcc.c
SRCS += stm32f4xx_hal_ltdc.c
SRCS += stm32f4xx_hal_sdram.c
SRCS += stm32f4xx_ll_fmc.c
SRCS += stm32f4xx_hal_pwr_ex.c
SRCS += stm32f4xx_hal_rcc_ex.c
SRCS += stm32f4xx_hal.c
SRCS += stm32f4xx_hal_cortex.c
SRCS += stm32f429i_discovery.c
SRCS += stm32f429i_discovery_sdram.c
SRCS += lcd_log.c
SRCS += l3gd20.c
SRCS += ili9341.c
SRCS += stmpe811.c
# Startup file written by ST
# The assembly code in this file is the first one to be
# executed. Normally you do not change this file.
SRCS += TrueSTUDIO/startup_stm32f429xx.s

# The header files we use are located here
INC_DIRS  = $(STM_DIR)/STM32F4xx_HAL_Driver/Inc
INC_DIRS += $(STM_DIR)/../Utilities/Log
INC_DIRS += $(STM_DIR)/BSP/STM32F429I-Discovery
INC_DIRS += Inc
INC_DIRS += $(STM_DIR)/CMSIS/Device/ST/STM32F4xx/Include
INC_DIRS += $(STM_DIR)/CMSIS/Include



INCS = stm32f4xx_ll_fmc.h  
# in case we have to many sources and don't want 
# to compile all sources every time
# OBJS = $(SRCS:.c=.o)


######################################################################
#                         SETUP TOOLS                                #
######################################################################


# This is the path to the toolchain
# (we don't put our toolchain on $PATH to keep the system clean)
TOOLS_DIR = /Users/chamod/gcc-arm-none-eabi-4_9-2015q1/bin

# The tool we use
CC      = $(TOOLS_DIR)/arm-none-eabi-gcc
OBJCOPY = $(TOOLS_DIR)/arm-none-eabi-objcopy
GDB     = $(TOOLS_DIR)/arm-none-eabi-gdb

## Preprocessor options

# directories to be searched for header files
INCLUDE = $(addprefix -I,$(INC_DIRS))

# #defines needed when working with the STM library
DEFS    = -DUSE_STDPERIPH_DRIVER
DEFS	+= -DSTM32F429xx
DEFS += -DEE_M24LR64
# if you use the following option, you must implement the function 
#    assert_failed(uint8_t* file, uint32_t line)
# because it is conditionally used in the library
# DEFS   += -DUSE_FULL_ASSERT

## Compiler options
CFLAGS  = -ggdb
# please do not optimize anything because we are debugging
CFLAGS += -O0 
CFLAGS += -Wall -Wextra -Warray-bounds
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += --specs=nosys.specs
## Linker options
# tell ld which linker file to use
# (this file is in the current directory)
LFLAGS  = -TSW4STM32/STM32F429I-Discovery/STM32F429ZITx_FLASH.ld


######################################################################
#                         SETUP TARGETS                              #
######################################################################

.PHONY: $(PROJ_NAME)
$(PROJ_NAME): $(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
#$(CC) $(INCLUDE) $(DEFS) $(CFLAGS)   $(LFLAGS) $^ -o $@ 
	 $(CC) $(INCLUDE) $(DEFS) $(CFLAGS) -include $(INCS) $(LFLAGS) $^ -o $@
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf   $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f *.o $(PROJ_NAME).elf $(PROJ_NAME).hex $(PROJ_NAME).bin

# Flash the STM32F4
flash: 
	st-flash write $(PROJ_NAME).bin 0x8000000

.PHONY: debug
debug:
# before you start gdb, you must start st-util
	$(GDB) $(PROJ_NAME).elf
