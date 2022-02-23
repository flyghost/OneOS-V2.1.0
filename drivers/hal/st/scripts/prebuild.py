import sys
import glob  
import os.path 
import re
from build_tools import *

import importlib
import importlib.util

def gen_stm32_bsp_file(prj_path, bsp_path):
    source = prj_path + "/" + bsp_path + "/main.c"
    target = prj_path + "/" + bsp_path + "/bsp.c"
     
    f1 = open(source, 'r+', newline = '')
    f2 = open(target, 'w+', newline = '')

    defined_sdram = False
    for ss in f1.readlines():
        if ss.find("SDRAM_HandleTypeDef", 0) != -1:
            defined_sdram = True
            
        ss = ss.replace("#include \"main.h\"", 
                        "#include \"main.h\"\n"
                      + "#include <oneos_config.h>")
            
        ss = ss.replace("int main(void)", "int hardware_init(void)")
        ss = ss.replace("/* USER CODE END WHILE */", "/* USER CODE END WHILE */\n  return 0;")
        ss = ss.replace("if (HAL_ETH_Init", "if (0 && HAL_ETH_Init")
        if defined_sdram:
            ss = ss.replace("  MX_FMC_Init();", 
                            "  MX_FMC_Init();\n"
                          + "  void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram);\n"
                          + "  SDRAM_Initialization_Sequence(&hsdram1);")
        
        ss = ss.replace("  SystemClock_Config();", 
                        "#ifndef DEFAULT_SYSTEM_CLOCK_CONFIG\n"
                      + "  SystemClock_Config();\n"
                      + "#endif")
    
        ss = ss.replace("MX_OPENAMP_Init(RPMSG_REMOTE, NULL);", "//MX_OPENAMP_Init(RPMSG_REMOTE, NULL);")
    
        f2.write(ss)
        
    f1.close()
    f2.close()

def gen_stm32_it_file(prj_path, bsp_path):
    source = glob.glob(prj_path + "/" + bsp_path + "stm32*_it.c")[0]
    target = source.split('_it.c')[0] + "_it_bsp.c"
    
    file = open(source, 'r+', newline = '')
    
    target_ss = ''

    lpuart1_fix_status = 0
    for ss in file.readlines():
        if "USER CODE END LPUART1_IRQn 0" in ss:
            lpuart1_fix_status = 1
    
        if "HAL_UART_IRQHandler(&huart1);" in ss and lpuart1_fix_status == 1:
            lpuart1_fix_status = 2
            ss = ss.replace("huart1", "hlpuart1")
            
        if "USER CODE BEGIN LPUART1_IRQn 1" in ss:
            lpuart1_fix_status = 3
            
        if IsDefined(['OS_USING_SERIAL', 'HAL_UART_MODULE_ENABLED']):
            ss = ss.replace('/* USER CODE END EV */', \
                            '/* USER CODE END EV */\n'\
                            '#ifdef HAL_UART_MODULE_ENABLED\n'\
                            'int HAL_USART_IDLE_PROCESS(UART_HandleTypeDef *huart);\n'\
                            '#endif')
            
            if 'HAL_UART_IRQHandler(' in ss:
                huart = ss.split('&')[1].split(')')[0]
                ss = '  if (HAL_USART_IDLE_PROCESS(&%s))\n'\
                     '    return;\n'\
                     '  \n'\
                     '  HAL_UART_IRQHandler(&%s);\n' % (huart, huart)
    
        ss = ss.replace("void SDMMC1_IRQHandler(void)", "void SDMMC1_IRQHandler_remove(void)")
        ss = ss.replace("void SDMMC2_IRQHandler(void)", "void SDMMC2_IRQHandler_remove(void)")
        ss = ss.replace("void SDIO_IRQHandler(void)", "void SDIO_IRQHandler_remove(void)")
        
        target_ss += ss
        
    file.close()
        
    file = open(target, 'w+', newline = '')
    file.write(target_ss)
    file.close()
    
