# -*- coding:utf-8 -*-
#
# File      : cmd_menuconfig.py
# COPYRIGHT (C) 2012 - 2020, CMCC IOT
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#

import os
import platform
import kconfig_mconf

'''menuconfig for system configuration'''

# make oneos_config.h from .config


def mk_osconfig(filename):
    try:
        config = open(filename)
    except:
        print( 'open .config failed',e)
        return

    osconfig = open('oneos_config.h', 'w')
    osconfig.write('#ifndef __ONEOS_CONFIG_H__\n')
    osconfig.write('#define __ONEOS_CONFIG_H__\n\n')

    empty_line = 1

    for line in config:
        line = line.lstrip(' ').replace('\n', '').replace('\r', '')

        if len(line) == 0:
            continue

        if line[0] == '#':
            if len(line) == 1:
                if empty_line:
                    continue

                osconfig.write('\n')
                empty_line = 1
                continue

            #comment_line = line[1:]
            if line.startswith('# CONFIG_'):
                line = ' ' + line[9:]
            else:
                line = line[1:]
                osconfig.write('/*%s */\n' % line)

            empty_line = 0
        else:
            empty_line = 0
            setting = line.split('=')
            if len(setting) == 2:
                if setting[0].startswith('CONFIG_'):
                    setting[0] = setting[0][7:]

                # remove CONFIG_PKG_XX_PATH or CONFIG_PKG_XX_VER
                if type(setting[0]) == type('a') and (setting[0].endswith('_PATH') or setting[0].endswith('_VER')):
                    continue

                if setting[1] == 'y':
                    osconfig.write('#define %s\n' % setting[0])
					
				# ignore unchosen macros
                elif setting[1] == '':
                    continue
                else:
                    osconfig.write('#define %s %s\n' %
                                   (setting[0], setting[1]))

            elif len(setting) > 2:
                alt_data = line[len(setting[0]) + 1:]

                if setting[0].startswith('CONFIG_'):
                    setting[0] = setting[0][7:]

                # remove CONFIG_PKG_XX_PATH or CONFIG_PKG_XX_VER
                if type(setting[0]) == type('a') and (setting[0].endswith('_PATH') or setting[0].endswith('_VER')):
                    continue

                osconfig.write('#define %s %s\n' % (setting[0], alt_data))

    if os.path.isfile('rtconfig_project.h'):
        osconfig.write('#include "rtconfig_project.h"\n')

    osconfig.write('\n')
    osconfig.write('#endif /* __ONEOS_CONFIG_H__ */\n\n')
    osconfig.close()
    


def find_macro_in_config(filename, macro_name):
    try:
        config = open(filename)
    except:
        print ('open .config failed')
        return

    empty_line = 1

    for line in config:
        line = line.lstrip(' ').replace('\n', '').replace('\r', '')

        if len(line) == 0:
            continue

        if line[0] == '#':
            if len(line) == 1:
                if empty_line:
                    continue

                empty_line = 1
                continue

            #comment_line = line[1:]
            if line.startswith('# CONFIG_'):
                line = ' ' + line[9:]
            else:
                line = line[1:]

            # print line

            empty_line = 0
        else:
            empty_line = 0
            setting = line.split('=')
            if len(setting) >= 2:
                if setting[0].startswith('CONFIG_'):
                    setting[0] = setting[0][7:]

                    if setting[0] == macro_name and setting[1] == 'y':
                        return True

    return False


def cmd(*args):
    os_version = platform.platform(True)[10:13]

    if not os.path.exists('Kconfig'):
        if platform.system() == "Windows":
            os.system('chcp 65001  > nul')

        print("\n\033[1;31;40m<menuconfig> 命令应当在某一特定 project 目录下执行，例如：\"OneOS\project\stm32f10x-kylin-std-display-demo\"\033[0m")
        print("\033[1;31;40m请确保当前目录为 project 根目录，并且该目录中有 Kconfig 文件。\033[0m\n")

        print ("<menuconfig> command should be used in a project root path with a Kconfig file.")
        print ("Example: \"OneOS\project\stm32f10x-kylin-std-display-demo\"")
        print ("You should check if there is a Kconfig file in your project root first.")

        if platform.system() == "Windows":
            os.system('chcp 437  > nul')

        return False

    fn = '.config'

    if os.path.isfile(fn):
        mtime = os.path.getmtime(fn)
    else:
        mtime = -1

    if platform.system() == "Windows":
        os.system('chcp 437  > nul')
        
    if args == "--help":
        print( 'use', args.menuconfig_fn)
        import shutil
        shutil.copy(args.menuconfig_fn, fn)
    else:
        kconfig_mconf._main()

    if os.path.isfile(fn):
        mtime2 = os.path.getmtime(fn)
    else:
        mtime2 = -1
    
    if mtime != mtime2:
        mk_osconfig(fn)


def add_parser(sub):
    parser = sub.add_parser('menuconfig', help=__doc__, description=__doc__)

    parser.add_argument('--config',
                        help='Using the user specified configuration file.',
                        dest='menuconfig_fn')

    parser.add_argument('--generate',
                        help='generate oneos_config.h by .config.',
                        action='store_true',
                        default=False,
                        dest='menuconfig_g')

    parser.add_argument('--silent',
                        help='Silent mode,don\'t display menuconfig window.',
                        action='store_true',
                        default=False,
                        dest='menuconfig_silent')

    parser.add_argument('-s', '--setting',
                        help='buildbox config,auto update packages and create mdk/iar project',
                        action='store_true',
                        default=False,
                        dest='menuconfig_setting')

    parser.add_argument('--easy',
                        help='easy mode,place kconfig file everywhere,just modify the option buildbox="OS_ROOT" default "../.."',
                        action='store_true',
                        default=False,
                        dest='menuconfig_easy')

    parser.set_defaults(func=cmd)
