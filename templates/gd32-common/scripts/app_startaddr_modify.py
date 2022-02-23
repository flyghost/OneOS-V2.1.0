import os
import sys
import shutil
import flash_size_table
import sram_size_table
import sector_size_table

def modify(series,model,soc):
    old_path = os.getcwd()
    source_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(source_path+'/../target')
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
    with open('.config', 'r') as f:
        with open('.config.new', 'w') as g:
            for line in f.readlines():
                if 'CONFIG_BSP_BOOT_OPTION=y' in line:
                    g.write(line)
                elif 'CONFIG_BSP_TEXT_SECTION_ADDR' in line:
                    app_addr = 0x08000000 + bootloader_size * 1024 + cfg_size * 1024
                    g.write("CONFIG_BSP_TEXT_SECTION_ADDR=0x%08x\n" % app_addr)
                elif 'CONFIG_BSP_TEXT_SECTION_SIZE' in line:
                    if flash_size <= bank0_size:
                        app_size = flash_size - bootloader_size - cfg_size
                    else:
                        app_size = bank0_size - bootloader_size - cfg_size
                    app_size = app_size * 1024
                    g.write("CONFIG_BSP_TEXT_SECTION_SIZE=0x%08x\n" % app_size)
                elif 'CONFIG_BSP_DATA_SECTION_ADDR' in line:
                    g.write('CONFIG_BSP_DATA_SECTION_ADDR=0x%s\n' % sram_size_table.get_startaddr(soc, 'sram_1'))
                elif 'CONFIG_BSP_DATA_SECTION_SIZE' in line:
                    sram1_size = int(sram_size_table.get_size_kb(soc, 'sram_1'))
                    sram1_size = sram1_size * 1024
                    g.write('CONFIG_BSP_DATA_SECTION_SIZE=0x%08x\n' % sram1_size)
                else:
                    g.write(line)
                    
    shutil.move('.config.new', '.config')
    os.chdir(old_path)