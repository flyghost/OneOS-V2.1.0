
KERNEL_DRIVER_SRC_LIST=     \
    bus/softqspi.c          \
    bus/softspi.c     

LIB_UTILS_SRC_LIST =        \
    utils/interrupt_char.c  \
    utils/mpirq.c           \
    utils/printf.c          \
    utils/pyexec.c          \
    utils/stdout_helpers.c  \
    utils/sys_stdio_mphal.c 

LIB_READLINE_SRC_LIST=      \
    mp-readline/readline.c 

KERNEL_LIB_SRC_LIST =       \
    $(LIB_UTILS_SRC_LIST)   \
    $(LIB_READLINE_SRC_LIST)

MPYTHON_KERNEL_DIR = $(TOP_DIR)/kernel
MPYTHON_KERNEL_PY_DIR = $(MPYTHON_KERNEL_DIR)/py
MPYTHON_KERNEL_EXTMOD_DIR =  $(MPYTHON_KERNEL_DIR)/extmod
KERNEL_PY_SRC_LIST = $(wildcard $(MPYTHON_KERNEL_PY_DIR)/*.c)

MPYTHON_KERNEL_LIB_DIR_lIST = \
    utils \
    mp-readline

MPYTHON_KERNEL_DRIVER_DIR_lIST = \
    bus

MPYTHON_KERNEL_LIB_DIR = $(addprefix $(MPYTHON_KERNEL_DIR)/lib/,$(MPYTHON_KERNEL_LIB_DIR_lIST))
MPYTHON_KERNEL_DRIVER_DIR = $(addprefix $(MPYTHON_KERNEL_DIR)/drivers/,$(MPYTHON_KERNEL_DRIVER_DIR_lIST))


MPYTHON_KERNEL_INC_DIR_LIST =   \
    $(MPYTHON_KERNEL_DIR)    \
    $(MPYTHON_KERNEL_LIB_DIR)   \
    $(MPYTHON_KERNEL_DRIVER_DIR) \
    $(MPYTHON_KERNEL_EXTMOD_DIR) \

MPYTHON_KERNEL_INC = $(addprefix -I, $(MPYTHON_KERNEL_INC_DIR_LIST))

MPYTHON_KERNEL_SRC =   \
    $(KERNEL_PY_SRC_LIST) \
    $(addprefix $(MPYTHON_KERNEL_DIR)/lib/, $(KERNEL_LIB_SRC_LIST)) \
    $(addprefix $(MPYTHON_KERNEL_DIR)/drivers/, $(KERNEL_DRIVER_SRC_LIST)) 
        


