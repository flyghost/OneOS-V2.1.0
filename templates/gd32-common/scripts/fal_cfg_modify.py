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
    with open('fal_cfg.c', 'r') as f:
        with open('fal_cfg.new', 'w') as g:
            for line in f.readlines():
                if '{ "cfg", "flash_0"' in line:
                    g.write('    { "cfg", "flash_0"  , 0, 0x%08x, FAL_PART_INFO_FLAGS_UNLOCKED},\n' % (cfg_size * 1024))
                else:
                    g.write(line);

                
    shutil.move('fal_cfg.new', 'fal_cfg.c')
    os.chdir(old_path)



