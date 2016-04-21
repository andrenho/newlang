# prelude
AC_INIT([zoe], [0.3.0], [https://github.com/andrenho/zoe/issues])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror dist-bzip2])

# check for programs
: ${CXXFLAGS=""}             	    # remove default silly CXXFLAGS
AC_PROG_CXX([g++ clang++])
AC_PROG_LEX
AC_PROG_YACC

AC_CHECK_PROGS([gold], [ld.gold])      # look for gold linker (instead of ld)
AM_CONDITIONAL([GOLD], [test "$gold" = ld.gold])

AC_CHECK_PROGS([cloc], [cloc], [false])   # look for cpplint
if test "$cloc" = false; then
  AC_MSG_WARN(['cloc' was not found in the path. The target 'cloc' won't be built.'])
fi
AM_CONDITIONAL([CLOC], [test "$cloc" = cloc])

AC_CHECK_PROGS([cpplint], [cpplint], [false])   # look for cpplint
if test "$cpplint" = false; then
  AC_MSG_WARN(['cpplint' was not found in the path. The target 'cpplint' won't be built.'])
fi
AM_CONDITIONAL([CPPLINT], [test "$cpplint" = cpplint])

# check for libraries
AX_LIB_READLINE

# check for header files

# check for typedefs, structures and compiler characteristics
AX_CXX_COMPILE_STDCXX_14([noext], [mandatory])
AM_CONDITIONAL([GCC], [test "$GXX" = yes])

# check for library funtions

# check for conditional options
AC_ARG_ENABLE([debug], 
	      AS_HELP_STRING([--enable-debug], [Enable debugging info]),
	      [debug=yes])
AM_CONDITIONAL([DEBUG], [test "$debug" = yes])
# TODO - profile, coverage

# output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
