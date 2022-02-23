import os
import sys
import shutil
import flash_size_table
import sram_size_table
import arch_table


startaddr_func_table = {}
startaddr_func_table['IROM'] = 'get_IROM_startaddr_text'
startaddr_func_table['IRAM'] = 'get_sram_1_startaddr_text'
startaddr_func_table['OCR_RVCT4'] = 'get_IROM_startaddr_text'
startaddr_func_table['OCR_RVCT9'] = 'get_sram_1_startaddr_text'
startaddr_func_table['OCR_RVCT10'] = 'get_sram_2_startaddr_text'

size_func_table = {}
size_func_table['IROM'] = 'get_IROM_size'
size_func_table['IRAM'] = 'get_sram_1_size'
size_func_table['OCR_RVCT4'] = 'get_IROM_size'
size_func_table['OCR_RVCT9'] = 'get_sram_1_size'
size_func_table['OCR_RVCT10'] = 'get_sram_2_size'


def get_RegisterFile_text(series, soc):
    if series == 'GD32F4XX':
        ret = '          <RegisterFile>$$Device:'+ soc.upper()+'$Device\\Include\\'+ series.lower() + '.h</RegisterFile>\n'
    elif series == 'GD32F403':
        ret = '          <RegisterFile>$$Device:'+ soc.upper()+'$Device\\Include\\'+ series.lower() + '.h</RegisterFile>\n'
    else:
        ret = '          <RegisterFile>$$Device:'+ soc.upper()+'$Device\\Include\\'+ series.lower() + '.h</RegisterFile>\n'
    return ret

def get_FLM_file_name(series, model, soc):
    if series == "GD32E10x":
        ret = series
    elif series == "GD32E50x":
        ret = "%s_%d" % (series, flash_size_table.get_size_kb(soc))
    elif series == "GD32F30x":
        if (model == "GD32F305") or (model == "GD32F307"):
            ret = series +"_CL"
        elif model == "GD32F303" :
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 512:
                ret = series +"_HD"
            else:
                ret = series +"_XD"
    elif series == "GD32F3x0":
        ret = series
    elif series == "GD32F4xx":
        ret = series
    elif series == "GD32F403":
        ret = series
    elif series == "GD32F10x":
        if (model == "GD32F105") or (model == "GD32F107"):
            ret = series +"_CL"
        elif (model == "GD32F101") or (model == "GD32F103"):
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 128:
                ret = series +"_MD"
            elif flash_size <= 512:
                ret = series +"_HD"
            else:
                ret = series +"_XD"
    elif series == "GD32F20x":
        ret = series +"_CL"
    return ret

def get_Dll_name_tail(series):
    arch = arch_table.get_arch(series)
    dll_name = None
    if arch == 'M3':
        dll_name = 'CM3'
    elif arch == 'M4':
        dll_name = 'CM3'
    elif arch == 'M33':
        dll_name = 'V8M'
    return dll_name


def get_FlashDriverDll_text(series, model, soc):
    header = '          <FlashDriverDll>UL2%s(-S0 -C0 -P0 -FD20000000 -FC1000 -FN1 -FF0' % get_Dll_name_tail(series)
    if series == 'GD32E50x':
        ret = '%s -FS08000000 -FL%06x -FP0($$Device:%s$Flash\\%s.FLM))</FlashDriverDll>\n' \
                %(get_FLM_file_name(series, model, soc), flash_size_table.get_size_byte(soc), soc.upper(), get_FLM_file_name(series, model, soc))
    else:
        ret =  '%s -FS08000000 -FL%06x -FP0($$Device:%s$Flash\\%s.FLM))</FlashDriverDll>\n' \
                % (series, flash_size_table.get_size_byte(soc), soc.upper(), get_FLM_file_name(series, model, soc))
    ret = header + ret
    return ret

