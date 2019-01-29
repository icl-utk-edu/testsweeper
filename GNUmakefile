# Usage:
# make by default:
#    - Runs configure.py to create make.inc, if it doesn't exist.
#    - Compiles libtest.so, or libtest.a (if static=1).
#    - Compiles the tester, example.
#
# make config    - Runs configure.py to create make.inc.
# make lib       - Compiles libtest.so, or libtest.a (if static=1).
# make clean     - Deletes all objects, libraries, and the tester.
# make distclean - Also deletes make.inc and dependency files (*.d).

#-------------------------------------------------------------------------------
# Configuration
# Variables defined in make.inc, or use make's defaults:
#   CXX, CXXFLAGS   -- C compiler and flags
#   LDFLAGS, LIBS   -- Linker options, library paths, and libraries
#   AR, RANLIB      -- Archiver, ranlib updates library TOC
#   prefix          -- where to install libtest
#
# OpenMP is optional; used only for timer and flushing caches.

include make.inc

# Existence of .make.inc.$${PPID} is used so 'make config' doesn't run
# configure.py twice when make.inc doesn't exist initially.
make.inc:
	python configure.py
	touch .make.inc.$${PPID}

.PHONY: config
config:
	if [ ! -e .make.inc.$${PPID} ]; then \
		python configure.py; \
	fi

# defaults if not given in make.inc
CXXFLAGS ?= -O3 -std=c++11 -MMD \
            -Wall -pedantic \
            -Wshadow \
            -Wno-unused-local-typedefs \
            -Wno-unused-function \

#CXXFLAGS += -Wmissing-declarations
#CXXFLAGS += -Wconversion
#CXXFLAGS += -Werror

# GNU make doesn't have defaults for these
RANLIB   ?= ranlib
prefix   ?= /usr/local/lapackpp

# auto-detect OS
# $OSTYPE may not be exported from the shell, so echo it
ostype = $(shell echo $${OSTYPE})
ifneq ($(findstring darwin, $(ostype)),)
	# MacOS is darwin
	macos = 1
endif

#-------------------------------------------------------------------------------
# if shared
ifneq ($(static),1)
	CXXFLAGS += -fPIC
	LDFLAGS  += -fPIC
endif

#-------------------------------------------------------------------------------
# MacOS needs shared library's path set
ifeq ($(macos),1)
	install_name = -install_name @rpath/$(notdir $@)
else
	install_name =
endif

#-------------------------------------------------------------------------------
# Files

lib_src  = libtest.cc
lib_obj  = $(addsuffix .o, $(basename $(lib_src)))
dep      = $(addsuffix .d, $(basename $(lib_src)))

test_src = example.cc
test_obj = $(addsuffix .o, $(basename $(test_src)))
dep     += $(addsuffix .d, $(basename $(test_src)))

test     = example

lib_a  = ./libtest.a
lib_so = ./libtest.so

ifeq ($(static),1)
	lib = $(lib_a)
else
	lib = $(lib_so)
endif

#-------------------------------------------------------------------------------
# libtest specific flags and libraries

# additional flags and libraries for testers
TEST_LDFLAGS += -L. -Wl,-rpath,$(abspath .)
TEST_LIBS    += -ltest

#-------------------------------------------------------------------------------
# Rules
.DELETE_ON_ERROR:
.SUFFIXES:
.PHONY: all lib src test headers include docs clean distclean
.DEFAULT_GOAL = all

all: lib test

install: lib
	mkdir -p $(DESTDIR)$(prefix)/include
	mkdir -p $(DESTDIR)$(prefix)/lib$(LIB_SUFFIX)
	cp *.{hh} $(DESTDIR)$(prefix)/include
	cp libtest.* $(DESTDIR)$(prefix)/lib$(LIB_SUFFIX)

uninstall:
	$(RM) $(addprefix $(DESTDIR)$(prefix), $(headers))
	$(RM) $(DESTDIR)$(prefix)/lib$(LIB_SUFFIX)/libtest.*

#-------------------------------------------------------------------------------
# libtest library
$(lib_so): $(lib_obj)
	mkdir -p lib
	$(CXX) $(LDFLAGS) -shared $(install_name) $(lib_obj) $(LIBS) -o $@

$(lib_a): $(lib_obj)
	mkdir -p lib
	$(RM) $@
	$(AR) cr $@ $(lib_obj)
	$(RANLIB) $@

lib: $(lib)

#-------------------------------------------------------------------------------
# tester
$(test): $(test_obj) $(lib)
	$(CXX) $(TEST_LDFLAGS) $(LDFLAGS) $(test_obj) \
		$(TEST_LIBS) $(LIBS) -o $@

test: $(test)

#-------------------------------------------------------------------------------
# headers
# precompile headers to verify self-sufficiency
headers     = $(wildcard include/*.hh)
headers_gch = $(addsuffix .gch, $(headers))

headers: $(headers_gch)

headers/clean:
	$(RM) *.hh.gch

#-------------------------------------------------------------------------------
# documentation
docs:
	doxygen docs/doxygen/doxyfile.conf
	@echo ========================================
	cat docs/doxygen/errors.txt
	@echo ========================================
	@echo "Documentation available in docs/html/index.html"
	@echo ========================================

# sub-directory redirects
src/docs: docs
test/docs: docs

#-------------------------------------------------------------------------------
# general rules
clean:
	$(RM) *.o *.a *.so example

distclean: clean
	$(RM) make.inc *.d

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# preprocess source
%.i: %.cc
	$(CXX) $(CXXFLAGS) -I$(blaspp_dir)/test -I$(libtest_dir) -E $< -o $@

# precompile header to check for errors
%.h.gch: %.h
	$(CXX) $(CXXFLAGS) -I$(blaspp_dir)/test -I$(libtest_dir) -c $< -o $@

%.hh.gch: %.hh
	$(CXX) $(CXXFLAGS) -I$(blaspp_dir)/test -I$(libtest_dir) -c $< -o $@

-include $(dep)

#-------------------------------------------------------------------------------
# debugging
echo:
	@echo "static        = '$(static)'"
	@echo
	@echo "lib_a         = $(lib_a)"
	@echo "lib_so        = $(lib_so)"
	@echo "lib           = $(lib)"
	@echo
	@echo "lib_src       = $(lib_src)"
	@echo
	@echo "lib_obj       = $(lib_obj)"
	@echo
	@echo "test_src      = $(test_src)"
	@echo
	@echo "test_obj      = $(test_obj)"
	@echo
	@echo "test          = $(test)"
	@echo
	@echo "dep           = $(dep)"
	@echo
	@echo "CXX           = $(CXX)"
	@echo "CXXFLAGS      = $(CXXFLAGS)"
	@echo
	@echo "LDFLAGS       = $(LDFLAGS)"
	@echo "LIBS          = $(LIBS)"
	@echo
	@echo "TEST_LDFLAGS  = $(TEST_LDFLAGS)"
	@echo "TEST_LIBS     = $(TEST_LIBS)"
