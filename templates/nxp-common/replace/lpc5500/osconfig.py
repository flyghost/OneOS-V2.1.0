import os

# toolchains options
ARCH       = 'arm'
CPU        = 'cortex-m4'
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
elif CROSS_TOOL == 'keil':
    COMPILER    = 'armcc'
    # Notice: The installation path of armcc cannot have Chinese
    COMPILER_PATH   = r'C:/Keil_v5'
elif CROSS_TOOL == 'iar':
    COMPILER    = 'iar'
    COMPILER_PATH   = r'C:/Program Files (x86)/IAR Systems/Embedded Workbench 8.0'


BUILD = 'debug'

if COMPILER == 'gcc':
    # toolchains
    if COMPILER_PATH == '':
        COMPILER_PATH = os.getenv('OS_EXEC_PATH')
    PREFIX = 'arm-none-eabi-'
    CC = PREFIX + 'gcc'
    AS = PREFIX + 'gcc'
    AR = PREFIX + 'ar'
    CXX = PREFIX + 'g++'
    LINK = PREFIX + 'gcc'
    RESULT_SUFFIX = 'elf'
    SIZE = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY = PREFIX + 'objcopy'

    DEVICE = ' -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections'
    CFLAGS = DEVICE + ' -Dgcc'
    AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp -Wa,-mimplicit-it=thumb '
    LFLAGS = DEVICE + ' -Wl,--gc-sections,-Map=oneos.map,-cref,-u,Reset_Handler -T board/linker_scripts/link.lds'

    CPATH = ''
    LPATH = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -gdwarf-2 -g'
        AFLAGS += ' -gdwarf-2'
    else:
        CFLAGS += ' -Os -g'

    CXXFLAGS = CFLAGS 

    POST_ACTION = OBJCPY + ' -R .reserved_ram -O binary $TARGET oneos.bin\n' + SIZE + ' $TARGET \n'

elif COMPILER == 'armcc':
    # toolchains
    CC = 'armclang'
    CXX = 'armclang'
    AS = 'armasm'
    AR = 'armar'
    LINK = 'armlink'
    RESULT_SUFFIX = 'axf'

    CFLAGS = '-xc -std=c99 --target=arm-arm-none-eabi -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -c -fno-rtti -funsigned-char -fshort-enums -fshort-wchar -gdwarf-3 -O0 -ffunction-sections -Id-macro -fno-common  -fdata-sections  -ffreestanding  -fno-builtin  -mthumb -MD '
    
    AFLAGS = ' --cpu cortex-m33 '
    
    LFLAGS = ' --scatter "board\linker_scripts\link.sct" --info sizes --info totals --info unused --info veneers --list oneos.map --strict'
    LFLAGS += ' --libpath=' + COMPILER_PATH + '/ARM/ARMCC/lib'

    COMPILER_PATH += '/ARM/ARMCLANG/bin/'

    if BUILD == 'debug':
        CFLAGS += ' -g -O0 '
        AFLAGS += ' -g'
    else:
        CFLAGS += ' -O2'

    CXXFLAGS = CFLAGS

    POST_ACTION = 'fromelf --bin $TARGET --output oneos.bin \nfromelf -z $TARGET'

elif COMPILER == 'iar':
    # toolchains
    CC = 'iccarm'
    CXX = 'iccarm'
    AS = 'iasmarm'
    AR = 'iarchive'
    LINK = 'ilinkarm'
    RESULT_SUFFIX = 'out'

    DEVICE = '-Dewarm'

    CFLAGS = DEVICE
    CFLAGS += ' --diag_suppress Pa050'
    CFLAGS += ' --no_cse'
    CFLAGS += ' --no_unroll'
    CFLAGS += ' --no_inline'
    CFLAGS += ' --no_code_motion'
    CFLAGS += ' --no_tbaa'
    CFLAGS += ' --no_clustering'
    CFLAGS += ' --no_scheduling'
    CFLAGS += ' --endian=little'
    CFLAGS += ' --cpu=Cortex-M33'
    CFLAGS += ' -e'
    CFLAGS += ' --fpu=VFPv4_sp'
    CFLAGS += ' --dlib_config "' + COMPILER_PATH + '/arm/INC/c/DLib_Config_Normal.h"'
    CFLAGS += ' --silent'

    AFLAGS = DEVICE
    AFLAGS += ' -s+'
    AFLAGS += ' -w+'
    AFLAGS += ' -r'
    AFLAGS += ' --cpu Cortex-M33'
    AFLAGS += ' --fpu VFPv4_sp'
    AFLAGS += ' -S'

    if BUILD == 'debug':
        CFLAGS += ' --debug'
        CFLAGS += ' -On'
    else:
        CFLAGS += ' -Oh'

    LFLAGS = ' --config "board/linker_scripts/link.icf"'
    LFLAGS += ' --entry __iar_program_start'

    CXXFLAGS = CFLAGS
    
    COMPILER_PATH = COMPILER_PATH + '/arm/bin/'
    POST_ACTION = 'ielftool --bin $TARGET oneos.bin'
