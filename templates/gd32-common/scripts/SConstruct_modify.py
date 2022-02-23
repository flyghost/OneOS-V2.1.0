import os
import sys
import shutil

def modify(series,model,soc):
    old_path = os.getcwd()

    source_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(source_path+'/../target')
    with open('SConstruct', 'r') as f:
        with open('SConstruct.new', 'w') as g:
            for line in f.readlines():
                if 'OS_ROOT = os.path.normpath(os.getcwd()' in line: 
                    g.write('    OS_ROOT = os.path.normpath(os.getcwd() + \'/../..\')')
                else:
                    g.write(line)
    shutil.move('SConstruct.new', 'SConstruct')

    os.chdir(old_path)




