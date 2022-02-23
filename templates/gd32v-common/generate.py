import os
import sys
import shutil

def template(soc_type):
    
    print(soc_type)

    realdir = os.path.dirname(os.path.realpath(__file__))
        
    source_path = os.path.join(realdir, './')
    target_path = os.path.join(realdir, 'target/')
    
    if os.path.exists(source_path):
        
        if os.path.exists(target_path):
            shutil.rmtree(target_path)
        
        if os.path.exists(target_path):
            return None
        
        shutil.copytree(source_path, target_path)
        
        return target_path

    return None
