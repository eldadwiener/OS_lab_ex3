hwgrader
========
A package for running test for linux_kernel course. This package
contains several modules that simplify writing/executing tests.

To install the package cd into the root folder of the package and
enter:

 python setup.py install

Tests:
--------------
The testing mechanism is based on the [unittest][1] package of python
(see link for documentation).
To run the test enter:
 
    > python <path to test>

e.g.:

 python tests/2019s/hw1/hw1_test.py

Note, You should be inside your source code folder when running the
command above (contains the pre-built LKM).
 
[1]: http://docs.python.org/release/2.2.1/lib/module-unittest.html
