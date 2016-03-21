include build/version.mk
include build/config.mk

# 
# VARIABLES
#
SRC = src/main.c	\
      src/compiler.c	\
      src/vm.c		\
      src/options.c	\
      src/repl.c	\
      src/lexer.c	\
      src/parser.c

HEADERS = $(filter-out src/main.h, ${SRC:.c=.h})

OBJ = ${SRC:.c=.o}
DIST = COPYING INSTALL README.md GNUmakefile build/config.mk \
       build/version.mk zoe.1

CPPFLAGS += -DVERSION=\"${VERSION}\" -D_GNU_SOURCE @build/warnings.txt -Isrc -std=c11 -march=native

ifdef DEBUG
  CPPFLAGS += -g -ggdb3 -O0 -DDEBUG
  LDFLAGS += -g
else
  # TODO - O2 or O3?
  CPPFLAGS += -O2 -flto
  LDFLAGS += -flto
endif

# libraries
LDFLAGS += -lreadline

#
# COMPILATION RULES
#

all: zoe

# relax warnings in generation of lexer/parser C units
src/lexer.o: src/lexer.c
	${CC} -c -o $@ $<

src/parser.o: src/parser.c
	${CC} -c -o $@ $<

%.o: %.c
	${CC} -c ${CPPFLAGS} -o $@ $<

.l.c:
	flex $<

.y.c:
	bison $<

zoe: depend ${OBJ} ${HEADERS}
	${CC} -o $@ ${OBJ} ${LDFLAGS}
-include depend

lexer.c: lexer.l

parser.c: parser.y

depend: ${SRC}
	@echo checking dependencies
	@${CC} -MM ${CPPFLAGS} ${SRC} >depend


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
	cp zoe.1 ${MANPREFIX}/man1

uninstall:
	rm -f ${PREFIX}/bin/zoe
	rm -f ${MANPREFIX}/man1/zoe.1

install-strip:
	install -ps zoe ${PREFIX}/bin
	cp zoe.1 ${MANPREFIX}/man1

#
# CLEANING RULES
#

clean:
	rm -f zoe *.o src/*.o

distclean:
	${MAKE} clean
	rm depend

maintainer-clean:
	${MAKE} distclean
	rm src/lexer.c src/lexer.h src/parser.h src/parser.c

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


.PHONY: all options clean distclean maintainer-clean dist uninstall install install-strip check-leaks
