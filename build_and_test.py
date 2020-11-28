#!/usr/bin/env python3

import os
import sys

if 'rebuild' in sys.argv:
    if sys.platform == 'win32':
        os.system('del /F /Q build')
    else:
        os.system('rm -rf build')

os.system('mkdir build')
os.chdir('build')
os.system('cmake ..')
if sys.platform == 'win32':
    os.system('cmake --build . --config Release')
    os.chdir('..')
    os.system('.\\build\\Release\\test_runner.exe')
else:
    os.system('make')
    os.chdir('..')
    os.system('./build/test_runner')
