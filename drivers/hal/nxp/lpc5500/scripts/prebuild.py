import sys
import glob  
import os.path 
import re
from build_tools import *

import importlib
import importlib.util

datas = \
{
    # default='_config'
    
    'RTC'           : 'OS_NULL',
    'UTICK'         : 'OS_NULL',
}

def gen_nxp_device_data(device_type, device):
    data = datas.get(device_type, '_config')       # default='_config'
    if data == 'OS_NULL':
        return data
    else:
        return '&' + device + data

def gen_nxp_devices_file(prj_path, bsp_path):
    source = prj_path + "/" + bsp_path + "/peripherals.h"
    peripheral = prj_path + "/" + bsp_path + "/peripherals.c"
    target = prj_path + "/board/devices.c"
    
    f1 = open(source, 'r+', newline = '')
    f2 = open(peripheral, 'r+', newline = '')
    f3 = open(target, 'w+', newline = '')
    
    AddDefined('BSP_USING_GPIO')

    for ss in f2.readlines():
        device = re.compile('void USB(.*)_init').findall(ss)
        if len(device) > 0:
            AddDefined('BSP_USING_USB')
            f3.write('static const struct lpc_usb_info usb_info = {};\nOS_HAL_DEVICE_DEFINE("USB_Type", "usb", usb_info);\n\n')
            continue
            
    for ss in f1.readlines():            
        device = re.compile('define (.*)_PERIPHERAL ').findall(ss)
        if len(device) == 0:
            continue
            
        device = device[0]                                      # I2C1
        #print("device: " + str(device))
        
        device_type = re.findall(r'[A-Z].*[A-Z]', device)[0]    # I2C
        #print("device_type: " + str(device_type))
        
        device_index = device[len(device_type):]                # 1
        #print("device_index: " + str(device_index))

        key = "BSP_USING_" + device_type                        # BSP_USING_LPI2C
        #print(key)
        AddDefined(key)
        
        data = gen_nxp_device_data(device_type, device)

        f3.write('static const struct lpc_' + device_type.lower() + '_info ' + device.lower() + '_info = {' + device + '_PERIPHERAL, ' + data + '};\n')
        f3.write('OS_HAL_DEVICE_DEFINE("' + device_type + '_Type", "' + device.lower() + '", ' + device.lower() + '_info);\n\n')
        
    f1.close()
    f3.close()

def prebuild(prj_path, bsp_path = '/board/board/board/'):
    print("project " + prj_path)
    gen_nxp_devices_file(prj_path, bsp_path)
    
    loader = importlib.machinery.SourceFileLoader('prebuild.py', Env['OS_ROOT'] + '/drivers/boot/cotex-m/prebuild.py')
    spec   = importlib.util.spec_from_loader(loader.name, loader)
    mod    = importlib.util.module_from_spec(spec)
    loader.exec_module(mod)
    mod.gen_cotex_m_link_file(prj_path)