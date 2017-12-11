pwd = $(shell pwd)

CXX      = g++
CXXFLAGS = -fPIC -fopenmp -Wall -pedantic -std=c++11 -MMD
LDFLAGS  = -fPIC -fopenmp
LIBS     = -L. -Wl,-rpath,${pwd} -ltest

# --------------------
# MacOS (darwin) needs shared library's path set
# $OSTYPE may not be exported from the shell, so echo it
ostype = ${shell echo $${OSTYPE}}
ifneq ($(findstring darwin, ${ostype}),)
   install_name = -install_name @rpath/$(notdir $@)
else
   install_name =
endif

# --------------------
# library
src = libtest.cc
obj = $(addsuffix .o, $(basename ${src}))
dep = $(addsuffix .d, $(basename ${src}))

.PHONY: all lib

all: lib example
lib: libtest.a libtest.so

libtest.a: ${obj}
	ar cr $@ $^
	ranlib $@

libtest.so: ${obj}
	${CXX} ${LDFLAGS} -shared ${install_name} -o $@ $^

# --------------------
# example
ex_src = example.cc
ex_obj = $(addsuffix .o, $(basename ${ex_src}))
ex_dep = $(addsuffix .d, $(basename ${ex_src}))

example: ${ex_obj} | lib
	${CXX} ${LDFLAGS} -o $@ $^ ${LIBS}

# --------------------
# general rules
%.o: %.cc
	${CXX} ${CXXFLAGS} -c -o $@ $<

-include ${dep}
-include ${ex_dep}

.PHONY: clean distclean

clean:
	-rm -f libtest.a libtest.so ${obj} ${dep}
	-rm -f example ${ex_obj} ${ex_dep}

distclean: clean
	-rm -f *.a *.so *.o *.d

# ------------------------------------------------------------------------------
# precompile headers to verify self-sufficiency
headers     = ${wildcard *.hh}
headers_gch = ${addprefix gch/, ${addsuffix .gch, ${headers}}}

test_headers: ${headers_gch}

gch/%.hh.gch: %.hh | gch
	${CXX} ${CXXFLAGS} ${LAPACKPP_FLAGS} -c -o $@ $<

# make directories
gch:
	mkdir $@
