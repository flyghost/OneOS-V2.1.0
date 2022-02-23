import sys
import glob  
import os.path 
import re
from build_tools import *

def gen_cotex_m_link_file_gcc(prj_path, app_addr, app_size, ram_addr, ram_size):
    link_sct = prj_path + "/board/linker_scripts/link.lds"
    
    if not os.path.exists(link_sct):
        return
        
    sss = ''
    f1 = open(link_sct, 'r', newline = '')
    
    for ss in f1.readlines():
        if "ROM" in ss and "ORIGIN" in ss and "LENGTH" in ss:
            ss = "    ROM  (rx) : ORIGIN = %s, LENGTH = %s /* %s flash */\n" % (app_addr, app_size, app_size)
            
        if "RAM1" in ss and "ORIGIN" in ss and "LENGTH" in ss:
            ss = "    RAM1 (rx) : ORIGIN = %s, LENGTH = %s /* %s sram */\n" % (ram_addr, ram_size, ram_size)
            
        if "RAM" in ss and "ORIGIN" in ss and "LENGTH" in ss and ss.split()[0] == "RAM":
            ss = "    RAM (rx) : ORIGIN = %s, LENGTH = %s /* %s sram */\n" % (ram_addr, ram_size, ram_size)
    
        sss += ss
    
    f1.close()
    
    f1 = open(link_sct, 'w+', newline = '')
    f1.write(sss)
    f1.close()

def gen_cotex_m_link_file_keil(prj_path, app_addr, app_size, ram_addr, ram_size):
    link_sct = prj_path + "/board/linker_scripts/link.sct"
    
    if not os.path.exists(link_sct):
        return
        
    sss = ''
    f1 = open(link_sct, 'r', newline = '')
    
    for ss in f1.readlines():
        if "LR_IROM1" in ss:
            ss = "LR_IROM1 %s %s  {    ; load region size_region\n" % (app_addr, app_size)
            
        if "ER_IROM1" in ss:
            ss = "  ER_IROM1 %s %s  {  ; load address = execution address\n" % (app_addr, app_size)
            
        if "RW_IRAM1" in ss:
            ss = "  RW_IRAM1 %s %s  {  ; RW data\n" % (ram_addr, ram_size)
    
        sss += ss
    
    f1.close()
    
    f1 = open(link_sct, 'w+', newline = '')
    f1.write(sss)
    f1.close()

def gen_cotex_m_link_file_iar(prj_path, app_addr, app_size, ram_addr, ram_size):
    link_sct = prj_path + "/board/linker_scripts/link.icf"
    
    if not os.path.exists(link_sct):
        return
    
def gen_cotex_m_link_file(prj_path):
    oneos_config_h  = prj_path + "/oneos_config.h"
    
    if not os.path.exists(oneos_config_h):
        return
    
    app_addr = 0x12345678
    app_size = 0x12345678
    ram_addr = 0x12345678
    ram_size = 0x12345678
    
    with open(oneos_config_h, 'r+', newline = '') as f1:
        for ss in f1.readlines():
            if "#define BSP_TEXT_SECTION_ADDR " in ss:
                app_addr = ss.split()[2]
                
            if "#define BSP_TEXT_SECTION_SIZE " in ss:
                app_size = ss.split()[2]
                
            if "#define BSP_DATA_SECTION_ADDR " in ss:
                ram_addr = ss.split()[2]
                
            if "#define BSP_DATA_SECTION_SIZE " in ss:
                ram_size = ss.split()[2]
    
    if app_addr == 0x12345678 or app_size == 0x12345678\
    or ram_addr == 0x12345678 or ram_size == 0x12345678:
        return
    
    ram_addr = int(ram_addr, 16)
    ram_size = int(ram_size, 16)
    
    ram_addr += 0x10
    ram_size -= 0x10
    
    ram_addr = str(hex(ram_addr))
    ram_size = str(hex(ram_size))
    
    gen_cotex_m_link_file_gcc(prj_path, app_addr, app_size, ram_addr, ram_size)
    gen_cotex_m_link_file_keil(prj_path, app_addr, app_size, ram_addr, ram_size)
    gen_cotex_m_link_file_iar(prj_path, app_addr, app_size, ram_addr, ram_size)
