#
# File      : utils.py
# This file is part of CMCC IOT OS
# COPYRIGHT (C) 2012-2020, CMCC IOT
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

import sys
import os
import re

def splitall(loc):
    """
    Return a list of the path components in loc. (Used by relpath_).

    The first item in the list will be  either ``os.curdir``, ``os.pardir``, empty,
    or the root directory of loc (for example, ``/`` or ``C:\\).

    The other items in the list will be strings.

    Adapted from *path.py* by Jason Orendorff.
    """
    parts = []
    while loc != os.curdir and loc != os.pardir:
        prev = loc
        loc, child = os.path.split(prev)
        if loc == prev:
            break
        parts.append(child)
    parts.append(loc)
    parts.reverse()
    return parts

def _make_path_relative(origin, dest):
    """
    Return the relative path between origin and dest.

    If it's not possible return dest.


    If they are identical return ``os.curdir``

    Adapted from `path.py <http://www.jorendorff.com/articles/python/path/>`_ by Jason Orendorff.
    """
    origin = os.path.abspath(origin).replace('\\', '/')
    dest = os.path.abspath(dest).replace('\\', '/')
    #
    orig_list = splitall(os.path.normcase(origin))
    # Don't normcase dest!  We want to preserve the case.
    dest_list = splitall(dest)
    #
    if orig_list[0] != os.path.normcase(dest_list[0]):
        # Can't get here from there.
        return dest
    #
    # Find the location where the two paths start to differ.
    i = 0
    for start_seg, dest_seg in zip(orig_list, dest_list):
        if start_seg != os.path.normcase(dest_seg):
            break
        i += 1
    #
    # Now i is the point where the two paths diverge.
    # Need a certain number of "os.pardir"s to work up
    # from the origin to the point of divergence.
    segments = [os.pardir] * (len(orig_list) - i)
    # Need to add the diverging part of dest_list.
    segments += dest_list[i:]
    if len(segments) == 0:
        # If they happen to be identical, use os.curdir.
        return os.curdir
    else:
        # return os.path.join(*segments).replace('\\', '/')
        return os.path.join(*segments)

