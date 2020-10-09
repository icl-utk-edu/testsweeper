# See INSTALL.md for usage.

#-------------------------------------------------------------------------------
# Configuration
# Variables defined in make.inc, or use make's defaults:
#   CXX, CXXFLAGS   -- C compiler and flags
#   LDFLAGS, LIBS   -- Linker options, library paths, and libraries
#   AR, RANLIB      -- Archiver, ranlib updates library TOC
#   prefix          -- where to install TestSweeper
#
# OpenMP is optional; used only for timer and flushing caches.

ifeq ($(MAKECMDGOALS),config)
    # For `make config`, don't include make.inc with previous config;
    # force re-creating make.inc.
    .PHONY: config
    config: make.inc

    make.inc: force
else ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
    # For `make clean` or `make distclean`, don't include make.inc,
    # which could generate it. Otherwise, include make.inc.
    include make.inc
endif

force: ;

make.inc:
	python configure.py

# Defaults if not given in make.inc. GNU make doesn't have defaults for these.
RANLIB   ?= ranlib
prefix   ?= /opt/slate

# auto-detect OS
# $OSTYPE may not be exported from the shell, so echo it
ostype := $(shell echo $${OSTYPE})
ifneq ($(findstring darwin, $(ostype)),)
    # MacOS is darwin
    macos = 1
endif

#-------------------------------------------------------------------------------
# if shared
ifneq ($(static),1)
    CXXFLAGS += -fPIC
    LDFLAGS  += -fPIC
    lib_ext = so
else
    lib_ext = a
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

lib_src  = testsweeper.cc version.cc
lib_obj  = $(addsuffix .o, $(basename $(lib_src)))
dep     += $(addsuffix .d, $(basename $(lib_src)))

tester_src = example.cc
tester_obj = $(addsuffix .o, $(basename $(tester_src)))
dep       += $(addsuffix .d, $(basename $(tester_src)))

tester = example

#-------------------------------------------------------------------------------
# Get Mercurial id, and make version.o depend on it via .id file.

ifneq ($(wildcard .git),)
    id := $(shell git rev-parse --short HEAD)
    version.o: CXXFLAGS += -DTESTSWEEPER_ID='"$(id)"'
endif

last_id := $(shell [ -e .id ] && cat .id || echo 'NA')
ifneq ($(id),$(last_id))
    .id: force
endif

.id:
	echo $(id) > .id

version.o: .id

#-------------------------------------------------------------------------------
# TestSweeper specific flags and libraries

# additional flags and libraries for testers
TEST_LDFLAGS += -L. -Wl,-rpath,$(abspath .)
TEST_LIBS    += -ltestsweeper

#-------------------------------------------------------------------------------
# Rules
.DELETE_ON_ERROR:
.SUFFIXES:
.PHONY: all lib src test tester headers include docs clean distclean
.DEFAULT_GOAL = all

all: lib tester

install: lib
	mkdir -p $(DESTDIR)$(prefix)/include
	mkdir -p $(DESTDIR)$(prefix)/lib$(LIB_SUFFIX)
	cp $(headers) $(DESTDIR)$(prefix)/include
	cp $(lib) $(DESTDIR)$(prefix)/lib$(LIB_SUFFIX)

uninstall:
	$(RM) $(addprefix $(DESTDIR)$(prefix)/include/, $(headers))
	$(RM) $(DESTDIR)$(prefix)/lib$(LIB_SUFFIX)/libtestsweeper.*

#-------------------------------------------------------------------------------
# if re-configured, recompile everything
$(lib_obj) $(tester_obj): make.inc

#-------------------------------------------------------------------------------
# TestSweeper library
lib_a  = libtestsweeper.a
lib_so = libtestsweeper.so
lib    = libtestsweeper.$(lib_ext)

$(lib_so): $(lib_obj)
	$(CXX) $(LDFLAGS) -shared $(install_name) $(lib_obj) $(LIBS) -o $@

$(lib_a): $(lib_obj)
	$(RM) $@
	$(AR) cr $@ $(lib_obj)
	$(RANLIB) $@

lib: $(lib)

#-------------------------------------------------------------------------------
# tester
$(tester): $(tester_obj) $(lib)
	$(CXX) $(TEST_LDFLAGS) $(LDFLAGS) $(tester_obj) \
		$(TEST_LIBS) $(LIBS) -o $@

# sub-directory rules
test: $(tester)
tester: $(tester)

check:
	cd test; python run_tests.py

#-------------------------------------------------------------------------------
# headers
# precompile headers to verify self-sufficiency
headers     = testsweeper.hh
headers_gch = $(addsuffix .gch, $(basename $(headers)))

headers: $(headers_gch)

headers/clean:
	$(RM) $(headers_gch)

#-------------------------------------------------------------------------------
# documentation
docs:
	@echo "Doxygen not yet implemented."

docs-todo:
	doxygen docs/doxygen/doxyfile.conf
	@echo ========================================
	cat docs/doxygen/errors.txt
	@echo ========================================
	@echo "Documentation available in docs/html/index.html"
	@echo ========================================

#-------------------------------------------------------------------------------
# general rules
clean:
	$(RM) *.o *.a *.so *.gch $(tester)

distclean: clean
	$(RM) make.inc $(dep)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# preprocess source
%.i: %.cc
	$(CXX) $(CXXFLAGS) -E $< -o $@

# precompile header to check for errors
%.gch: %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.gch: %.hh
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(dep)

#-------------------------------------------------------------------------------
# debugging
echo:
	@echo "static        = '$(static)'"
	@echo "id            = '$(id)'"
	@echo "last_id       = '$(last_id)'"
	@echo
	@echo "lib_a         = $(lib_a)"
	@echo "lib_so        = $(lib_so)"
	@echo "lib           = $(lib)"
	@echo
	@echo "lib_src       = $(lib_src)"
	@echo
	@echo "lib_obj       = $(lib_obj)"
	@echo
	@echo "tester_src    = $(tester_src)"
	@echo
	@echo "tester_obj    = $(tester_obj)"
	@echo
	@echo "tester        = $(tester)"
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
