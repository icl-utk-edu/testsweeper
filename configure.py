#!/usr/bin/env python3
#
# Copyright (c) 2017-2022, University of Tennessee. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause
# This program is free software: you can redistribute it and/or modify it under
# the terms of the BSD 3-Clause license. See the accompanying LICENSE file.
#
# Usage: python3 configure.py [--interactive]

from __future__ import print_function

import sys
import re
import config
from   config import Error, font, print_msg, print_warn, print_header

#-------------------------------------------------------------------------------
# header

print( '-'*80 + '\n' +
font.bold( font.blue( '                              Welcome to TestSweeper.' ) ) +
'''

By default, configure will automatically choose the first valid value it finds
for each option. You can set it to interactive to find all possible values and
give you a choice:
    ''' + font.blue( 'make config interactive=1' ) + '''

If you have multiple compilers, we suggest specifying your desired compiler by
setting CXX, as the automated search may prefer a different compiler.

For ANSI colors, set color=auto (when output is TTY), color=yes, or color=no.

See INSTALL.md for more details.
''' + '-'*80 )

#-------------------------------------------------------------------------------
def main():
    config.init( namespace='TestSweeper', prefix='/opt/slate' )
    config.prog_cxx()

    print_header( 'C++ compiler flags' )
    # Pick highest level supported. oneAPI needs C++17.
    # Crusher had issue with -std=c++20 (2022-07).
    config.prog_cxx_flag(
        ['-std=c++17', '-std=c++14', '-std=c++11'])
    config.prog_cxx_flag( '-O2' )
    config.prog_cxx_flag( '-MMD' )
    config.prog_cxx_flag( '-Wall' )
    config.prog_cxx_flag( '-Wno-unused-local-typedefs' )
    config.prog_cxx_flag( '-Wno-unused-function' )
    config.prog_cxx_flag( '-pedantic' )
    config.prog_cxx_flag( '-Wshadow' )
   #config.prog_cxx_flag( '-Wmissing-declarations' )
   #config.prog_cxx_flag( '-Wconversion' )
   #config.prog_cxx_flag( '-Werror' )

    config.openmp()

    config.output_files( 'make.inc' )
    print( 'log in config/log.txt' )

    print( '-'*80 )
# end

#-------------------------------------------------------------------------------
try:
    main()
except Error as ex:
    print_warn( 'A fatal error occurred. ' + str(ex) +
                '\nTestSweeper could not be configured. Log in config/log.txt' )
    exit(1)
