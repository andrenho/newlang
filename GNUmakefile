include build/version.mk
include build/config.mk

# 
# VARIABLES
#
SRC_EXE = src/main.c	\
          src/options.c	\
          src/repl.c

SRC_LIB = lib/zoe.c	\
	  lib/lexer.c	\
	  lib/parser.c

OBJ_EXE = ${SRC_EXE:.c=.o}
OBJ_LIB = ${SRC_LIB:.c=.o}

DIST = COPYING INSTALL README.md GNUmakefile build/config.mk \
       build/version.mk zoe.1

CPPFLAGS += -DVERSION=\"${VERSION}\" -D_GNU_SOURCE @build/warnings.txt -Ilib -Isrc -std=c11 -march=native

ifdef DEBUG
  CPPFLAGS += -g -ggdb3 -O0 -DDEBUG
  LDFLAGS += -g
else
  # TODO - O2 or O3?
  CPPFLAGS += -O2 -flto
  LDFLAGS += -flto
endif

# libraries
LDFLAGS +=

#
# COMPILATION RULES
#

all: libzoe.so.${VERSION} zoe

# relax warnings in generation of lexer/parser C units
lib/lexer.o: lib/lexer.c
	${CC} -g -c -fPIC -I. -o $@ $<

lib/parser.o: lib/parser.c
	${CC} -g -c -fPIC -I. -o $@ $<

%.o: %.c
	${CC} -c -fPIC ${CPPFLAGS} -o $@ $<

.l.c: 
	flex $<

.y.c:
	bison $<

zoe: depend ${OBJ_EXE} libzoe.so.${VERSION}
	${CC} -o $@ ${OBJ_EXE} libzoe.so.${VERSION} -Wl,-rpath,. ${LDFLAGS} -lreadline
-include depend

libzoe.so.${VERSION}: ${OBJ_LIB}
	${CC} -shared -Wl,-soname,libzoe.so.0 -o $@ $? ${LDFLAGS}
	ln -s libzoe.so.${VERSION} libzoe.so.0

lib/lexer.c: lib/lexer.l lib/parser.h

lib/parser.c: lib/parser.y

lib/parser.h: lib/parser.c

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
	rm -f lib/lexer.c lib/lexer.h lib/parser.h lib/parser.c

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