def xml_indent(elem, level=0):
    i = "\n" + level*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            xml_indent(elem, level+1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i


source_ext = ["c", "h", "s", "S", "cpp", "xpm"]
source_list = []

def walk_children(child):
    global source_list
    global source_ext

    # print child
    full_path = child.rfile().abspath
    file_type_list  = full_path.rsplit('.',1)
    #print file_type
    if (len(file_type_list) > 1):
        file_type = full_path.rsplit('.',1)[1]

        if file_type in source_ext:
            if full_path not in source_list:
                source_list.append(full_path)

    children = child.all_children()
    if children != []:
        for item in children:
            walk_children(item)

def PrefixPath(prefix, path):
    path = os.path.abspath(path)
    prefix = os.path.abspath(prefix)

    if sys.platform == 'win32':
        prefix = prefix.lower()
        path = path.lower()

    if path.startswith(prefix):
        return True
    
    return False

def ListMap(l):
    ret_list = []
    for item in l:
        if type(item) == type(()):
            ret = ListMap(item)
            ret_list += ret
        elif type(item) == type([]):
            ret = ListMap(item)
            ret_list += ret
        else:
            ret_list.append(item)

    return ret_list

def TargetGetList(env, postfix):
    global source_ext
    global source_list

    target = env['target']

    source_ext = postfix
    for item in target:
        walk_children(item)

    source_list.sort()

    return source_list

def ProjectInfo(env):

    project  = env['project']
    OS_ROOT  = env['OS_ROOT']
    BSP_ROOT = env['BSP_ROOT']

    FILES       = []
    DIRS        = []
    HEADERS     = []
    CPPPATH     = []
    CPPDEFINES  = []

    for group in project:
        # get each files
        if 'src' in group and group['src']:
            FILES += group['src']

        # get each include path
        if 'CPPPATH' in group and group['CPPPATH']:
            CPPPATH += group['CPPPATH']

    if 'CPPDEFINES' in env:
        CPPDEFINES = env['CPPDEFINES']
        CPPDEFINES = ListMap(CPPDEFINES)

    # process FILES and DIRS
    if len(FILES):
        # use absolute path 
        for i in range(len(FILES)):
            FILES[i] = os.path.abspath(str(FILES[i]))
            DIRS.append(os.path.dirname(FILES[i]))

        FILES.sort()
        DIRS = list(set(DIRS))
        DIRS.sort()

    # process HEADERS
    HEADERS = TargetGetList(env, ['h'])

    # process CPPPATH
    if len(CPPPATH):
        # use absolute path 
        for i in range(len(CPPPATH)):
            CPPPATH[i] = os.path.abspath(CPPPATH[i])

        # remove repeat path
        paths = [i for i in set(CPPPATH)]
        CPPPATH = []
        for path in paths:
            if PrefixPath(OS_ROOT, path):
                CPPPATH += [os.path.abspath(path).replace('\\', '/')]

            elif PrefixPath(BSP_ROOT, path):
                CPPPATH += [os.path.abspath(path).replace('\\', '/')]

            else:
                CPPPATH += ['"%s",' % path.replace('\\', '/')]

        CPPPATH.sort()

    # process CPPDEFINES
    if len(CPPDEFINES):
        CPPDEFINES_TMP = [i for i in set(CPPDEFINES)]
        CPPDEFINES = []
        for index in CPPDEFINES_TMP:
            CPPDEFINES += [index.replace('=', ' ').replace('\\', '')]
        CPPDEFINES.sort()

    proj = {}
    proj['FILES']       = FILES
    proj['DIRS']        = DIRS
    proj['HEADERS']     = HEADERS
    proj['CPPPATH']     = CPPPATH
    proj['CPPDEFINES']  = CPPDEFINES

    return proj

def VersionCmp(ver1, ver2):
    la=[]
    if ver1:
        la = re.split("[. ]", ver1)
    lb = re.split("[. ]", ver2)

    f = 0
    if len(la) > len(lb):
        f = len(la)
    else:
        f = len(lb)
    for i in range(f):
        try:
            if int(la[i]) > int(lb[i]):
                return 1
            elif int(la[i]) == int(lb[i]):
                continue
            else:
                return -1
        except (IndexError, ValueError) as e:
            if len(la) > len(lb):
                return 1
            else:
                return -1
    return 0

def GCCC99Patch(cflags):
    import build_tools
    gcc_version = build_tools.IsDefined('GCC_VERSION')
    if gcc_version:
        gcc_version = gcc_version.replace('"', '')
    if VersionCmp(gcc_version, "4.8.0") == 1:
        # remove -std=c99 after GCC 4.8.x
        cflags = cflags.replace('-std=c99', '')

    return cflags

def ReloadModule(module):
    import sys
    if sys.version_info.major >= 3:
        import importlib
        importlib.reload(module)
    else:
        reload(module)

    return


def getDowlaodFileName(file_url, headers):
    disposition = headers.get("Content-Disposition")
    tool_file_name = None
    if disposition is not None:
        for item in disposition.split(";"):
            if item.strip().startswith("filename"):
                tool_file_name = item.split("=")[-1].strip("\'\"")
    if tool_file_name is None:
        tool_file_name = os.path.basename(file_url)
    if tool_file_name is None:
        tool_file_type = headers.get("content-type").split("/")[-1]
        tool_file_name = file_url.split("/")[-1] + tool_file_type
    return tool_file_name


def SetupToolFromRemote(tool_path, tool_url):
    """
    setup tool from remote
    :param tool_path: local tool path
    :param tool_url:setup file remote url
    :return:
    """
    try:
        import requests
    except ImportError:
        print("import requests failed, please install python library 'requests', you could try command :")
        print("python -m pip install requests")
        exit(0)
    header = {
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36"
    }
    print("Required compiler missing, Trying to download from: "+tool_url)
    print("...")
    response = requests.get(tool_url, headers=header)
    tool_path = os.path.abspath(tool_path)
    print("Installing compiler to "+tool_path)
    print("...")
    tool_file_name = getDowlaodFileName(tool_url, response.headers)
    tool_file_path = os.path.join(tool_path, tool_file_name)
    if not os.path.exists(tool_path):
        os.makedirs(tool_path)
    try:
        with open(tool_file_path, "wb") as f:
            f.write(response.content)
        if tool_file_name.endswith(".zip"):
            import platform
            try:
                if platform.system() == "Linux":
                    cmd_line="unzip -q "+tool_file_path+" -d "+tool_path
                    os.system(cmd_line)
                else:
                    import zipfile
                    with zipfile.ZipFile(tool_file_path) as zf:
                        zf.extractall(tool_path)
            except Exception as e:
                print(e)
        elif tool_file_name.endswith(".rar"):
            cmd = "unrar x -inul {rar} {folder}".format(rar=tool_file_path, folder=tool_path)
            print(cmd)
            os.system(cmd)
            # import rarfile
            # try:
            #     with rarfile.RarFile(tool_file_path) as rf:
            #         rf.extractall(tool_path)
            # except Exception as e:
            #     print(e)
        elif tool_file_name.endswith(".tar.gz") or tool_file_name.endswith(".tgz"):
            import tarfile
            try:
                with tarfile.open(tool_file_path, "r:gz") as tf:
                    tf.extractall(path=tool_path)
            except Exception as e:
                print(e)
        elif tool_file_name.endswith(".tar.bz2"):
            import tarfile
            try:
                with tarfile.open(tool_file_path, "r:bz2") as tf:
                    tf.extractall(path=tool_path)
            except Exception as e:
                print(e)
        elif tool_file_name.endswith(".gz"):
            import gzip
            try:
                with gzip.GzipFile(tool_file_path) as gf:
                    gf.extractall(tool_path)
            except Exception as e:
                print(e)
    except Exception as e:
        print(str(e))
    finally:
        os.remove(tool_file_path)
        pass
