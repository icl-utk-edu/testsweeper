#!/usr/bin/env python
#
# Usage: python configure.py [--interactive]

from __future__ import print_function

import sys
import re
import config
from   config import Error, font

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

See INSTALL.txt for more details.
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

    if (config.environ['color'] in ('n', 'no', 'never')):
        config.environ.append( 'CXXFLAGS', '-DNO_COLOR' )

    config.output_files( 'make.inc' )
    print( 'log in config/log.txt' )

    print( '-'*80 )
# end

#-------------------------------------------------------------------------------
try:
    main()
except Error as ex:
    print( font.bold( font.red( 'A fatal error occurred. ' + str(ex) + '\n'
           'TestSweeper could not be configured. Log in config/log.txt' ) ) )
    exit(1)
