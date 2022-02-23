
MCU_PLATFORM ?= 217G
MPYTHON_USER_DIR = $(TOP_DIR)/user

#COMPONENTS_SRC_LIST =         \
    components/modubinascii.c \
    components/moduhashlib.c \
    components/modujson.c \
    components/modure.c \
    components/mpthreadport.c 

#MACHINE_SRC_LIST =   \
    machine_adc.c \
    machine_hw_i2c.c \
    machine_hw_spi.c \
    machine_i2c.c \
    machine_lowpower.c \
    machine_pin.c \
    machine_spi.c \
    machine_uart.c \
    machine_wdt.c \
    modnetwork.c \
    modpyb.c \
    virtpin.c 

#MACHINE_DRIVER_SRC_LIST=  \
    usr_adc.c \
    usr_file.c \
    usr_gccollect.c \
    usr_i2c.c \
    usr_lpower.c \
    usr_pin.c \
    usr_pin_map.c \
    usr_spi.c \
    usr_stdio.c \
    usr_timer.c \
    usr_uart.c \
    usr_wdt.c  
    #usr_thread.c    

#START_SRC_LIST =        \
    start/src/file.c    \
    start/src/mpy_main.c 


COMPONENTS_SRC_LIST = $(wildcard $(MPYTHON_USER_DIR)/components/Src*.c)

USER_MACHINE_SRC_LIST = \
    $(wildcard $(MPYTHON_USER_DIR)/Machine/Src/*.c) \
    $(wildcard $(MPYTHON_USER_DIR)/Machine/driver/$(MCU_PLATFORM)/*.c)


START_SRC_LIST = $(wildcard $(MPYTHON_USER_DIR)/start/src/*.c)


#USER_MACHINE_SRC_LIST =     \
    $(addprefix Machine/Src/, $(MACHINE_SRC_LIST))  \
    $(addprefix Machine/driver/$(MCU_PLATFORM)/, $(MACHINE_DRIVER_SRC_LIST))

MPYTHON_USER_SRC_LISRT =    \
    $(COMPONENTS_SRC_LIST)  \
    $(USER_MACHINE_SRC_LIST)\
    $(START_SRC_LIST)



MPYTHON_USER_START_INC_DIR =                \
    $(MPYTHON_USER_DIR)/start/Include       \
    $(MPYTHON_USER_DIR)/start/Include/genhdr


MPYTHON_USER_MACHINE_INC_DIR = $(MPYTHON_USER_DIR)/Machine/Include
MPYTHON_USER_COMPONENTS_INC_DIR =  $(MPYTHON_USER_DIR)/components/Include


MPYTHON_USER_INC_DIR_LIST =             \
    $(MPYTHON_USER_START_INC_DIR)       \
    $(MPYTHON_USER_MACHINE_INC_DIR)     \
    $(MPYTHON_USER_COMPONENTS_INC_DIR)

MPYTHON_USER_INC = $(addprefix -I, $(MPYTHON_USER_INC_DIR_LIST))

#MPYTHON_USER_SRC = $(addprefix $(MPYTHON_USER_DIR)/, $(MPYTHON_USER_SRC_LISRT))
MPYTHON_USER_SRC = $(MPYTHON_USER_SRC_LISRT)
