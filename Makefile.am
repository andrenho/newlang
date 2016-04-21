#
# programs to build
#
bin_PROGRAMS = zoe
zoe_SOURCES = src/main.cc

EXTRA_DIST = build/zoe.supp

#
# tests
#
TESTS = check_zoe
check_PROGRAMS = check_zoe
check_zoe_SOURCES = tests/tests.cc
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh

# 
# custom targets
#
if CLOC
cloc: clean
	cloc --exclude-dir=old configure.ac Makefile.am src
endif

if CPPLINT
CPPLINT_FILTERS ?= -legal,-build/include,-whitespace,-readability/namespace,-build/header_guard,-build/namespaces,-readability/todo,-build/c++11,-runtime/references
lint: clean
	cpplint --filter=${CPPLINT_FILTERS} --linelength=120 $(zoe_SOURCES)
endif

if COV
coverage:
	gcov $(zoe_SOURCES)
endif

@VALGRIND_CHECK_RULES@
VALGRIND_SUPPRESSIONS_FILES = build/zoe.supp

.PHONY: cloc cpplint coverage

# 
# variables
#
AM_CXXFLAGS = @build/WARNINGS
AM_LDFLAGS = -lm

# gcc specific warnings
if GCC
  AM_CXXFLAGS += -Wunsafe-loop-optimizations -Wzero-as-null-pointer-constant -Wuseless-cast 
endif

# debugging flags
if DEBUG
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


