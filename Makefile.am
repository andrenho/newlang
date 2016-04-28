#
# programs to build
#
#lib_LTLIBRARIES = libzoe.la
libzoe_la_SOURCES = vm/ztype.hh vm/ztype.cc			\
		    vm/zvalue.hh 				\
		    vm/znil.hh 					\
		    vm/zbool.hh vm/zbool.cc			\
		    vm/znumber.hh vm/znumber.cc			\
		    vm/zstring.hh vm/zstring.cc			\
		    vm/zarray.hh vm/zarray.cc			\
		    vm/ztable.hh vm/ztable.cc 			\
		    vm/zoevm.hh vm/zoevm.cc 			\
		    vm/exceptions.hh                            \
		    vm/opcode.hh 				\
		    compiler/bytecode.hh compiler/bytecode.cc 	\
		    compiler/literals.hh			\
		    compiler/lexer.ll compiler/lexer.hh		\
		    compiler/parser.yy
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
#check_zoe_CXXFLAGS = $(AM_CXXFLAGS)
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh

# 
# custom targets
#
if CLOC
cloc:
	cloc --exclude-dir=old --skip-archive=compiler/lexer.hh configure.ac Makefile.am $(zoe_SOURCES) tests/tests.cc
endif

if CLANG_TIDY
lint: 
	clang-tidy $(filter-out compiler/lexer.ll compiler/lexer.hh compiler/parser.yy, $(zoe_SOURCES)) tests/tests.cc \
		"-checks=*,-google-build-using-namespace,-google-readability-todo" -- -I. --std=c++14
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
DISTCLEANFILES = zoe-*.tar.*

# 
# flags
#
ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = 
AM_LDFLAGS = -lm

AM_LFLAGS = --header-file=../compiler/lexer.hh    # this hack is necessary because, by default, yylwrap build the header file in the temporary aux directory
AM_YFLAGS = -d
BUILT_SOURCES = compiler/lexer.hh compiler/parser.hh

compiler/lexer.hh: compiler/lexer.ll

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
