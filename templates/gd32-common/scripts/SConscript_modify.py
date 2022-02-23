import os
import sys
import shutil
import flash_size_table

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


def get_CPPDEFINES_text(series, model, soc):
    if series == "GD32E10x":
        name = series.upper()
    elif series == "GD32E50x":
        if (model == "GD32E505") or (model == "GD32E507"):
            name = series +"_cl"
        elif model == "GD32E503" :
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 512:
                name = series +"_hd"
            else:
                name = series +"_xd"
        name = name.upper() + ", " + series.upper()
    elif series == "GD32F30x":
        if (model == "GD32F305") or (model == "GD32F307"):
            name = series +"_cl"
        elif model == "GD32F303" :
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 512:
                name = series +"_hd"
            else:
                name = series +"_xd"
        name = name.upper()
    elif series == "GD32F3x0":
        name = model.upper()
    elif series == "GD32F4xx":
        name = model.upper()
    elif series == "GD32F403":
        name = model.upper()
    elif series == "GD32F10x":
        if (model == "GD32F105") or (model == "GD32F107"):
            name = series +"_cl"
        elif (model == "GD32F101") or (model == "GD32F103"):
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 128:
                name = series +"_md"
            elif flash_size <= 512:
                name = series +"_hd"
            else:
                name = series +"_xd"
        name = name.upper()
    elif series == "GD32F20x":
        name = series +"_cl"
        name = name.upper()
    return name


def modify(series,model,soc):
    old_path = os.getcwd()
    source_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(source_path+'/../target/board')
    with open('SConscript', 'r') as f:
        with open('SConscript.new', 'w') as g:
            for line in f.readlines():
                if '_it.c' in line: 
                    g.write('%s_it.c\n' % series.lower())
                elif 'system_' in line:
                    g.write('system_%s.c\n' % series.lower()) 
                elif '_gcc.s' in line: 
                    g.write('    src += [pwd + \'/startup/startup_%s_gcc.s\']\n' % startup_file_name_get(series, model, soc))
                elif '_arm.s' in line: 
                    g.write('    src += [pwd + \'/startup/startup_%s_arm.s\']\n' % startup_file_name_get(series, model, soc))
                elif '_iar.s' in line: 
                    g.write('    src += [pwd + \'/startup/startup_%s_iar.s\']\n' % startup_file_name_get(series, model, soc))
                elif 'CPPDEFINES = [' in line:
                    g.write('CPPDEFINES = [\'%s\']\n' % get_CPPDEFINES_text(series, model, soc).upper())
                elif 'path +=' in line:
                    g.write(line.replace("GD32F30x", series, 3))
                else:
                    g.write(line)
    shutil.move('SConscript.new', 'SConscript')

    # with open('SConscript', 'r') as test_file:
    #     test=test_file.read()
    #     print(test)

    os.chdir(old_path)




