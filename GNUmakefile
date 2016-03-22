include build/version.mk
include build/config.mk

# 
# VARIABLES
#
SRC_LIB = lib/zoe.c#		\
	  lib/lex.yy.c		\
	  lib/parser.tab.c

SRC_EXE = src/main.c		\
          src/options.c		\
	  ${SRC_LIB}

OBJ_LIB = ${SRC_LIB:.c=.o}
OBJ_EXE = ${SRC_EXE:.c=.o}

DIST = COPYING INSTALL README.md GNUmakefile build/config.mk \
       build/version.mk zoe.1

CPPFLAGS += -DVERSION=\"${VERSION}\" -D_GNU_SOURCE @build/warnings.txt -Ilib -Isrc -std=c11 -march=native -fPIC

ifdef DEBUG
  OPT_CPPFLAGS = -g -ggdb3 -O0 -DDEBUG
  OPT_LDFLAGS = -g
else
  # TODO - O2 or O3?
  OPT_CPPFLAGS += -O2 -flto
  OPT_LDFLAGS += -flto
endif

# libraries
LDFLAGS +=

#
# ALL 
#

all: libzoe.so.${VERSION} zoe

#
# LEXER AND PARSER
#

# relax warnings in generation of lexer/parser C units
lib/lex.yy.o: lib/lex.yy.c
	${CC} ${OPT_CPPFLAGS} -c -I. -o $@ $<

lib/parser.tab.o: lib/parser.tab.c
	${CC} ${OPT_CPPFLAGS} -c -I. -o $@ $<

.l.c: 
	flex --header-file=lib/lex.yy.h $<

.y.c:
	bison -d $<

lib/lex.yy.c: lib/lex.yy.l

lib/parser.tab.c: lib/parser.tab.y

#
# COMPILATION RULES
#

%.o: %.c
	${CC} -c ${CPPFLAGS} ${OPT_CPPFLAGS} -o $@ $<

zoe: depend ${OBJ_EXE} libzoe.so.${VERSION}
	${CC} -o $@ ${OBJ_EXE} libzoe.so.${VERSION} -Wl,-rpath,. ${LDFLAGS} ${OPT_LDFLAGS}
-include depend

libzoe.so.${VERSION}: ${OBJ_LIB}
	${CC} -shared -Wl,-soname,libzoe.so.0 -o $@ $? ${LDFLAGS} ${OPT_LDFLAGS}
	ln -s libzoe.so.${VERSION} libzoe.so.0

# TODO
#depend: ${SRC_EXE} ${SRC_LIB}
#	@echo checking dependencies
#	@${CC} -MM ${CPPFLAGS} ${SRC} >depend


# 
# DESCRIBE VARIABLES
#

options:
	@echo "zoe configuration options"
	@echo "CPPFLAGS = ${CPPFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"


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
	valgrind --leak-check=full --show-leak-kinds=all --suppressions=build/zoe.supp ./zoe

gen-suppressions: all
	valgrind --leak-check=full --show-leak-kinds=all --error-limit=no --gen-suppressions=all --log-file=build/zoe.supp ./zoe
	sed -i -e '/^==.*$$/d' build/zoe.supp


.PHONY: all options clean distclean maintainer-clean dist uninstall install install-strip check-leaks depend
