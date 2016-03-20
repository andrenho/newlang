# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# add debugging information
DEBUG = no

# compiler and linker
CC = gcc

# basic flags
WARNINGS = -Wpedantic -Wall -Wextra -Wshadow -Wstrict-overflow -fno-strict-aliasing
CPPFLAGS += -DVERSION=\"${VERSION}\" ${WARNINGS} -Isrc -std=c11
