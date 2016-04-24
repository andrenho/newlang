#
# programs to build
#
#lib_LTLIBRARIES = libzoe.la
libzoe_la_SOURCES = vm/ztype.h vm/ztype.cc			\
		    vm/zvalue.h 				\
		    vm/znil.h 					\
		    vm/zbool.h 					\
		    vm/znumber.h 				\
		    vm/zstring.h 				\
		    vm/zarray.h 				\
		    vm/ztable.h vm/ztable.cc 			\
		    vm/zoevm.h vm/zoevm.cc 			\
		    vm/opcode.h 				\
		    compiler/bytecode.h compiler/bytecode.cc 	\
		    compiler/literals.h
#libzoe_la_LDFLAGS = -version-info 0:0:0

bin_PROGRAMS = zoe
zoe_SOURCES = exe/main.cc $(libzoe_la_SOURCES)
#zoe_CXXFLAGS = $(AM_CXXFLAGS)

EXTRA_DIST = build/zoe.supp

#
# tests
#
TESTS = check_zoe
check_PROGRAMS = check_zoe
check_zoe_SOURCES = tests/tests.cc $(libzoe_la_SOURCES)
check_zoe_CXXFLAGS = $(AM_CXXFLAGS)
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh

# 
# custom targets
#
if CLOC
cloc: clean
	cloc --exclude-dir=old configure.ac Makefile.am $(zoe_SOURCES) tests/tests.cc
endif

if CPPLINT
CPPLINT_FILTERS ?= -legal,-build/include,-whitespace,-readability/namespace,-build/header_guard,-build/namespaces,-readability/todo,-build/c++11,-runtime/references
lint: clean
	cpplint --filter=$(CPPLINT_FILTERS) --linelength=120 $(zoe_SOURCES) tests/tests.cc
endif

if COV
coverage:
	gcov $(zoe_SOURCES)
endif

@VALGRIND_CHECK_RULES@
VALGRIND_SUPPRESSIONS_FILES = build/zoe.supp

check-leaks: check_zoe
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=build/zoe.supp ./check_zoe

gen-suppressions: check_zoe
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-limit=no --gen-suppressions=all --log-file=build/zoe.supp ./check_zoe
	sed -i -e '/^==.*$$/d' build/zoe.supp

.PHONY: cloc cpplint coverage

#
# clean
#
CLEANFILES = *.log *.gcov **/*.gcov **/*.gcda **/*.gcno

# 
# variables
#
ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = 
AM_LDFLAGS = -lm

# gcc specific warnings
if GCC
  AM_CXXFLAGS += -Wunsafe-loop-optimizations -Wzero-as-null-pointer-constant -Wuseless-cast 
endif

# debugging flags
if DEBUG
  AM_CXXFLAGS += -Wfatal-errors -Wextra -Wall -Wcast-align -Wcast-qual 		\
	        -Wchar-subscripts -Wcomment -Wdisabled-optimization 		\
	        -Wfloat-equal -Wformat -Wformat=2 -Wformat-nonliteral 		\
	        -Wformat-security -Wformat-y2k -Wimport -Winit-self 		\
	        -Winvalid-pch -Wmissing-braces -Wmissing-field-initializers 	\
	        -Wmissing-format-attribute -Wmissing-include-dirs 		\
	        -Wmissing-noreturn -Wpacked -Wparentheses -Wpointer-arith 	\
	        -Wredundant-decls -Wreturn-type -Wsequence-point 		\
	        -Wsign-compare -Wstack-protector -Wstrict-aliasing 		\
	        -Wstrict-aliasing=2 -Wswitch -Wtrigraphs -Wuninitialized	\
	        -Wunknown-pragmas -Wunreachable-code -Wunused 			\
	        -Wunused-function -Wunused-label -Wunused-parameter 		\
	        -Wunused-value -Wunused-variable -Wvariadic-macros 		\
	        -Wvolatile-register-var -Wwrite-strings -Winvalid-pch 		\
		-Wconversion -Weffc++ -Wold-style-cast				\
	        -Winline -Wswitch-enum -Wmissing-declarations
  AM_CXXFLAGS += -g -ggdb3 -O0 -DDEBUG -fno-inline-functions 
  AM_LDFLAGS += -g
if GCC
  AM_CXXFLAGS += -fno-inline-small-functions
endif
else
  AM_CXXFLAGS += -DNDEBUG -Ofast -fomit-frame-pointer -ffast-math -mfpmath=sse -fPIC -msse -msse2 -msse3 -mssse3 -msse4 -flto
  AM_LDFLAGS += -flto
endif

# gold flags
if GOLD
  AM_LDFLAGS += -fuse-ld=gold
endif

# profiler flags
if PROFILE
  AM_CXXFLAGS += -pg
  AM_LDFLAGS += -pg
endif

# coverage flags
if COV
  AM_CXXFLAGS += -fprofile-arcs -ftest-coverage
  AM_LDFLAGS += -lgcov --coverage
endif
