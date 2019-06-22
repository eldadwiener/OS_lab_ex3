#!/usr/bin/python

"""
Copy changes of a repository (workcopy to root of repo)

Author: Amit Aides
"""
from __future__ import division
import shutil
import os


REPO_PATH = r'D:\amit\teaching\linux_kernel\files\linux-2.4.18-14custom'
DEST_PATH = r'D:\amit\teaching\linux_kernel\shared\kernel_modifications'

def main():
    """main"""

    import subprocess as sbp
    
    os.chdir(REPO_PATH)
    p = sbp.Popen('hg status --rev 0 -m -a', stdout=sbp.PIPE)
    temp = p.stdout.read()
    
    updated_paths = [k[2:] for k in temp.split('\n') if k]
    
    if os.path.exists(DEST_PATH):
        shutil.rmtree(DEST_PATH, ignore_errors=True)

    for src_file in updated_paths:
        dst_file = os.path.join(DEST_PATH, src_file)
        dst_path, file_name = os.path.split(dst_file)
        
        if not os.path.exists(dst_path):
            os.makedirs(dst_path)
        
        shutil.copyfile(src_file, dst_file)


if __name__ == '__main__':
    if os.name != 'nt':
        raise Exception('This script should be run on the win32 host')
    
    main()
    
