import os
import sys
import shutil
import importlib.machinery

from gen_config import init_config_by_kconfig


def searchfile(path, name):
    projects = []
    L = len(os.path.abspath('.')) + 1
    for item in os.listdir(path):
        if os.path.isdir(os.path.join(path, item)):
            projects += searchfile(os.path.join(path, item), name)
        else:
            if name in item:
                projects.append(path[L:])
                
    return projects


def buildone(proj):    
    cwd = os.getcwd()
    proj_dir = os.path.join(cwd, proj)
    os.chdir(proj_dir)
    
    os.system('rm .sconsign.dblite')

    init_config_by_kconfig()

    if os.path.exists('template.uvprojx'):
        os.system('scons --ide=mdk5')    
    
    os.chdir(cwd)
    
def refresh_target(path):
    projects = searchfile(os.path.abspath(path), 'osconfig.py')
    
    for proj in projects:
        buildone(proj)


def target():
    global g
    g = {}
    soc_type = None

    if sys.platform == "win32":
        os.system('menuconfig')
    else:
        menuconfig_path = os.path.join(os.path.dirname(__file__), "linux_menuconfig")
        sys.path.append(menuconfig_path)
        command = 'python ' + '"' + os.path.join(menuconfig_path, "menuconfig.py") + '"'
        os.system(command)

    with open("oneos_config.h", 'r') as f:
        for ss in f.readlines():
            if '#define TEMP_' in ss:
                soc_type = ss.split('_')[1].replace('\n','')
            if '#define ' in ss:
                g[ss.split()[1]] = 1
           
    if soc_type is None:
        print("unknown soc_type")
        return None

    print("soc_type: %s" % soc_type)
    target_path = './' + soc_type
    
    if os.path.exists(target_path):
        print("soc_type: %s already exist" % soc_type)
        return soc_type

    mod = importlib.machinery.SourceFileLoader('SConscript', "../templates/configs/SConscript").load_module()
    # spec = importlib.util.spec_from_loader(loader.name, loader)
    # mod = importlib.util.module_from_spec(spec)
    # loader.exec_module(mod)
    source_path = mod.soc_type_inq(g)
    if source_path is None or not os.path.exists(source_path):
        print("not support %s" % soc_type)
        return None
    
    try:
        shutil.copytree(source_path, target_path)
    except shutil.Error as exc:
        errors = exc.args[0]
        for error in errors:
            src, dst, msg = error
            if not os.path.exists(src):
                shutil.copytree(src, dst)
    shutil.rmtree(source_path)
    
    refresh_target(target_path)

    return soc_type
