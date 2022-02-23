import os

# toolchains options
ARCH       = 'mips'
CPU        = 'x1000e'
CROSS_TOOL = 'gcc'

# bsp lib config
BSP_LIBRARY_TYPE = None

if os.getenv('OS_CC'):
    CROSS_TOOL = os.getenv('OS_CC')
if os.getenv('OS_ROOT'):
    OS_ROOT = os.getenv('OS_ROOT')

# cross_tool provides the cross compiler
if  CROSS_TOOL == 'gcc':
    COMPILER    = 'gcc'
    COMPILER_PATH = ''


BUILD = 'debug'
#BUILD = 'release'

if COMPILER == 'gcc':
    # toolchains
    if COMPILER_PATH == '':
        COMPILER_PATH = os.getenv('OS_EXEC_PATH')
    PREFIX  = 'mips-sde-elf-'
    CC      = PREFIX + 'gcc'
    CXX     = PREFIX + 'g++'
    AS      = PREFIX + 'gcc'
    AR      = PREFIX + 'ar'
    LINK    = PREFIX + 'g++'
    RESULT_SUFFIX = 'elf'
    SIZE    = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY  = PREFIX + 'objcopy'
    STRIP   = PREFIX + 'strip'

    DEVICE = ' -mips32r2 -msoft-float -mfp32'
    CFLAGS  = DEVICE + ' -EL -G0 -mno-abicalls -fno-zero-initialized-in-bss -fno-pic -fno-builtin -fno-exceptions -ffunction-sections -fno-omit-frame-pointer'
    AFLAGS  = ' -c' + DEVICE + ' -EL -x assembler-with-cpp'
    LFLAGS  = DEVICE + ' -EL -Wl,--gc-sections,-Map=oneos.map,-cref,-u,Reset_Handler -T board/linker_scripts/x1000e_ram.lds'
    CPATH   = ''
    LPATH   = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -gdwarf-2'
        AFLAGS += ' -gdwarf-2'
    else:
        CFLAGS += ' -O2'

    CXXFLAGS = CFLAGS

    M_CFLAGS = DEVICE + ' -EL -G0 -O2 -mno-abicalls -fno-common -fno-exceptions -fno-omit-frame-pointer -mlong-calls -fno-pic '
    M_CXXFLAGS = M_CFLAGS
    M_LFLAGS = DEVICE + ' -EL -r -Wl,--gc-sections,-z,max-page-size=0x4' +\
                                    ' -nostartfiles -static-libgcc'
    M_POST_ACTION = STRIP + ' -R .hash $TARGET\n' + SIZE + ' $TARGET \n'

DUMP_ACTION = OBJDUMP + ' -D -S $TARGET > oneos.asm\n'
POST_ACTION = OBJCPY + ' -R .reserved_ram -O binary $TARGET oneos.bin\n' + SIZE + ' $TARGET \n'
