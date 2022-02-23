# ------------------------------------------------
#   Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#	   
# ------------------------------------------------


##########################################################################################
#    environment variables
##########################################################################################

-include kernel/kernel.mk
-include user/user.mk
-include user/stm32f411.mk
-include user/217G.mk

##########################################################################################
# 														target
##########################################################################################
TARGET = Mpython1.0

##########################################################################################
# 														platform (linux, windows)
##########################################################################################
p = linux
PLATFORM = $(p)

ifeq ($(PLATFORM), $(filter $(PLATFORM),linux l L Linux LINUX))
ROOT_DIR :=$(shell pwd)
else
ifeq ($(PLATFORM), $(filter $(PLATFORM),windows w W Windows WINDOWS))
ROOT_DIR := .
else
$(error You input an error parameters! plase run 'make help' commond!)
endif
endif

TOP_DIR = $(ROOT_DIR)
##########################################################################################
# 	说明：
#  MCU_PREFIX: mcu型号前缀
#  MCU_NUM： mcu的系列数字
#  MCU_TAIL： mcu型号尾巴
#  例1：mcu型号是stm32f411xE （请看 stm32f4xx.h 具体要定义哪个宏才能生效， 这里会应该是
#   STM32F411xE）
#  所以 MCU_PREFIX = stm32f4 （下面会转化大小写） MCU_NUM= 11， MCU_TAIL = xE
#  例2：mcu型号是stm32l476rg （请看 stm32l4xx.h 具体要定义哪个宏才能生效， 这里会应该是
#   STM32L476xx）
#  所以 MCU_PREFIX = stm32l4 （下面会转化大小写） MCU_NUM= 76， MCU_TAIL = xx
##########################################################################################
MCU_NUM = 11
MCU_PREFIX = stm32f4
MCU_TAIL = xE
MCU_PLATFORM = 217G
MCU_PLATFORM ?= $(MCU_PREFIX)$(MCU_NUM)

CMSIS_MCU = $(shell echo $(MCU_PREFIX) | tr '[:lower:]' '[:upper:]')


##########################################################################################
# 														cross compile
##########################################################################################
#CROSS_COMPILE_DIR = C:/CSDTK/CSDTK4.0/mips-elf-4.4.2/bin
CROSS_COMPILE = mips-elf-
CROSS_COMPILE ?= arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
GCC_AS = $(CC) -x assembler-with-cpp
AR =$(CROSS_COMPILE)ar
SIZE = $(CROSS_COMPILE)size
Q=@

export TOP_DIR
export MCU_PLATFORM
export CMSIS_MCU

#BUILD ?= out/chowOS
BUILD = lib

TARGET_DIR = $(TOP_DIR)/../$(BUILD)
BUILD_DIR ?= $(TARGET_DIR)/$(TARGET)
 

#ifeq ($(wildcard $(OS_DRIVER_PATH)/config/$(CMSIS_MCU)xx/.),)
#$(error Invalid BOARD specified: $(OS_DRIVER_PATH)/config/$(CMSIS_MCU)xx)
#else 
#@echo "$(OS_DRIVER_PATH)/config/$(CMSIS_MCU)xx"
#endif

LD_DIR=$(BUILD)/$(TARGET)


##########################################################################################
# 														building variables
##########################################################################################
# debug build?
DEBUG = 1

# Debugging/Optimization
COPT = -Og



##########################################################################################
#															CFLAGS
##########################################################################################

# cpu
ifeq ($(MCU_PLATFORM), 217G)
CPU = 
FPU =
FLOAT-ABI =

else
#|------------|-------------------------------------------|----------------|
#|	Cortex-M4 | -mthumb -mcpu=cortex-m4 -mfloat-abi=hard	| thumb					 |
#|  (Hard FP) | -mfpu=fpv4-sp-d16													| fpv4-sp /hard	 |
#|------------|-------------------------------------------|----------------|
CPU = -mcpu=cortex-m4