def get_svd_file_name(series, model, soc):
    if series == "GD32E10x":
        ret = series
    elif series == "GD32E50x":
        if (model == "GD32E505") or (model == "GD32E507"):
            ret = series +"_CL"
        elif model == "GD32E503" :
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 512:
                ret = series +"_HD"
            else:
                ret = series +"_XD"
    elif series == "GD32F30x":
        if (model == "GD32F305") or (model == "GD32F307"):
            ret = series +"_CL"
        elif model == "GD32F303" :
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 512:
                ret = series +"_HD"
            else:
                ret = series +"_XD"
    elif series == "GD32F3x0":
        ret = series
    elif series == "GD32F4xx":
        ret = series
    elif series == "GD32F403":
        ret = series
    elif series == "GD32F10x":
        if (model == "GD32F105") or (model == "GD32F107"):
            ret = series +"_CL"
        elif (model == "GD32F101") or (model == "GD32F103"):
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 128:
                ret = series +"_MD"
            elif flash_size <= 512:
                ret = series +"_HD"
            else:
                ret = series +"_XD"
    elif series == "GD32F20x":
        ret = series +"_CL"

    return ret

def get_SFDFile_text(series, model, soc):
    if series == 'GD32F10x':
        ret = '          <SFDFile>$$Device:'+ soc.upper()+'$SVD\\'+ 'GD32F10x\\' + get_svd_file_name(series, model, soc) + '.svd</SFDFile>\n'
    else:
        ret = '          <SFDFile>$$Device:'+ soc.upper()+'$SVD\\'+ get_svd_file_name(series, model, soc) + '.svd</SFDFile>\n'
    return ret

def get_Cpu_text(series, soc):
    if series == 'GD32E50x':
        ret = '          <Cpu>IRAM(%s,0x%x) IROM(%s,0x%06x) CPUTYPE("Cortex-%s") FPU3(SFPU) DSP CLOCK(12000000) ELITTLE</Cpu>\n' \
            % (get_IRAM_startaddr_text(soc), get_IRAM_size(soc), get_IROM_startaddr_text(soc), get_IROM_size(soc), arch_table.get_arch(series))
    elif (series == 'GD32F10x') or (series == 'GD32F20x'):
        ret = '          <Cpu>IRAM(%s,0x%x) IROM(%s,0x%06x) CPUTYPE("Cortex-%s") CLOCK(12000000) ELITTLE</Cpu>\n' \
            % (get_IRAM_startaddr_text(soc), get_IRAM_size(soc), get_IROM_startaddr_text(soc), get_IROM_size(soc), arch_table.get_arch(series))
    else:
        ret = '          <Cpu>IRAM(%s,0x%x) IROM(%s,0x%06x) CPUTYPE("Cortex-%s") FPU2 CLOCK(12000000) ELITTLE</Cpu>\n' \
            % (get_IRAM_startaddr_text(soc), get_IRAM_size(soc), get_IROM_startaddr_text(soc), get_IROM_size(soc), arch_table.get_arch(series))
    return ret

def get_hadIRAM2_text(soc):
    sram = sram_size_table.get_size_byte(soc, 'sram_2')
    if sram != None:
        if sram == 0:
            return '0'
        else:
            return '1'
    else:
        return ''

def get_RwSelD_text(soc):
    sram = sram_size_table.get_size_byte(soc, 'sram_2')
    if sram != None:
        if sram == 0:
            return '3'
        else:
            return '4'
    else:
        return ''

def get_StartAddress_text(idx,soc):
    if idx in startaddr_func_table:
        return eval(startaddr_func_table[idx])(soc)
    else:
        return ''

def get_Size_text(idx, soc):
    if idx in size_func_table:
        return eval(size_func_table[idx])(soc)
    else:
        return ''

def get_IROM_startaddr_text(soc):
    return '0x08000000'

def get_IROM_size(soc):
    rom_size = flash_size_table.get_size_byte(soc)
    if rom_size != None:
        return rom_size
    else:
        return 0

