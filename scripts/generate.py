import os
import sys
import shutil
import importlib
import importlib.util
import time
import re

def find_soc(src, soc):
    soc_dir = [os.path.basename(src)]
    #soc_dir = ['hk32-common', 'replace', 'HK32F030', 'replace', 'TEMP_HK32F030CBT6']
    
    names = os.listdir(src)
    
    for name in names:
        if name == soc:
            soc_dir.append(name)
            #print(soc_dir)
            return soc_dir

        srcname = os.path.join(src, name)

        if not os.path.isdir(srcname):
            continue
            
        sub_soc_dir = find_soc(srcname, soc)
        if len(sub_soc_dir) > 1:
            soc_dir.extend(sub_soc_dir)
            #print(soc_dir)
            return soc_dir
    
    #print(soc_dir)
    return soc_dir

def copytree(src, dst, ignore, symlinks=False):
    names = os.listdir(src)
    if not os.path.isdir(dst):
        os.makedirs(dst)
    
    errors = []
    for name in names:
        if isinstance(ignore, list):
            if name in ignore:
                continue
                
        elif isinstance(ignore, str):
            if name == ignore:
                continue
    
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        
        try:
            if symlinks and os.path.islink(srcname):
                linkto = os.readlink(srcname)
                os.symlink(linkto, dstname)
            elif os.path.isdir(srcname):
                copytree(srcname, dstname, ignore, symlinks)
            else:
                if os.path.isdir(dstname):
                    os.rmdir(dstname)
                elif os.path.isfile(dstname):
                    os.remove(dstname)
                shutil.copy2(srcname, dstname)
        except (IOError, os.error) as why:
            errors.append((srcname, dstname, str(why)))
        except OSError as err:
            errors.extend(err.args[0])
        
def template(source_path, g):
    target_path = os.path.join(source_path, 'target')

    for item in g:
        if 'TEMP_' in item:
            soc = item

    print(source_path)
    print(target_path)
    print(soc)
    
    if os.path.exists(target_path):
        shutil.rmtree(target_path)
        
    # find soc dir
    soc_dir = find_soc(source_path, soc)
    print("soc_dir")
    print(soc_dir)
    #soc_dir = ['hk32-common', 'replace', 'HK32F030', 'replace', 'TEMP_HK32F030CBT6']
    
    if len(soc_dir) < 2:
        print("invalid soc_dir %s" % soc_dir)
        return None
    
    # copy replace files
    replace_path = source_path
    copytree(replace_path, target_path, ['replace', 'generate.py'])
    
    for i in range(1, len(soc_dir)):
        if i % 2 == 1 and soc_dir[i] != 'replace':
            print("invalid soc_dir %s\n"
                  "soc_dir[%d] is %s, expect 'replace'"
                  % (soc_dir, i, soc_dir[i]))
            return None
    
        replace_path = os.path.join(replace_path, soc_dir[i])
        
        if soc_dir[i] == 'replace':
            continue
            
        copytree(replace_path, target_path, ['replace', 'generate.py'])
    
    return target_path
