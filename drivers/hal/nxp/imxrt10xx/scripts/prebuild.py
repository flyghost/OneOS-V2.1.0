import sys
import glob  
import os.path 
import re
from build_tools import *

datas = \
{
    # default='_config'
    
    'LPI2C'    : '_masterConfig',
    'SAI'      : '_Tx_config,_Rx_config',   
}

def gen_nxp_device_data(device_type, device):
    data = device + datas.get(device_type, '_config')       # default='_config'
    if data != 'OS_NULL':
        data = '&' + data
        
    if ',' in data:
        data = data.split(',')
        data = data[0] + ', &' + device + data[1]
        
    return data

def gen_nxp_devices_file(prj_path, bsp_path):
    source = prj_path + "/" + bsp_path + "/peripherals.h"
    target = prj_path + "/board/devices.c"
    config_files = prj_path + "/board/board_config.h"
    
    config_list = []
    config_line = ''
    
    f1 = open(source, 'r+', newline = '')
    f2 = open(target, 'w+', newline = '')
    f3 = open(config_files, 'w+', newline = '')
    
    
    AddDefined('BSP_USING_GPIO')
    config_line = '#define BSP_USING_GPIO\n'
    if config_line not in config_list:
        config_list.append(config_line)

    for ss in f1.readlines(): 
        dma_device = re.compile('define (.*)_DMA_BASEADDR ').findall(ss)
        if len(dma_device) != 0:
            AddDefined("BSP_USING_DMA")
            config_line = '#define BSP_USING_DMA\n'
            if config_line not in config_list:
                config_list.append(config_line)
            continue
      
        device = re.compile('define (.*)_PERIPHERAL ').findall(ss)
        if len(device) == 0:
            continue
            
        device = device[0]                                      # LPI2C1
        #print("device: " + str(device))
        
        device_type = re.findall(r'[A-Z].*[A-Z]', device)[0]    # LPI2C
        #print("device_type: " + str(device_type))
        
        device_index = device[len(device_type):]                # 1
        #print("device_index: " + str(device_index))

        key = "BSP_USING_" + device_type                        # BSP_USING_LPI2C
        #print(key)
        AddDefined(key)
        config_line = '#define ' + key + '\n'
        if config_line not in config_list:
            config_list.append(config_line)
            
        data = gen_nxp_device_data(device_type, device)

        f2.write('static const struct nxp_' + device_type.lower() + '_info ' + device.lower() + '_info = {' + device + '_PERIPHERAL, ' + data + '};\n')
        f2.write('OS_HAL_DEVICE_DEFINE("' + device_type + '_Type", "' + device.lower() + '", ' + device.lower() + '_info);\n\n')
    
    f3.writelines(config_list)
    
    f1.close()
    f2.close()
    f3.close()
    
    
def prebuild(prj_path, bsp_path = '/board/board/board/'):
    print("project " + prj_path)
    gen_nxp_devices_file(prj_path, bsp_path)
    