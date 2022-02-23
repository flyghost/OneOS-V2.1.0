import os
import re
import subprocess
import json
from functools import reduce

def path_join(*dir_file):
    return reduce(lambda x, y: os.path.join(x,y), dir_file)

def get_iotjs_path():
    return os.path.dirname(os.path.abspath(__file__))

def iter_require(enable_modules, modules, module):
    if "require" in modules[module].keys():
        for m in modules[module]["require"]:
            if m not in enable_modules:
                iter_require(enable_modules, modules, m)

    if "js_file" in modules[module].keys() \
        or "native_files" in modules[module].keys():
        enable_modules.add(module)

def get_enabled_modules(modules_config):
    enable_modules = set()
    iter_require(enable_modules, modules_config, "enable_modules")
    return enable_modules

def get_all_modules(modules_config):
    allm = set()
    for key in modules_config:
        if "js_file" in modules_config[key] \
            or "native_files" in modules_config[key]:
            allm.add(key)
    return allm

def update_module_define(enable_modules, disables_modules):
    define_path = path_join(get_iotjs_path(), 'src', 'iotjs_module_define.h')
    module_define = dict()
    for m in enable_modules:
        module_define[m] = 1
    for m in disables_modules:
        module_define[m] = 0
    try:
        os.remove(define_path)
    except(FileNotFoundError):
        pass
    with open(define_path, 'w+') as f:
        for key in sorted(module_define.keys()):
            d = "#define ENABLE_MODULE_{} {}\n".format(key.upper(), module_define[key])
            f.write(d)

def get_init_func(modules_config, enable_modules):
    func_set = dict()
    for m in enable_modules:
        if 'init' in modules_config[m].keys():
            func_set[m] = modules_config[m]['init']
    return func_set

def update_module_ini(modules_config, enable_modules):
    module_ini_path = path_join(get_iotjs_path(), 'src', 'iotjs_module_inl.h')
    __module_ini__ = """/* File generated */
{0}

const unsigned iotjs_module_count = {1};

const
iotjs_module_ro_data_t iotjs_module_ro_data[{1}] = {{
{2}
}};

iotjs_module_rw_data_t iotjs_module_rw_data[{1}] = {{
{3}
}};
"""
    func_set = get_init_func(modules_config, enable_modules)
    func_decl = "\n".join(
        ["extern jerry_value_t {}(void);".format(x) for x in func_set.values()]
    )
    ro_data = "\n".join(
        ["  {{ \"{}\", {} }},".format(k,v) for k, v in func_set.items()]
    )
    rw_data = "  { 0 },\n" * len(func_set)
    try:
        os.remove(module_ini_path)
    except(FileNotFoundError):
        pass
    with open(module_ini_path, "w+") as f:
        f.write(__module_ini__.format(func_decl, len(func_set), ro_data, rw_data))

def updata_iotjs(modules_config, enable_modules):
    iotjs_path = get_iotjs_path()
    snapshot_tool_path = path_join(iotjs_path, "tools", "jerry-snapshot.exe")
    js2cpy_path = path_join(iotjs_path, "tools", "js2c.py")

    modules_cmd = ['iotjs=' + path_join(iotjs_path, 'src', 'js', 'iotjs.js')]

    for m in enable_modules:
        if "js_file" in modules_config[m].keys():
            js_path = path_join(iotjs_path, 'src', modules_config[m]["js_file"])
            modules_cmd.append(m + '=' + js_path)

    cmd = ['python',
            js2cpy_path,
            '--buildtype=debug',
            '--snapshot-tool=' + snapshot_tool_path,
            '--modules',
            ','.join(modules_cmd)]

    try:
        os.remove(path_join(iotjs_path, 'src', 'iotjs_js.c'))
        os.remove(path_join(iotjs_path, 'src', 'iotjs_js.h'))
    except(FileNotFoundError):
        pass
    subprocess.call(cmd)

if __name__ == "__main__":
    modules_config = None
    with open(path_join(get_iotjs_path(), 'src', 'modules.json'), 'r') as fjson:
        modules_config = json.load(fjson)

    enable_modules = get_enabled_modules(modules_config["modules"])
    all_modules = get_all_modules(modules_config["modules"])
    update_module_define(enable_modules, all_modules - enable_modules)

    update_module_ini(modules_config["modules"], enable_modules)

    updata_iotjs(modules_config["modules"], enable_modules)
