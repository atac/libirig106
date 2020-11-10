#!/usr/bin/env python3

import os


os.system('rm -rf build && mkdir build')
os.chdir('build')
os.system('cmake ..')
os.system('make')
