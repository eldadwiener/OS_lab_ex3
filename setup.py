#!/usr/bin/python

from distutils.core import setup, Extension

module1 = Extension(
            'MPI',
            define_macros = [('MAJOR_VERSION', '1'),
                             ('MINOR_VERSION', '0')],
            sources = ['py_mpi_messages.c']
            )

setup(
    name = 'PackageName',
    version = '1.0',
    description = 'system calls wrappers',
    author = 'Amit Aides',
    author_email = 'amitibo@tx',
    url = 'http://www.ee.technion.ac.il',
    long_description = "System calls wrappers for third assignment in course 046210.",
    ext_modules = [module1]
    )
