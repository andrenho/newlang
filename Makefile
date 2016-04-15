include build/version.mk
include build/config.mk

# 
# VARIABLES
#

SRC_LIB=lib/zoe.cc		\
	lib/bytecode.cc		\
	lib/lexer.cc		\
	lib/parser.cc 		\
	lib/zvalue.cc

SRC_EXE=src/main.cc		\
	src/options.cc		\
	src/repl.cc

SRC_TST=tests/tests.cc

OBJ_LIB=${SRC_LIB:.cc=.o}
OBJ_EXE=${SRC_EXE:.cc=.o}
OBJ_TST=${SRC_TST:.cc=.o}

HEADERS=$(filter-out src/main.h,${SRC_EXE:.cc=.h}) ${SRC_LIB:.cc=.h} lib/opcode.h

DIST=COPYING INSTALL README.md Makefile build/config.mk HACKING \
     build/version.mk build/WARNINGS zoe.1 $(wildcard tests/*)  \
     lib/lexer.l lib/parser.y doc/zoe.html

CPPFLAGS+=-DVERSION=\"${VERSION}\" -D_GNU_SOURCE -I. -std=c++14 -march=native -fPIC

ifdef PROFILE
  CPPFLAGS+=-pg
  LDFLAGS+=-pg
endif

ifdef COV
  CPPFLAGS+=-fprofile-arcs -ftest-coverage
  LDFLAGS+=-lgcov --coverage
endif

ifdef DEBUG
  CPPFLAGS+=-g -ggdb3 -O0 -DDEBUG -fno-inline-functions -fno-inline-small-functions
  LDFLAGS+=-g
  BISON_FLAGS+=--debug
  FLEX_FLAGS+=--debug
else
  CPPFLAGS+=-DNDEBUG -Ofast -fomit-frame-pointer -ffast-math -mfpmath=sse -fPIC -msse -msse2 -msse3 -mssse3 -msse4 -flto
  LDFLAGS+=-flto
endif

# libraries
LDFLAGS+=-fuse-ld=gold -lm -lreadline

# cpplint filters
CPPLINT_FILTERS=-legal,-build/include,-whitespace,-readability/namespace,-build/header_guard,-build/namespaces,-readability/todo,-build/c++11,-runtime/references

#
# ALL 
#

all: depend libzoe.so.${VERSION} zoe

#
# LEXER AND PARSER
#

# relax warnings in generation of lexer/parser C units
lib/lexer.o: lib/lexer.cc lib/parser.cc
	${CXX} ${CPPFLAGS} -fPIC -c -I. -Ilib -o $@ $<

lib/parser.o: lib/parser.cc lib/lexer.cc
	${CXX} ${CPPFLAGS} -fPIC -c -I. -Ilib -o $@ $<

lib/lexer.cc: lib/lexer.l
	flex --header-file=lib/lexer.h ${FLEX_FLAGS} -o $@ $<

lib/parser.cc: lib/parser.y
	bison -v -Wall -d ${BISON_FLAGS} -o $@ $<

lib/lexer.h:

lib/parser.h:

#
# COMPILATION RULES
#

-include depends

%.o: %.cc
	${CXX} -c ${CPPFLAGS} @build/WARNINGS -o $@ $<

zoe: ${OBJ_EXE} ${OBJ_LIB}
	${CXX} -o $@ ${OBJ_EXE} ${OBJ_LIB} ${LDFLAGS}

libzoe.so.${VERSION}: ${OBJ_LIB}
	${CXX} -shared -fPIC -Wl,-soname,libzoe.so.0 -o $@ $? ${LDFLAGS}

depend: ${HEADERS} ${SRC_LIB} ${SRC_EXE} ${SRC_TST}
	@echo checking dependencies
	@${CXX} -MM ${CPPFLAGS} ${SRC_LIB} ${SRC_LIB:.cc=.h} | sed -re 's/^([^ ])/lib\/\1/' > depends
	@${CXX} -MM ${CPPFLAGS} ${SRC_EXE} $(filter-out src/main.h,${SRC_EXE:.cc=.h}) | sed -re 's/^([^ ])/src\/\1/' >> depends
	@${CXX} -MM ${CPPFLAGS} ${SRC_TST} | sed -re 's/^([^ ])/tests\/\1/' >> depends

# 
# DESCRIBE VARIABLES
#

options:
	@echo "zoe configuration options"
	@echo "CPPFLAGS = ${CPPFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CXX      = ${CXX}"

#
# INSTALATION RULES
#

install: all
	install -p zoe ${PREFIX}/bin
	install -p libzoe.so.${VERSION} ${PREFIX}/lib
	ln -f -s ${PREFIX}/lib/libzoe.so.${VERSION} ${PREFIX}/lib/libzoe.so.0
	mkdir -p ${MANPREFIX}/man1
	cp zoe.1 ${MANPREFIX}/man1

uninstall:
	rm -f ${PREFIX}/bin/zoe
	rm -f ${PREFIX}/lib/libzoe.so.${VERSION}
	rm -f ${PREFIX}/lib/libzoe.so.0
	rm -f ${MANPREFIX}/man1/zoe.1

install-strip:
	${MAKE} install
	strip ${PREFIX}/bin/zoe

#
# CLEANING RULES
#

clean:
	rm -f zoe libzoe.so.0 libzoe.so.${VERSION} runtests **/*.o gmon.out **/*.gcda **/*.gcno **/*.gcov *.gcov lib/parser.output

distclean:
	${MAKE} clean
	rm -f depends

maintainer-clean:
	${MAKE} distclean
	rm -f lib/lexer.cc lib/lexer.h lib/parser.h lib/parser.cc

# 
# PACKAGING RULES
#

dist: distclean
	mkdir -p zoe-${VERSION}
	cp --parents ${DIST} ${SRC_LIB} ${SRC_EXE} ${HEADERS} zoe-${VERSION}
	tar cf zoe-${VERSION}.tar zoe-${VERSION}
	bzip2 -f zoe-${VERSION}.tar
	rm -rf zoe-${VERSION} zoe-${VERSION}.tar

# 
# TESTS
#

cloc: maintainer-clean
	cloc --exclude-dir=old .

runtests: ${OBJ_LIB} ${OBJ_TST} zoe
	${CXX} -o runtests ${OBJ_TST} ${OBJ_LIB} ${LDFLAGS}

check: runtests
	./runtests

lint:
	${MAKE} maintainer-clean
	cpplint --filter=${CPPLINT_FILTERS} --linelength=120 lib/*.h lib/*.cc src/*.h src/*.cc tests/*.cc
	@echo -------------------------------------------------
	cppcheck --std=c++11 --enable=all --inconclusive lib/*.h lib/*.cc src/*.h src/*.cc tests/*.cc

check-leaks: zoe
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=build/zoe.supp ./zoe -D

check-leaks-check: runtests
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=build/zoe.supp ./runtests -D

coverage:
	${MAKE} clean
	${MAKE} check COV=1
	gcov ${SRC_LIB}

cachegrind:
	valgrind --tool=cachegrind ./runtests

gen-suppressions: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-limit=no --gen-suppressions=all --log-file=build/zoe.supp ./zoe
	sed -i -e '/^==.*$$/d' build/zoe.supp


.PHONY: all options clean distclean maintainer-clean dist uninstall install install-strip check-leaks depend