def get_decimal_text(number):
    if number != None:
        return '{:d}'.format(number)
    else:
        return '0'

def get_IRAM_startaddr_text(soc):
    return get_sram_1_startaddr_text(soc)
    
def get_IRAM_size(soc):
    return get_sram_1_size(soc)

def get_sram_1_startaddr_text(soc):
    return '0x20000000'

def get_sram_1_size(soc):
    sram = sram_size_table.get_size_byte(soc, 'sram_1')
    if sram != None:
        return sram
    else:
        return 0
    
def get_sram_2_startaddr_text(soc):
    sram = sram_size_table.get_size_byte(soc, 'sram_2')
    if sram != None:
        if sram != 0:
            return '0x10000000'
        else:
            return '0x0'
    else:
        return ''

def get_sram_2_size(soc):
    sram = sram_size_table.get_size_byte(soc, 'sram_2')
    if sram != None:
        if sram != 0:
            return sram
        else:
            return 0
    else:
        return ''

def get_component_text():
    text = "    <components>\n\
      <component Cclass=\"CMSIS\" Cgroup=\"CORE\" Cvendor=\"ARM\" Cversion=\"5.4.0\" condition=\"ARMv6_7_8-M Device\">\n\
        <package name=\"CMSIS\" schemaVersion=\"1.3\" url=\"http://www.keil.com/pack/\" vendor=\"ARM\" version=\"5.7.0\"/>\n\
        <targetInfos>\n\
          <targetInfo name=\"oneos\"/>\n\
        </targetInfos>\n\
      </component>\n\
    </components>\n"
    return text

def get_Groups_text():
    text = '        <Group>\n\
          <GroupName>::CMSIS</GroupName>\n\
        </Group>\n\
      </Groups>\n'
    return text

def get_v6Lang_text(series):
    arch = arch_table.get_arch(series)
    if arch == 'M33':
        ret = '3'
    elif arch == 'M4':
        ret = '1'
    elif arch == 'M3':
        ret = '1'
    return ret

def get_uAC6_text(series):
    arch = arch_table.get_arch(series)
    if arch == 'M33':
        ret = '1'
    elif arch == 'M4':
        ret = '0'
    elif arch == 'M3':
        ret = '0'
    return ret

