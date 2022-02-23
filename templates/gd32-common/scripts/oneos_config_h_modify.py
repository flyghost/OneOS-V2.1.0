import os
import sys
import shutil
import flash_size_table


def get_series_with_capital_tail_text(series, model, soc):
    if series == "GD32E10x":
        name = series.upper()
    elif series == "GD32E50x":
        if (model == "GD32E505") or (model == "GD32E507"):
            name = series.upper() +"_CL"
        elif model == "GD32E503" :
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 512:
                name = series.upper() +"_HD"
            else:
                name = series.upper() +"_XD"
    elif series == "GD32F30x":
        if (model == "GD32F305") or (model == "GD32F307"):
            name = series.upper() +"_CL"
        elif model == "GD32F303" :
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 512:
                name = series.upper() +"_HD"
            else:
                name = series.upper() +"_XD"
    elif series == "GD32F3x0":
        name = model.upper()
    elif series == "GD32F4xx":
        name = model.upper()
    elif series == "GD32F403":
        name = model.upper()
    elif series == "GD32F10x":
        if (model == "GD32F105") or (model == "GD32F107"):
            name = series.upper() +"_CL"
        elif (model == "GD32F101") or (model == "GD32F103"):
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 128:
                name = series.upper() +"_MD"
            elif flash_size <= 512:
                name = series.upper() +"_HD"
            else:
                name = series.upper() +"_XD"
    elif series == "GD32F20x":
        name = series.upper() +"_CL"
            
    return name


def modify(series,model,soc):
    old_path = os.getcwd()
    source_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(source_path+'/../target')
    with open('oneos_config.h', 'r') as f:
        with open('oneos_config.new', 'w') as g:
            for line in f.readlines():
                if 'MANUFACTOR_xxxxxxxx' in line: 
                    g.write(line.replace("xxxxxxxx", series.upper(), 8))
                elif 'SERIES_xxxxxxxx' in line:
                    g.write(line.replace("xxxxxxxx", series.upper(), 8))
                elif 'SOC_xxxxxxxx' in line: 
                    g.write(line.replace("xxxxxxxx", get_series_with_capital_tail_text(series, model, soc), 11))
                else:
                    g.write(line)
    shutil.move('oneos_config.new', 'oneos_config.h')



    os.chdir(old_path)

