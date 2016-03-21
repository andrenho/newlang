include build/version.mk
include build/config.mk

# 
# VARIABLES
#
SRC = src/main.c

OBJ = ${SRC:.c=.o}
DIST = COPYING INSTALL README.md GNUmakefile build/config.mk \
       build/version.mk zoe.1

CPPFLAGS += -DVERSION=\"${VERSION}\" ${WARNINGS} -Isrc -std=c11 -march=native
LDFLAGS +=

ifneq (${DEBUG}, no)
  CPPFLAGS += -g -ggdb3 -O0 -DDEBUG
  LDFLAGS += -g
else
  # TODO - O2 or O3?
  CPPFLAGS += -O2 -flto
  LDFLAGS += -flto
endif

#
# COMPILATION RULES
#

all: zoe

options:
	@echo "zoe configuration options"
	@echo "CPPFLAGS = ${CPPFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo ${CC} -c -o $@ $<
	@${CC} -c ${CPPFLAGS} -o $@ $<

zoe: depend ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}
-include depend

depend: ${SRC}
	@echo checking dependencies
	@${CC} -MM ${CPPFLAGS} ${SRC} >depend

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

check:


.PHONY: all options clean distclean maintainer-clean dist uninstall install install-strip
