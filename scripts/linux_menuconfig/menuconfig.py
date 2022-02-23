# -*- coding:utf-8 -*-
#
# File      : menuconfig.py
# This file is part of OneOS RTOS
#

import os
import sys
import argparse
import platform

import cmd_menuconfig

__version__ = 'OneOS packages v1.1.0'



def main():
    bsp_root = os.getcwd()
    os_root = os.path.join(bsp_root, "../..")
    script_root = os.path.split(os.path.realpath(__file__))[0]

    sys.path = sys.path + [os.path.join(script_root)]


    try:
        bsp_root.encode().decode("ascii")
        
    except Exception as e:
        
        if platform.system() == "Windows":
            os.system('chcp 65001  > nul')
        
        print ("\n\033[1;31;40m警告：\033[0m")
        print ("\033[1;31;40m当前路径不支持非英文字符，请修改当前路径为纯英文路径。\033[0m")
        print ("\033[1;31;40mThe current path does not support non-English characters.\033[0m")
        print ("\033[1;31;40mPlease modify the current path to a pure English path.\033[0m")
        print(bsp_root)

        if platform.system() == "Windows":
            os.system('chcp 437  > nul')

        return False
    cmd_menuconfig.cmd()


if __name__ == '__main__':
    main()

