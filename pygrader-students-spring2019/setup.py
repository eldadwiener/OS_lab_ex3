#!/usr/bin/python

from distutils.core import setup
from distutils.extension import Extension
import os
from hwgrader import BASE_INSTALL_PATH, AUTOLOGIN_FILES_FOLDER, \
     MMLOG_FILES_FOLDER, KERNEL_MODIFICATIONS_FOLDER, BUILD_SCRIPT, BUILD_NM_SCRIPT, \
     WD_FILES_FOLDER


def listDataFiles(root_install, root_data):
    def func(arg, dirname, fnames):
        for name in fnames:
            p = os.path.join(dirname, name)
            if os.path.isfile(p):
                arg.append(p)

    file_list = []
    os.path.walk(root_data, func, file_list)

    data_list = []
    for f in file_list:
        base_path, fn = os.path.split(f)
        data_list.append((os.path.join(root_install, base_path[4:]), [f]))
        
    return data_list


def main():

    #
    # Compile necessary files.
    #
    os.chdir('src')
    status = os.system('make')
    if status:
        raise Exception('Unable to make grader object/executables')
    os.chdir('..')

    #
    # Create base dir
    #
    #if os.path.exists(BASE_INSTALL_PATH):
    #    shutil.rmtree(BASE_INSTALL_PATH)
    #os.makedirs(BASE_INSTALL_PATH)
    
    #
    # Copy necessary files.
    #
    data_files = listDataFiles(BASE_INSTALL_PATH, os.path.join('src', KERNEL_MODIFICATIONS_FOLDER))
    data_files += listDataFiles(BASE_INSTALL_PATH, os.path.join('src', AUTOLOGIN_FILES_FOLDER))
    data_files.append((os.path.join(BASE_INSTALL_PATH, AUTOLOGIN_FILES_FOLDER), ['src/autologin']))
    data_files.append((os.path.join(BASE_INSTALL_PATH, MMLOG_FILES_FOLDER), ['src/mmlog_module.o']))
    data_files.append((os.path.join(BASE_INSTALL_PATH, WD_FILES_FOLDER), ['src/softdog.o']))

    #
    # Run the setup
    #
    setup(
        name='hwgrader',
        version='0.9',
        description='HW grader written in python',
        author='Amit Aides',
        author_email='amitibo@tx.technion.ac.il',
        url='http://www.technion.ac.il/~amitibo/',
        packages=['hwgrader'],
        ext_modules=[Extension('hwgrader.watchdogdev', ['src/watchdogdev.c'])],
        scripts=[
            'scripts/make_changed.py',
            'scripts/%s' % BUILD_SCRIPT,
            'scripts/%s' % BUILD_NM_SCRIPT,
            'scripts/mmlog_module_load',
            'scripts/mmlog_module_unload',
            'scripts/wd_module_load',
            'scripts/wd_module_unload'
            ],
        data_files=data_files
        )

    
if __name__ == '__main__':
    main()