def modify(series, model, soc):
    arch = arch_table.get_arch(series)
    old_path = os.getcwd()
    source_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(source_path+'/../target')
    ram_rom_idx = ''
    with open('template.uvprojx', 'r') as f:
        with open('template.new', 'w') as g:
            for line in f.readlines():
                if '<Device>' in line: 
                    g.write('          <Device>' + soc.upper() + '</Device>\n')
                elif '<PackID>' in line: 
                    if series == "GD32F10x":
                        g.write('          <PackID>GigaDevice.' + series + '_DFP.2.0.2</PackID>\n')
                    if series == "GD32F20x":
                        g.write('          <PackID>GigaDevice.' + series + '_DFP.2.1.0</PackID>\n')
                    else:
                        g.write('          <PackID>GigaDevice.' + series + '_DFP.2.0.0</PackID>\n')
                elif '<Cpu>' in line: 
                    # g.write('          <Cpu>GigaDevice.' + series + '_DFP.2.0.0</Cpu>\n')
                    g.write(get_Cpu_text(series, soc))
                elif '<FlashDriverDll>' in line:
                    g.write(get_FlashDriverDll_text(series, model, soc))
                elif '<RegisterFile>' in line: 
                    g.write(get_RegisterFile_text(series, soc))
                elif '<RwSelD>' in line:
                    g.write('            <RwSelD>' + get_RwSelD_text(soc) + '</RwSelD>\n')
                elif '<SFDFile>' in line:
                    g.write(get_SFDFile_text(series, model, soc))
                elif '<hadIRAM2>' in line:
                    g.write('            <hadIRAM2>' + get_hadIRAM2_text(soc) + '</hadIRAM2>\n')
                elif '<IRAM>' in line:
                    ram_rom_idx = 'IRAM'
                    g.write(line)
                elif '</IRAM>' in line:
                    ram_rom_idx = ''
                    g.write(line)
                elif '<IROM>' in line:
                    ram_rom_idx = 'IROM'
                    g.write(line)
                elif '</IROM>' in line:
                    ram_rom_idx = ''
                    g.write(line)
                elif '<OCR_RVCT4>' in line:
                    ram_rom_idx = 'OCR_RVCT4'
                    g.write(line)
                elif '</OCR_RVCT4>' in line:
                    ram_rom_idx = ''
                    g.write(line)
                elif '<OCR_RVCT9>' in line:
                    ram_rom_idx = 'OCR_RVCT9'
                    g.write(line)
                elif '</OCR_RVCT9>' in line:
                    ram_rom_idx = ''
                    g.write(line)
                elif '<OCR_RVCT10>' in line:
                    ram_rom_idx = 'OCR_RVCT10'
                    g.write(line)
                elif '</OCR_RVCT10>' in line:
                    ram_rom_idx = ''
                    g.write(line)
                elif '<StartAddress>' in line:
                    startaddr_text = get_StartAddress_text(ram_rom_idx, soc)
                    if startaddr_text != '' : 
                        g.write('                <StartAddress>%s</StartAddress>\n' % startaddr_text)
                    else:
                        g.write(line)
                elif '<Size>' in line:
                    Size_text = get_Size_text(ram_rom_idx, soc)
                    if Size_text != '' : 
                        g.write('                <Size>0x%x</Size>\n' % Size_text)
                    else:
                        g.write(line)
                elif '<components/>' in line:
                    if series == 'GD32E50x':
                        g.write(get_component_text())
                    else:
                        g.write(line)
                elif '<v6Lang>' in line:
                    g.write("            <v6Lang>%s</v6Lang>\n" % get_v6Lang_text(series))
                elif '<uAC6>' in line:
                    g.write("      <uAC6>%s</uAC6>\n" % get_uAC6_text(series))
                elif 'TargetDllName' in line:
                    g.write("          <TargetDllName>SARM%s.DLL</TargetDllName>\n" % get_Dll_name_tail(series))
                elif 'TargetDlgDllArguments' in line:
                    g.write("          <TargetDlgDllArguments>-pC%s</TargetDlgDllArguments>\n" % arch_table.get_arch(series))
                elif '</Groups>' in line:
                    if series == 'GD32E50x':
                        g.write(get_Groups_text())
                    else:
                        g.write(line)
                elif 'oneos.bin' in line:
                    g.write(line.replace("oneos", soc, 10))
                elif '<SimDlgDllArguments>' in line:
                    if arch == 'M3':
                        g.write('          <SimDlgDllArguments>' + '-pCM3' + '</SimDlgDllArguments>\n')
                    elif arch == 'M4':
                        g.write('          <SimDlgDllArguments>' + '-pCM4' + '</SimDlgDllArguments>\n')
                    elif arch == 'M33':
                        g.write('          <SimDlgDllArguments>' + '-pCM33' + '</SimDlgDllArguments>\n')
                elif '<AdsCpuType>' in line:
                    if arch == 'M3':
                        g.write('            <AdsCpuType>' + '"Cortex-M3"' + '</AdsCpuType>\n')
                    elif arch == 'M4':
                        g.write('            <AdsCpuType>' + '"Cortex-M4"' + '</AdsCpuType>\n')
                    elif arch == 'M33':
                        g.write('            <AdsCpuType>' + '"Cortex-M33"' + '</AdsCpuType>\n')
                elif '<Optim>' in line:
                    g.write('            <Optim>' + '3' + '</Optim>\n')
                else:
                    g.write(line)
    shutil.move('template.new', 'template.uvprojx')
    os.chdir(old_path)

