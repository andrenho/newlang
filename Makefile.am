AUTOMAKE_OPTIONS = subdir-objects foreign

INCLUDES = -I$(top_builddir)

bin_PROGRAMS = zoe
zoe_SOURCES = src/main.c
#zoe_LDADD = @LEXLIB@

@VALGRIND_CHECK_RULES@
VALGRIND_SUPPRESSION_FILES = extra/zoe.supp
EXTRA_DIST = extra/zoe.supp
