#!/usr/bin/env python3

import os
import sys


if sys.platform == 'win32':
    os.system('del /F /Q build && mkdir build')
    os.chdir('build')
    os.system('cmake ..')
    os.system('cmake build .')
    # os.chdir('..')
    # os.system('./build/run_tests')
else:
    os.system('rm -rf build && mkdir build')
    os.chdir('build')
    os.system('cmake ..')
    os.system('make')
    os.chdir('..')
    os.system('./build/run_tests')