# Select hardware floating-point support
ifeq ($(CMSIS_MCU)xx,$(filter $(CMSIS_MCU)xx,STM32F765xx STM32F767xx STM32F769xx STM32H743xx))
FPU = -mfpu=fpv5-d16
FLOAT-ABI = -mfloat-abi=hard
else
ifeq ($(MCU_SERIES),f0)
FPU =
FLOAT-ABI = -msoft-float
else
# fpu
FPU = -mfpu=fpv4-sp-d16
# float-abi
FLOAT-ABI = -mfloat-abi=hard
endif
endif

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
endif
# macros for gcc
# AS defines
AS_DEFS = 

# C defines

ifeq ($(MCU_PLATFORM), 217G)
C_DEFS = -DCROSS_COMPILE_MIPS_GCC
else
C_DEFS =  \
-DUSE_HAL_DRIVER \
-D$(CMSIS_MCU)$(MCU_NUM)$(MCU_TAIL)
endif


# AS includes
AS_INCLUDES = 

##########################################################################################
#															C includes
##########################################################################################
#SYS_INC = C:/CSDTK/CSDTK4.0/mips-elf-4.4.2/mips-elf/sys-include
INC = $(MPYTHON_KERNEL_INC)
INC += $(MPYTHON_USER_INC)
INC += $(MCU_INC)
#INC += -I$(SYS_INC)
#INC += -I$(SYS_INC)/machine
#INC += -I$(SYS_INC)/sys

ifeq ($(MCU_PLATFORM), 217G)

CFLAGS = $(INC) $(C_DEFS) -std=gnu99

else
# Basic Cortex-M flags
#please read share\doc\gcc-arm-none-eabi\readme.txt file.
#support thumb conditional instruction (IT block) such as 'vstmdbeq' 
#'moveq' 'vldmiane' 'bicne' and so on
CFLAGS_CORTEX_M = -Wa,-mimplicit-it=thumb

# Options for particular MCU series
CFLAGS_MCU_l4 = $(CFLAGS_CORTEX_M) -mtune=cortex-m4 

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(COPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(INC) $(COPT) -Wall -fdata-sections -ffunction-sections
CFLAGS += $(CFLAGS_MCU_l4)
CFLAGS += -Wpointer-arith -std=gnu99 -nostdlib #-Werror


ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

endif #end ($(MCU_PLATFORM), 217G)

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


##########################################################################################
# 														source
##########################################################################################
# C sources

MPY_SRC_C = $(MPYTHON_KERNEL_SRC)
MPY_SRC_C += $(MPYTHON_USER_SRC)


# default action: build all
all: $(TARGET_DIR)/lib$(TARGET).a

##########################################################################################
# 														build the application
##########################################################################################
# list of objects

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(MPY_SRC_C:.c=.o)))
vpath %.c $(sort $(dir $(MPY_SRC_C)))

$(BUILD_DIR)/%.o: %.c  Makefile | $(BUILD_DIR) 
	@echo 'Building C file: $<'
	$(Q)$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@


$(TARGET_DIR)/lib$(TARGET).a : $(OBJECTS) Makefile
	@echo 'LINK : $<'
	$(Q)$(AR) rs $@ $(OBJECTS)

$(BUILD_DIR):
	mkdir -p $@		

.PHONY: clean test help
##########################################################################################
# clean up
##########################################################################################
clean:
	-rm -fR $(TARGET_DIR)
  

test:
	@echo "INC = $(INC)"
	@echo "CFLAGS = $(CFLAGS)"
#	@echo "$(CMSIS_MCU)$(MCU_NUM)xx  = $(CMSIS_MCU)$(MCU_NUM)xx"
#	@echo "$(OS_DRIVER_PATH)/config/$(CMSIS_MCU)xx"
	@echo "CFLAGS = $(CFLAGS)"
#	@echo "OS_SRC_S = $(OS_SRC_S)"




help:
	@echo "-----------------help--------------------"
	@echo "p : platform(linux or windows). "
	@echo "  examples:make p=linux (or make, make p=l,L,Linux, LINUX) means compile code on linux. "
	@echo "           make p=windows (p=w,W,Windows, WINDOWS)means compile code on windows!"


##########################################################################################
# dependencies
##########################################################################################
-include $(wildcard $(BUILD_DIR)/*.d)
