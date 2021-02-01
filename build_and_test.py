#!/usr/bin/env python3

import os
import shutil
import sys

windows = sys.platform == 'win32'


def run(cmd):
    status = os.system(cmd)
    if status and not windows:
        print('%s failed: %s' % (cmd, status))
        sys.exit(status)


# Ensure build directory exists (recreate if requested)
if 'rebuild' in sys.argv:
    shutil.rmtree('build')
if not os.path.exists('build'):
    run('mkdir build')

# Run cmake
os.chdir('build')
run('cmake ..')

# Run platform-specific build command
run('cmake --build . --config Release' if windows else 'make')
os.chdir('..')

# Run test suite (must be from root project directory due to test file paths)
test_runner = './build/test_runner'
if windows:
    test_runner = '.\\build\\Release\\test_runner.exe'
if not os.path.exists(test_runner):
    print('Test runner not found!')
run(test_runner)
