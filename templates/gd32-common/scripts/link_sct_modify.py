import os
import sys
import shutil
import flash_size_table
import sram_size_table
import sector_size_table
import arch_table

def str2hex(s):
    #kb to b(1024)
    odata = "0x%08X" % (int(s,10)*1024)
    return odata

#替换sct LR_IROM1 ER_IROM1 RW_IRAM1
def modify(series, model, soc):
    old_path = os.getcwd()
    source_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(source_path+'/../target/board/linker_scripts')
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
    
    if flash_size <= bank0_size:
        app_size = flash_size - bootloader_size - cfg_size
    else:
        app_size = bank0_size - bootloader_size - cfg_size
    app_size = app_size * 1024
    app_addr = 0x08000000 + bootloader_size * 1024 + cfg_size * 1024
    with open('link.sct', 'r') as f:
        with open('link.new', 'w') as g:
            for line in f.readlines():
                if '#! armcc -E -I' in line:
                    soc_arch = arch_table.get_arch(series)
                    if (soc_arch == 'M33'):
                        g.write("#! armclang -E -I.\ --target=arm-arm-none-eabi -mcpu=cortex-m33 -xc\n")
                    else:
                        g.write("#! armcc -E -I.\ --cpu Cortex-%s\n" % soc_arch)
                elif 'LR_IROM1' in line:
                    g.write("LR_IROM1  0x%08x   0x%08x {    ; load region size_region\n" % (app_addr, app_size))
                elif 'ER_IROM1' in line:
                    g.write("  ER_IROM1  0x%08x   0x%08x {  ; load address = execution address\n" % (app_addr, app_size))
                elif 'FLASH_SIZE_XXXX' in line:
                    g.write("    #define CODE_AREA_LENGTH 0x%x\n" % (flash_size * 1024))
                elif 'RW_IRAM1' in line:
                    g.write("  RW_IRAM1 0x20000000 0x%08x {  ; RW data\n" % sram_size_table.get_size_byte(soc, 'sram_1'))
                else:
                    g.write(line)
    shutil.move('link.new', 'link.sct')
    os.chdir(old_path)


