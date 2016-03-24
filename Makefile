include build/version.mk
include build/config.mk

# 
# VARIABLES
#

SRC_LIB=lib/zoe.cc

SRC_EXE=src/main.cc	\
	src/options.cc	\
	src/repl.cc

OBJ_LIB=${SRC_LIB:.cc=.o}
OBJ_EXE=${SRC_EXE:.cc=.o}

DIST=COPYING INSTALL README.md Makefile build/config.mk \
     build/version.mk zoe.1

CPPFLAGS+=-DVERSION=\"${VERSION}\" -D_GNU_SOURCE @build/WARNINGS -Ilib -Isrc -std=c++11 -march=native -fPIC

ifdef DEBUG
  OPT_CPPFLAGS=-g -ggdb3 -O0 -DDEBUG
  OPT_LDFLAGS=-g
else
  # TODO - O2 or O3?
  OPT_CPPFLAGS+=-O2 -flto
  OPT_LDFLAGS+=-flto
endif

# libraries
LDFLAGS +=

#
# ALL 
#

all: libzoe.so.${VERSION} zoe

#
# COMPILATION RULES
#

%.o: %.cc
	${CPP} -c ${CPPFLAGS} ${OPT_CPPFLAGS} -o $@ $<

zoe: depend ${OBJ_EXE}
	${CPP} -o $@ ${OBJ_EXE} ${LDFLAGS} ${OPT_LDFLAGS}
-include depend

libzoe.so.${VERSION}: ${OBJ_LIB}
	${CPP} -shared -Wl,-soname,libzoe.so.0 -o $@ $? ${LDFLAGS} ${OPT_LDFLAGS}

# TODO
#depend: ${SRC_EXE} ${SRC_LIB}
#	@echo checking dependencies
#	@${CPP} -MM ${CPPFLAGS} ${SRC} >depend

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
	rm -f zoe libzoe.so.0 libzoe.so.${VERSION} *.o src/*.o lib/*.o

distclean:
	${MAKE} clean
	rm -f depend

maintainer-clean:
	${MAKE} distclean
	rm -f lib/lex.yy.c lib/lex.yy.h lib/parser.tab.h lib/parser.tab.c

# 
# PACKAGING RULES
#

dist: distclean
	mkdir -p zoe-${VERSION}
	cp --parents ${DIST} ${SRC} zoe-${VERSION}
	tar cf zoe-${VERSION}.tar zoe-${VERSION}
	bzip2 -f zoe-${VERSION}.tar
	rm -rf zoe-${VERSION} zoe-${VERSION}.tar

# 
# TESTS
#

check-leaks: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=build/zoe.supp ./zoe

gen-suppressions: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-limit=no --gen-suppressions=all --log-file=build/zoe.supp ./zoe
	sed -i -e '/^==.*$$/d' build/zoe.supp


.PHONY: all options clean distclean maintainer-clean dist uninstall install install-strip check-leaks depend
