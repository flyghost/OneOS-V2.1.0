import sys
import glob  
import os.path 
import re
from build_tools import *

import importlib
import importlib.util

def prebuild(prj_path):
    print("project " + prj_path)
    
    loader = importlib.machinery.SourceFileLoader('prebuild.py', Env['OS_ROOT'] + '/drivers/boot/cotex-m/prebuild.py')
    spec   = importlib.util.spec_from_loader(loader.name, loader)
    mod    = importlib.util.module_from_spec(spec)
    loader.exec_module(mod)
    mod.gen_cotex_m_link_file(prj_path)
    
