#!/usr/bin/env python3

import os
import sys


def run(cmd):
    status = os.system(cmd)
    if status:
        print('%s failed: %s' % (cmd, status))
        sys.exit(status)


if 'rebuild' in sys.argv:
    if sys.platform == 'win32':
        run('del /F /Q build')
    else:
        run('rm -rf build')

if not os.path.exists('build'):
    run('mkdir build')
os.chdir('build')
run('cmake ..')
if sys.platform == 'win32':
    run('cmake --build . --config Release')
    os.chdir('..')
    if not os.path.exists('.\\build\\Release\\test_runner.exe'):
        print('Test runner not found!')
        import pprint
        pprint.pprint(os.walk('build'))
    run('.\\build\\Release\\test_runner.exe')
else:
    run('make')
    os.chdir('..')
    run('./build/test_runner')
