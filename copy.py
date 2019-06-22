from shutil import copyfile

files_to_copy = [\
    'include/linux/my_mpi.h',\
    'include/linux/sched.h',\
    'mpi_messages_api.h',\
    'arch/i386/kernel/entry.S',\
    'kernel/Makefile',\
    'kernel/my_mpi.c',\
    'kernel/fork.c',\
    'kernel/exit.c']

for file in files_to_copy:
    copyfile('/mnt/hgfs/shared_folder/OS_lab_ex_2/' + file, '/usr/src/linux-2.4.18-14custom/'+ file)