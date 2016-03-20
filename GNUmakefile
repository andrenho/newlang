include build/version.mk
include build/config.mk

# 
# VARIABLES
#
SRC = src/main.c

OBJ = ${SRC:.c=.o}
DIST = COPYING INSTALL README.md GNUmakefile extra/config.mk \
       extra/version.mk

CPPFLAGS += -DVERSION=\"${VERSION}\" -Isrc -std=c11
LDFLAGS +=

ifneq (${DEBUG}, no)
  CPPFLAGS += -g -O0 -DDEBUG
  LDFLAGS += -g
else
  CPPFLAGS += -O3
endif

#
# COMPILATION RULES
#

all: options zoe

options:
	@echo 
	@echo "zoe configuration options"
	@echo "CPPFLAGS = ${CPPFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CPPFLAGS} -o $@ $<

zoe: depend ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}
-include depend

depend: ${SRC}
	@echo checking dependencies
	@${CC} -MM ${CPPFLAGS} ${SRC} >depend

#
# INSTALATION RULES
#

install: all
	@echo installing executable to ${PREFIX}/bin
	@install -p zoe ${PREFIX}/bin
	@echo installing manual page to ${MANPREFIX}
	@install -p zoe.1 ${MANPREFIX}/man1

uninstall:


install-strip:


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

dist:


# 
# TESTS
#

check:



