#!/usr/bin/env python
#
# Usage: python configure.py [--interactive]

from __future__ import print_function

import sys
import re
import config
from   config import ansi_bold, ansi_red, ansi_blue, ansi_normal
from   config import Error

#-------------------------------------------------------------------------------
# header

print( '-'*80 + '\n' +
ansi_bold + ansi_red + '                              Welcome to testsweeper.' +
ansi_normal + '''

By default, configure will automatically choose the first valid value it finds
for each option. You can set it to interactive to find all possible values and
give you a choice:
    ''' + ansi_blue + 'make config interactive=1' + ansi_normal + '''

If you have multiple compilers, we suggest specifying your desired compiler by
setting CXX, as the automated search may prefer a different compiler.
''' + '-'*80 )

#-------------------------------------------------------------------------------
def main():
    config.init( prefix='/usr/local/testsweeper' )
    config.prog_cxx()
    config.prog_cxx_flags([
        '-O2', '-std=c++11', '-MMD',
        '-Wall',
        '-pedantic',
        '-Wshadow',
        '-Wno-unused-local-typedefs',
        '-Wno-unused-function',
        #'-Wmissing-declarations',
        #'-Wconversion',
        #'-Werror',
    ])
    config.openmp()

    config.output_files( 'make.inc' )

    print( '-'*80 )
# end

#-------------------------------------------------------------------------------
try:
    main()
except Error as err:
    print( ansi_bold + ansi_red + 'A fatal error occurred. ' + str(err) + '\n'
           'testsweeper could not be configured.' + ansi_normal )
