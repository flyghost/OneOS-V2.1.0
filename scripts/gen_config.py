#!/usr/bin/env python3
# coding:utf-8


"""
Updates an old .config file or creates a new one, by filling in default values
for all new symbols. This is the same as picking the default selection for all
symbols in oldconfig, or entering the menuconfig interface and immediately
saving.

The default input/output filename is '.config'. A different filename can be
passed in the KCONFIG_CONFIG environment variable.

When overwriting a configuration file, the old version is saved to
<filename>.old (e.g. .config.old).
"""
import kconfiglib_ex
import os
import sys


def mk_osconfig(filename):
    try:
        config = open(filename)
    except Exception as e:
        print('open .config failed: {}'.format(e))
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

            # comment_line = line[1:]
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


def init_config_by_kconfig():
    """generate .config from default Kconfig"""
    kconf = kconfiglib_ex.standard_kconfig()
    kconf.load_config()
    kconf.write_config()
    mk_osconfig('.config')
