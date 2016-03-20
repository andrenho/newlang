# Minimal autoconf version required
AC_PREREQ(2.69)

# Initialize autoconf
AC_INIT([zoe],		dnl Package name
	[0.0.1],	dnl Version (TODO)
	[https://github.com/andrenho/zoe/issues],  dnl Contact for issues
	[],		dnl Tarname (?)
	[https://github.com/andrenho/zoe])
AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(config.h)

# Initialize automake
AM_INIT_AUTOMAKE([1.15])

# Check for necessary programs
AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC
AX_VALGRIND_CHECK

# Check for libraries

# Check for header files
AC_HEADER_STDC

# Check for C11 support
AX_CHECK_COMPILE_FLAG([-std=c11], [
  CFLAGS+=" -std=c11"
], [
  echo "C compiler without C11 support"
  exit -1
])

# Create config files
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
