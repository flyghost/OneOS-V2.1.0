import os
import sys
import shutil
import importlib
import importlib.util
import time

def str2hex(s):
    #kb to b(1024)
    odata = "0x%08X" % (int(s,10)*1024)
    return odata   


def _ignore_copy_files(path, content):
        to_ignore = []
        for file_ in content:
            if file_ in ('board_config','peripheral','startup','generate.py', 'scripts'):
                to_ignore.append(file_)
        return to_ignore

def startup_path_get(series, model, soc):
    header = series.lower()
    if series == "GD32E10x":
        tail = ""
    elif series == "GD32F30x":
        if (model == "GD32F305") or (model == "GD32F307"):
            tail = series.lower() +"_cl"
        elif model == "GD32F303" :
            import flash_size_table
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 512:
                tail = series.lower() +"_hd"
            else:
                tail = series.lower() +"_xd"
    elif series == "GD32E50x":
        if (model == "GD32E505") or (model == "GD32E507"):
            tail = series.lower() +"_cl"
        elif model == "GD32E503" :
            import flash_size_table
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 512:
                tail = series.lower() +"_hd"
            else:
                tail = series.lower() +"_xd"
    elif series == "GD32F3x0":
        tail = ""
    elif series == "GD32F4xx":
        tail = model.lower()
    elif series == "GD32F403":
        tail = ""
    elif series == "GD32F10x":
        if (model == "GD32F105") or (model == "GD32F107"):
            tail = series.lower() +"_cl"
        elif (model == "GD32F101") or (model == "GD32F103"):
            import flash_size_table
            flash_size = flash_size_table.get_size_kb(soc)
            if flash_size <= 128:
                tail = series.lower() +"_md"
            elif flash_size <= 512:
                tail = series.lower() +"_hd"
            else:
                tail = series.lower() +"_xd"
    elif series == "GD32F20x":
        tail = series.lower() +"_cl"
    startup_path = header + "\\" + tail
    return startup_path

file_modify_table = {}

file_modify_table[0] = 'SConscript_modify.py'
file_modify_table[1] = 'board_h_modify.py'
file_modify_table[2] = 'app_startaddr_modify.py'
file_modify_table[3] = 'Kconfig_modify.py'
file_modify_table[4] = 'template_uvprojx_modify.py'
file_modify_table[5] = 'oneos_config_h_modify.py'
file_modify_table[6] = 'fal_cfg_modify.py'
file_modify_table[7] = 'link_sct_modify.py'
file_modify_table[8] = 'flash_info_modify.py'

def template(g):
    for item in g:
        if 'SERIES_' in item:  
            series = item
        elif 'MODEL_' in item:
            model = item
        elif 'TEMP_' in item:
            soc = item  
    gd_model = model.split("_")[1]
    gd_series = series.split("_")[1]
    gd_soc = soc.split("_")[1]
    print("gd_model: %s" % gd_model)
    print("gd_series: %s" % gd_series)
    print("gd_soc: %s" % gd_soc)

    print("gd_model: %s" % gd_model.lower())
    print("gd_series: %s" % gd_series.lower())
    print("gd_soc: %s" % gd_soc.lower())

    # copy project
    source_path = os.path.dirname(os.path.realpath(__file__))
    target_path = os.path.join(source_path, 'target')
    sys.path.append('../templates/gd32-common/scripts/')
    sys.path.append('../templates/gd32-common/')
    import scripts
    
    print(source_path)
    print(target_path)
    
    if not os.path.exists(target_path):
        os.makedirs(target_path)

    if os.path.exists(source_path):
        shutil.rmtree(target_path)
    
    shutil.copytree(source_path, target_path, ignore=_ignore_copy_files)

    # copy board_config
    # first,find path
    board_config_src_source_path = source_path + '\\board\\board_config\\' + gd_series.lower() + '\\Src'
    board_config_inc_source_path = source_path + '\\board\\board_config\\' + gd_series.lower() + '\\Inc'
    board_config_target_path = source_path + '\\target\\board' 
    # seocnd,copy files 
    for root, dirs, files in os.walk(board_config_src_source_path):
        for file in files:
            src_file = os.path.join(root, file)
            shutil.copy(src_file, board_config_target_path)
			
    for root, dirs, files in os.walk(board_config_inc_source_path):
        for file in files:
            src_file = os.path.join(root, file)
            shutil.copy(src_file, board_config_target_path)
	
    # copy peripheral
    peripheral_source_path = source_path + '\\board\\peripheral\\' + gd_series.lower()
    print(peripheral_source_path)
    peripheral_target_path = source_path + '\\target\\board\\peripheral' 
    shutil.copytree(peripheral_source_path, peripheral_target_path)

    #for root, dirs, files in os.walk(peripheral_source_path):
     #   for file in files:
    #        src_file = os.path.join(root, file)
      #      shutil.copy(src_file, peripheral_target_path)
    
    # copy startup
    source_startup_path = source_path + '\\board\\startup\\' + startup_path_get(gd_series, gd_model, gd_soc)
    target_startup_path = source_path + '\\target\\board\\startup\\'
    shutil.copytree(source_startup_path, target_startup_path)

    for item in file_modify_table:
        loader = importlib.machinery.SourceFileLoader('SConscript', "../templates/gd32-common/scripts/" + file_modify_table[item])
        spec = importlib.util.spec_from_loader(loader.name, loader)
        mod = importlib.util.module_from_spec(spec)
        loader.exec_module(mod)
        mod.modify(gd_series, gd_model, gd_soc)
        
    print("end generate.py")

    return target_path
    

    