def gen_stm32_devices_file(prj_path, bsp_path):
    for name in glob.glob(prj_path + "/" + bsp_path + '*msp.c'):
        name = os.path.basename(name)
        print(name)
    
    source = prj_path + "/" + bsp_path + "/main.c"
    target = prj_path + "/board/peripherals.c"
    msp = prj_path + "/" + bsp_path + "/" + name
    
    f1 = open(source, 'r+', newline = '')
    f2 = open(target, 'w+', newline = '')
    f3 = open(msp, 'r+', newline = '')
    
    device_type_list = [
            'ADC_HandleTypeDef',
            'CAN_HandleTypeDef',
            'CEC_HandleTypeDef',
            'CRC_HandleTypeDef',
            'CRYP_HandleTypeDef',
            'DAC_HandleTypeDef',
            'DCMI_HandleTypeDef',
            'DFSDM_Channel_HandleTypeDef',
            'DFSDM_Filter_HandleTypeDef',
            'DMA_HandleTypeDef',
            'DMA2D_HandleTypeDef',
            'DSI_HandleTypeDef',
            'ETH_HandleTypeDef',
            'EXTI_HandleTypeDef',
            'HASH_HandleTypeDef',
            'HCD_HandleTypeDef',
            'I2C_HandleTypeDef',
            'I2S_HandleTypeDef',
            'IRDA_HandleTypeDef',
            'IWDG_HandleTypeDef',
            'JPEG_HandleTypeDef',
            'LPTIM_HandleTypeDef',
            'LTDC_HandleTypeDef',
            'MDIOS_HandleTypeDef',
            'MMC_HandleTypeDef',
            'NAND_HandleTypeDef',
            'NOR_HandleTypeDef',
            'PCD_HandleTypeDef',
            'QSPI_HandleTypeDef',
            'RNG_HandleTypeDef',
            'RTC_HandleTypeDef',
            'SAI_HandleTypeDef',
            'SD_HandleTypeDef',
            'SDRAM_HandleTypeDef',
            'SMARTCARD_HandleTypeDef',
            'SMBUS_HandleTypeDef',
            'SPDIFRX_HandleTypeDef',
            'SPI_HandleTypeDef',
            'SRAM_HandleTypeDef',
            'TIM_HandleTypeDef',
            'UART_HandleTypeDef',
            'USART_HandleTypeDef',
            'WWDG_HandleTypeDef',
            'HRTIM_HandleTypeDef',
            ]

    AddDefined('HAL_GPIO_MODULE_ENABLED')
    AddDefined('HAL_FLASH_MODULE_ENABLED')
    for ss in f1.readlines():
        for device_type in device_type_list:
            index = ss.find(device_type, 0)
            if index != 0:
                continue
            index1 = ss.find(';', 0)
            instance = ss[len(device_type)+2:index1]
            f2.write('extern ' + ss)
                
            instance_NAME = str(instance.upper())

            index_type_name = ss.find('_HandleTypeDef', 0)
            type_name = ss[index:index_type_name]
            type_NAME = str(type_name.upper())
            key = "HAL_" + type_NAME + "_MODULE_ENABLED"
            #print(key)
            AddDefined(key)
                
            if device_type == 'I2C_HandleTypeDef':
                index2 = -1
                index3 = -1
                index4 = -1
                gpio_pin = ['0x00','0x00']
                f3 = open(msp, 'r+', newline = '')
                for gpio in f3.readlines(): 
                    index = gpio.find(instance_NAME + ' GPIO Configuration', 0)        
                    if index > 0:
                        index2 = index
                        
                    index3 = gpio.find(' P', 0)
                    index_SCL = gpio.find('_SCL', 0)
                    index_SDA = gpio.find('_SDA', 0)
                    if index2 != -1 and index3 != -1:
                        gpio_type = gpio[index3+2:index3+3]
                        gpio_pin_byte0 = gpio[index3+3:index3+4]
                        gpio_pin_byte1 = gpio[index3+4:index3+5]
                        if gpio_pin_byte1 == ' ':
                            gpio_num = (ord(gpio_type) - ord('A'))*16 + ord(gpio_pin_byte0)-ord('0')
                        else:
                            gpio_num = (ord(gpio_type) - ord('A'))*16 + (ord(gpio_pin_byte0)-ord('0'))*10 + ord(gpio_pin_byte1)-ord('0')
                        
                        if (index_SCL > 0): 
                            gpio_pin[0] = hex(gpio_num)
                            index_SCL = -1
                        if (index_SDA > 0):
                            gpio_pin[1] = hex(gpio_num)
                            index_SDA = -1
                        continue
                          
                    index4 = gpio.find('*/', 0)
                    flag_fined_pin = 0
                    if index4 != -1 and index2 != -1:
                        index2 = -1
                        flag_fined_pin = 1
                        instance_intercept = instance[0:3]
                        f2.write('struct stm32_' + instance_intercept + '_info ' + instance + '_info = {.instance = &h' + instance + ', ')
                        f2.write('.scl = ' + gpio_pin[0] + ', ')
                        f2.write('.sda = ' + gpio_pin[1] + '};\n')
                        f3.close()
                        break
                if flag_fined_pin == 1:
                    f2.write('OS_HAL_DEVICE_DEFINE("' + device_type + '", "hard_' + instance + '", ' + instance + "_info);\n\n")
            elif device_type == 'HCD_HandleTypeDef':
                instance_intercept = instance[0:3]
                f2.write('struct stm32_' + instance_intercept + '_info ' + instance + '_info = {.instance = &h' + instance + ', .host_type = ' + instance[-6:] + '};\n')
                f2.write('OS_HAL_DEVICE_DEFINE("' + device_type + '", "hard_' + instance + '", ' + instance + "_info);\n\n")
            elif device_type == 'PCD_HandleTypeDef':
                instance_intercept = instance[0:3]
                f2.write('struct stm32_' + instance_intercept + '_info ' + instance + '_info = {.instance = &h' + instance + ', .interface_type = "%s"};\n' % (instance[-14:].upper()))
                f2.write('OS_HAL_DEVICE_DEFINE("' + device_type + '", "hard_' + instance + '", ' + instance + "_info);\n\n")
            else:  
                f2.write('OS_HAL_DEVICE_DEFINE("' + device_type + '", "' + instance + '", h' + instance + ');\n\n')
            
    f1.close()
    f2.close()

def gen_stm32_middlewares_file(prj_path, bsp_path, ioc_path):
    if ioc_path == None:
        return

    ioc_file = prj_path + "/" + ioc_path
    
    with open(ioc_file, 'r+', newline = '') as fd:
        for ss in fd.readlines():
            if 'VP_OPENAMP_VS_OPENAMP.Mode=OpenAmp_Activated' in ss:
                AddDefined('HAL_OPENAMP_MODULE_ENABLED')
    
def prebuild(prj_path, bsp_path = '/board/CubeMX_Config/Src/', ioc_path = None):
    print("project " + prj_path)
    gen_stm32_bsp_file(prj_path, bsp_path)
    gen_stm32_devices_file(prj_path, bsp_path)
    gen_stm32_middlewares_file(prj_path, bsp_path, ioc_path)
    gen_stm32_it_file(prj_path, bsp_path)
    
    loader = importlib.machinery.SourceFileLoader('prebuild.py', Env['OS_ROOT'] + '/drivers/boot/cotex-m/prebuild.py')
    spec   = importlib.util.spec_from_loader(loader.name, loader)
    mod    = importlib.util.module_from_spec(spec)
    loader.exec_module(mod)
    mod.gen_cotex_m_link_file(prj_path)
    
