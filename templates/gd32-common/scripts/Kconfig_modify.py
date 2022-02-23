import os
import sys
import shutil
import flash_size_table
import sram_size_table

def startup_file_name_get(series, model, soc):
    if series == "GD32E10x":
        name = series.lower()
    elif series == "GD32E50x":
        if (model == "GD32E505") or (model == "GD32E507"):
            name = series.lower() +"_cl"
        elif model == "GD32E503" :
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 512:
                name = series.lower() +"_hd"
            else:
                name = series.lower() +"_xd"
    elif series == "GD32F30x":
        if (model == "GD32F305") or (model == "GD32F307"):
            name = series.lower() +"_cl"
        elif model == "GD32F303" :
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 512:
                name = series.lower() +"_hd"
            else:
                name = series.lower() +"_xd"
    elif series == "GD32F3x0":
        name = series.lower()
    elif series == "GD32F4xx":
        name = model.lower()
    elif series == "GD32F403":
        name = series.lower()
    elif series == "GD32F10x":
        if (model == "GD32F105") or (model == "GD32F107"):
            name = series.lower() +"_cl"
        elif (model == "GD32F101") or (model == "GD32F103"):
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 128:
                name = series.lower() +"_md"
            elif flash_size <= 512:
                name = series.lower() +"_hd"
            else:
                name = series.lower() +"_xd"
    elif series == "GD32F20x":
        name = series.lower() +"_cl"
    return name

def modify(series,model,soc):
    old_path = os.getcwd()
    source_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(source_path+'/../target')
    with open('Kconfig', 'r') as f:
        with open('Kconfig.new', 'w') as g:
            for line in f.readlines():
                if 'select SOC_' in line: 
                    g.write('    select SOC_' + startup_file_name_get(series, model, soc).upper() + '\n')
                else:
                    g.write(line)
    shutil.move('Kconfig.new', 'Kconfig')
    os.chdir(old_path)


