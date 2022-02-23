import os

# toolchains options
ARCH       = 'risc-v'
CPU        = 'bumblebee'
CROSS_TOOL = 'gcc'

# bsp lib config
BSP_LIBRARY_TYPE = None

if os.getenv('OS_CC'):
    CROSS_TOOL = os.getenv('OS_CC')
if os.getenv('OS_ROOT'):
    OS_ROOT = os.getenv('OS_ROOT')

# cross_tool provides the cross compiler
# COMPILER_PATH is the compiler execute path, for example, CodeSourcery, Keil MDK, IAR
if  CROSS_TOOL == 'gcc':
    COMPILER    = 'gcc'
    COMPILER_PATH = ''


BUILD = 'debug'

if COMPILER == 'gcc':
    # toolchains
    if COMPILER_PATH == '':
        COMPILER_PATH = os.getenv('OS_EXEC_PATH')
    PREFIX = 'riscv-nuclei-elf-'
    CC = PREFIX + 'gcc'
    AS = PREFIX + 'gcc'
    AR = PREFIX + 'ar'
    CXX = PREFIX + 'g++'
    LINK = PREFIX + 'gcc'
    RESULT_SUFFIX = 'elf'
    SIZE = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY = PREFIX + 'objcopy'

    DEVICE = ' -march=rv32imac -mabi=ilp32 -mcmodel=medlow -msmall-data-limit=8 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common'

    CFLAGS = DEVICE + ' -Dgcc'

    AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp'
    AFLAGS += ' -I. -I../../arch/risc-v/common/ -IRISCV/drivers/'

    LFLAGS = DEVICE + ' -nostartfiles -Xlinker  --gc-sections -Wl,-Map,"gd32vf103cbt6-longan-nano.map" --specs=nano.specs -T RISCV\env_Eclipse\GD32VF103xB.lds -nostartfiles'

    CPATH = ''
    LPATH = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -gdwarf-2 -g'
        AFLAGS += ' -gdwarf-2'
    else:
        CFLAGS += ' -Os -g'

    CXXFLAGS = CFLAGS 

    POST_ACTION = OBJCPY + ' -R .reserved_ram -O binary $TARGET oneos.bin\n' + SIZE + ' $TARGET \n'
