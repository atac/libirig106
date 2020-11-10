#!/usr/bin/env python3

import os
import sys


if sys.platform == 'win32':
    os.system('del /F /Q build && mkdir build')
    os.chdir('build')
    os.system('cmake ..')
    os.system('cmake --build . --config Release')
    os.chdir('..')
    os.system('.\\build\\Release\\test_runner.exe')
else:
    os.system('rm -rf build && mkdir build')
    os.chdir('build')
    os.system('cmake ..')
    os.system('make')
    os.chdir('..')
    os.system('./build/test_runner')
