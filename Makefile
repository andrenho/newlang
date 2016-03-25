include build/version.mk
include build/config.mk

# 
# VARIABLES
#

SRC_LIB=lib/zoe.cc		\
	lib/bytecode.cc		\
	lib/lex.yy.cc		\
	lib/parser.tab.cc

SRC_EXE=src/main.cc		\
	src/options.cc		\
	src/repl.cc

SRC_TST=tests/main.cc		\
	tests/zoe.cc		\
	tests/bytecode.cc

OBJ_LIB=${SRC_LIB:.cc=.o}
OBJ_EXE=${SRC_EXE:.cc=.o}
OBJ_TST=${SRC_TST:.cc=.o}

HEADERS=$(filter-out src/main.h,${SRC_EXE:.cc=.h}) ${SRC_LIB:.cc=.h}

DIST=COPYING INSTALL README.md Makefile build/config.mk \
     build/version.mk build/WARNINGS zoe.1 $(wildcard tests/*)

CPPFLAGS+=-DVERSION=\"${VERSION}\" -D_GNU_SOURCE -Ilib -Isrc -std=c++14 -march=native -fPIC

ifdef DEBUG
  CPPFLAGS+=-g -ggdb3 -O0 -DDEBUG
  LDFLAGS+=-g
else
  CPPFLAGS+=-Ofast -fomit-frame-pointer -ffast-math -mfpmath=sse -fPIC -msse -msse2 -msse3 -mssse3 -msse4 -flto
  LDFLAGS+=-flto
endif

# libraries
LDFLAGS+=-fuse-ld=gold

# filter for cpplint
LINT_FILTERS = -legal,-build/include,-whitespace,-readability/namespace,-build/header_guard,-build/namespaces,-readability/todo,-build/c++11

#
# ALL 
#

all: libzoe.so.${VERSION} zoe

#
# LEXER AND PARSER
#

# relax warnings in generation of lexer/parser C units
lib/lex.yy.o: lib/lex.yy.cc lib/parser.tab.cc
	${CPP} ${CPPFLAGS} -fPIC -c -I. -Ilib -o $@ $<

lib/parser.tab.o: lib/parser.tab.cc lib/lex.yy.cc
	${CPP} ${CPPFLAGS} -fPIC -c -I. -Ilib -o $@ $<

lib/lex.yy.cc: lib/lexer.l
	flex --header-file=lib/lex.yy.h -o $@ $<

lib/parser.tab.cc: lib/parser.y
	bison -d -o $@ $<

lib/lex.yy.h:

lib/parser.tab.h:

#
# COMPILATION RULES
#

-include depend

%.o: %.cc
	${CPP} -c ${CPPFLAGS} @build/WARNINGS -o $@ $<

zoe: depend ${OBJ_EXE} ${OBJ_LIB}
	${CPP} -o $@ ${OBJ_EXE} ${OBJ_LIB} ${LDFLAGS}

libzoe.so.${VERSION}: ${OBJ_LIB}
	${CPP} -shared -Wl,-soname,libzoe.so.0 -o $@ $? ${LDFLAGS}

depend: ${HEADERS} ${SRC_LIB} ${SRC_EXE} ${SRC_TST}
	@echo checking dependencies
	@${CPP} -MM ${CPPFLAGS} ${SRC_LIB} ${SRC_LIB:.cc=.h} | sed -re 's/^([^ ])/lib\/\1/' > depend
	@${CPP} -MM ${CPPFLAGS} ${SRC_EXE} $(filter-out src/main.h,${SRC_EXE:.cc=.h}) | sed -re 's/^([^ ])/lib\/\1/' >> depend
	@${CPP} -MM ${CPPFLAGS} ${SRC_TST} | sed -re 's/^([^ ])/lib\/\1/' >> depend

# 
# DESCRIBE VARIABLES
#

options:
	@echo "zoe configuration options"
	@echo "CPPFLAGS = ${CPPFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CPP      = ${CPP}"

#
# INSTALATION RULES
#

install: all
	install -p zoe ${PREFIX}/bin
	install -p libzoe.so.${VERSION} ${PREFIX}/lib
	ln -s ${PREFIX}/lib/libzoe.so.${VERSION} ${PREFIX}/lib/libzoe.so.0
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
	rm -f zoe libzoe.so.0 libzoe.so.${VERSION} unittests **/*.o

distclean:
	${MAKE} clean
	rm -f depend

maintainer-clean:
	${MAKE} distclean
	rm -f lib/lex.yy.cc lib/lex.yy.h lib/parser.tab.h lib/parser.tab.cc

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

unittests: ${OBJ_LIB} ${OBJ_TST}
	${CPP} -o unittests ${OBJ_TST} ${OBJ_LIB} ${LDFLAGS}

check: unittests
	./unittests

lint: 
	${MAKE} maintainer-clean
	cpplint --filter=${LINT_FILTERS} --linelength=120 lib/*.h lib/*.cc src/*.h src/*.cc

check-leaks: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=build/zoe.supp ./zoe

gen-suppressions: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-limit=no --gen-suppressions=all --log-file=build/zoe.supp ./zoe
	sed -i -e '/^==.*$$/d' build/zoe.supp


.PHONY: all options clean distclean maintainer-clean dist uninstall install install-strip check-leaks depend
