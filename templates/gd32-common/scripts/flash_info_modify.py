import os
import sys
import shutil
import flash_size_table
import sram_size_table
import sector_size_table

def modify(series,model,soc):
    old_path = os.getcwd()
    source_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(source_path+'/../target/board/ports/')
    bootloader_size = sector_size_table.get_partition_size(series, 'bootloader')
    cfg_size        = sector_size_table.get_partition_size(series, 'cfg')
    download_size   = sector_size_table.get_partition_size(series, 'download')
    bank0_size      = sector_size_table.get_bank0_size_array(series)
    flash_size      = flash_size_table.get_size_kb(soc)
    if bootloader_size == 0:
        if flash_size <= 64:
            bootloader_size = 16
        else:
            bootloader_size = 32
    with open('flash_info.c', 'r') as f:
        with open('flash_info.new', 'w') as g:
            for line in f.readlines():
                if '.capacity' in line:
                    g.write('    .capacity   = %d,\n' % (cfg_size * 1024))
                elif '.start_addr' in line:
                    if flash_size <= bank0_size:
                        app_size = flash_size - bootloader_size - cfg_size - bootloader_size
                    else:
                        app_size = bank0_size - bootloader_size - cfg_size - bootloader_size
                    g.write('    .start_addr = 0x%08x,\n' % (0x08000000 + bootloader_size * 1024))
                elif '.block_size' in line:
                    g.write('    .block_size = %d,\n' % (cfg_size * 1024))
                else:
                    g.write(line)
                    
    shutil.move('flash_info.new', 'flash_info.c')
    os.chdir(old_path)



