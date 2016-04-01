include build/version.mk
include build/config.mk

# 
# VARIABLES
#

SRC_PARSER=parser/bytecode.c	\
           parser/lex.yy.c	\
	   parser/parser.tab.c	

SRC_VM=vm/zvalue.c

SRC_EXE=exe/main.c		\
	exe/options.c		\
	exe/repl.c

SRC_TST=tests/tests.c

OBJ_PARSER=${SRC_PARSER:.c=.o}
OBJ_VM=${SRC_VM:.c=.o}
OBJ_EXE=${SRC_EXE:.c=.o}
OBJ_TST=${SRC_TST:.c=.o}

HEADERS=$(filter-out exe/main.h,${SRC_EXE:.c=.h}) ${SRC_PARSER:.c=.h} ${SRC_VM:.c=.h}

DIST=COPYING INSTALL README.md Makefile build/config.mk \
     build/version.mk build/WARNINGS zoe.1 $(wildcard tests/*)

CPPFLAGS+=-DVERSION=\"${VERSION}\" -D_GNU_SOURCE -I. -std=c11 -march=native -fPIC

ifdef DEBUG
  CPPFLAGS+=-g -ggdb3 -O0 -DDEBUG
  LDFLAGS+=-g
  BISON_FLAGS+=--debug
  FLEX_FLAGS+=--debug
else
  CPPFLAGS+=-Ofast -fomit-frame-pointer -ffast-math -mfpmath=sse -fPIC -msse -msse2 -msse3 -mssse3 -msse4 -flto
  LDFLAGS+=-flto
endif

# libraries
LDFLAGS+=-fuse-ld=gold -lm

# cpplint filters
CPPLINT_FILTERS=-legal/copyright,-whitespace,-build/header_guard,-readability/todo,-runtime/printf,-readability/casting

#
# ALL 
#

all: libzoe.so.${VERSION} zoe

#
# LEXER AND PARSER
#

# relax warnings in generation of lexer/parser C units
parser/lex.yy.o: parser/lex.yy.c parser/parser.tab.c
	${CC} ${CPPFLAGS} -fPIC -c -I. -o $@ $<

parser/parser.tab.o: parser/parser.tab.c parser/lex.yy.c
	${CC} ${CPPFLAGS} -fPIC -c -I. -o $@ $<

parser/lex.yy.c: parser/lexer.l
	flex --header-file=parser/lex.yy.h ${FLEX_FLAGS} -o $@ $<

parser/parser.tab.c: parser/parser.y
	bison -v -Wall -d ${BISON_FLAGS} -o $@ $<

parser/lex.yy.h:

parser/parser.tab.h:

#
# COMPILATION RULES
#

-include depend

%.o: %.c
	${CC} -c ${CPPFLAGS} @build/WARNINGS -o $@ $<

zoe: depend ${OBJ_EXE} ${OBJ_VM} ${OBJ_PARSER}
	${CC} -o $@ ${OBJ_EXE} ${OBJ_VM} ${OBJ_PARSER} ${LDFLAGS} -lreadline

libzoe.so.${VERSION}: ${OBJ_VM} ${OBJ_PARSER}
	${CC} -shared -fPIC -Wl,-soname,libzoe.so.0 -o $@ $? ${LDFLAGS}

depend: ${HEADERS} ${SRC_PARSER} ${SRC_VM} ${SRC_EXE} ${SRC_TST}
	@echo checking dependencies
	@${CC} -MM ${CPPFLAGS} ${SRC_PARSER} ${SRC_PARSER:.c=.h} | sed -re 's/^([^ ])/parser\/\1/' > depend
	@${CC} -MM ${CPPFLAGS} ${SRC_VM} ${SRC_VM:.c=.h} | sed -re 's/^([^ ])/vm\/\1/' >> depend
	@${CC} -MM ${CPPFLAGS} ${SRC_EXE} $(filter-out exe/main.h,${SRC_EXE:.c=.h}) | sed -re 's/^([^ ])/exe\/\1/' >> depend
	@${CC} -MM ${CPPFLAGS} ${SRC_TST} | sed -re 's/^([^ ])/tests\/\1/' >> depend

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
	rm -f zoe libzoe.so.0 libzoe.so.${VERSION} runtests **/*.o

distclean:
	${MAKE} clean
	rm -f depend

maintainer-clean:
	${MAKE} distclean
	rm -f parser/lex.yy.c parser/lex.yy.h parser/parser.tab.h parser/parser.tab.c

# 
# PACKAGING RULES
#

dist: distclean
	mkdir -p zoe-${VERSION}
	cp --parents ${DIST} ${SRC_VM} ${SRC_PARSER} ${SRC_EXE} ${HEADERS} zoe-${VERSION}
	tar cf zoe-${VERSION}.tar zoe-${VERSION}
	bzip2 -f zoe-${VERSION}.tar
	rm -rf zoe-${VERSION} zoe-${VERSION}.tar

# 
# TESTS
#

cloc:
	cloc --exclude-dir=old .

runtests: ${OBJ_VM} ${OBJ_PARSER} ${OBJ_TST}
	${CC} -o runtests ${OBJ_TST} ${OBJ_VM} ${OBJ_PARSER} ${LDFLAGS}

check: runtests
	./runtests

lint:
	${MAKE} maintainer-clean
	cpplint --filter=${CPPLINT_FILTERS} --linelength=120 **/*.h **/*.c

splint: 
	${MAKE} maintainer-clean
	splint -badflag ${CPPFLAGS} **/*.h **/*.c

check-leaks: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=build/zoe.supp ./zoe -D

gen-suppressions: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-limit=no --gen-suppressions=all --log-file=build/zoe.supp ./zoe
	sed -i -e '/^==.*$$/d' build/zoe.supp


.PHONY: all options clean distclean maintainer-clean dist uninstall install install-strip check-leaks depend
