import os
import sys
import shutil
import flash_size_table
import sram_size_table
import sector_size_table

def modify(series,model,soc):
    old_path = os.getcwd()
    source_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(source_path+'/../target/board')
    flash_size      = flash_size_table.get_size_kb(soc)
    bootloader_size = sector_size_table.get_partition_size(series, 'bootloader')
    if (bootloader_size == 0):
        if flash_size > 64:
            bootloader_size = 32
        else:
            bootloader_size = 16
    cfg_size        = sector_size_table.get_partition_size(series, 'cfg')
    download_size   = sector_size_table.get_partition_size(series, 'download')
    bank0_size      = sector_size_table.get_bank0_size_array(series)
    addr_idx = ''
    with open('board.h', 'r') as f:
        with open('board.new', 'w') as g:
            for line in f.readlines():
                if 'GD32_FLASH_END_ADDRESS' in line:
                    g.write(line)
                elif 'GD32_SRAM1_END' in line:
                    g.write(line)
                elif 'GD32_SRAM2_END' in line:
                    g.write(line)
                elif '#include <gd32f' in line:
                    g.write('#include <%s.h>\n' % series.lower())
                elif '#define SOC_MODEL' in line:
                    g.write('#define SOC_MODEL  "%s"\n' % soc.upper())
                elif '#ifdef OS_USE_BOOTLOADER' in line:
                    addr_idx = 'bootloader'
                    g.write(line)
                elif '#else' in line:
                    if (addr_idx == 'bootloader'):
                        addr_idx = 'no_bootloader'
                    g.write(line)
                elif '#endif' in line:
                    if (addr_idx == 'bootloader'):
                        addr_idx = ''
                    g.write(line)
                elif '#define GD32_APP_ADDR' in line:
                    if addr_idx == 'bootloader':
                        app_addr = 0x08000000 + bootloader_size * 1024 + cfg_size * 1024
                    else:
                        app_addr = 0x08000000
                    g.write("#define GD32_APP_ADDR 0x%08x\n" % app_addr)
                elif '#define GD32_APP_SIZE' in line:
                    if addr_idx == 'bootloader':
                        if flash_size <= bank0_size:
                            app_size = flash_size - bootloader_size - cfg_size
                        else:
                            app_size = bank0_size - bootloader_size - cfg_size
                        app_size = app_size * 1024
                    else:
                        app_size = flash_size * 1024
                    g.write("#define GD32_APP_SIZE 0x%08x\n" % app_size)
                elif 'GD32_FLASH_START_ADRESS' in line: 
                    g.write('#define GD32_FLASH_START_ADRESS ((uint32_t)0x%s)\n' % flash_size_table.get_startaddr(soc))
                elif 'GD32_FLASH_SIZE' in line:
                    g.write('#define GD32_FLASH_SIZE         (%d * 1024)\n' % flash_size_table.get_size_kb(soc)) 
                elif 'GD32_SRAM1_START' in line: 
                    g.write('#define GD32_SRAM1_START (0x%s)\n' % sram_size_table.get_startaddr(soc, 'sram_1'))
                elif 'GD32_SRAM1_SIZE' in line: 
                    g.write('#define GD32_SRAM1_SIZE  (%d * 1024)\n' % sram_size_table.get_size_kb(soc, 'sram_1'))
                elif 'GD32_SRAM2_START' in line: 
                    g.write('#define GD32_SRAM2_START (0x%s)\n' % sram_size_table.get_startaddr(soc, 'sram_2'))
                elif 'GD32_SRAM2_SIZE' in line: 
                    g.write('#define GD32_SRAM2_SIZE  (%d * 1024)\n' % sram_size_table.get_size_kb(soc, 'sram_2'))
                else:
                    g.write(line)
    shutil.move('board.new', 'board.h')
    os.chdir(old_path)



