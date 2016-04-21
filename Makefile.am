# 
# custom targets
#
if CLOC
cloc: clean
	cloc configure.ac Makefile.am src/*
endif

if CPPLINT
CPPLINT_FILTERS=-legal,-build/include,-whitespace,-readability/namespace,-build/header_guard,-build/namespaces,-readability/todo,-build/c++11,-runtime/references
lint: clean
	cpplint --filter=${CPPLINT_FILTERS} --linelength=120 lib/*.h lib/*.cc src/*.h src/*.cc tests/*.cc
endif

#
# subdirs
#
SUBDIRS = src
