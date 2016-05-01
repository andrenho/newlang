#
# prelude
#
AC_INIT([zoe], [0.3.15-alpha], [https://github.com/andrenho/zoe/issues])
AC_CONFIG_SRCDIR([exe/main.cc])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror -Wno-portability dist-bzip2 subdir-objects])

AM_PROG_AR
LT_INIT([disable-static])

#
# check for files
#
AC_REQUIRE_AUX_FILE([tap-driver.sh])

#
# check for programs
#
: ${CXXFLAGS=""}             	    # remove default silly CXXFLAGS
AC_PROG_CXX([g++ clang++])
AC_LANG_CPLUSPLUS
AC_LANG(C++)
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AX_VALGRIND_DFLT([drd], [off])
AX_VALGRIND_DFLT([helgrind], [off])
AX_VALGRIND_CHECK

if test "x$LEX" != xflex; then
  missing=`./build-aux/missing flex`
  AC_MSG_ERROR([flex not found.])
fi

if test "x$YACC" != "xbison -y"; then
  missing=`./build-aux/missing bison`
  AC_MSG_ERROR([bison not found.])
fi

# gold
AC_CHECK_PROGS([gold], [ld.gold])      # look for gold linker (instead of ld)
AM_CONDITIONAL([GOLD], [test "$gold" = ld.gold])

# cloc
AC_CHECK_PROGS([cloc], [cloc], [false])   # look for clang-tidy
if test "$cloc" = false; then
  AC_MSG_WARN(['cloc' was not found in the path. The target 'cloc' won't be built.'])
fi
AM_CONDITIONAL([CLOC], [test "$cloc" = cloc])

# clang-tidy
AC_CHECK_PROGS([clang_tidy], [clang-tidy], [false])   # look for clang-tidy
if test "$clang_tidy" = false; then
  AC_MSG_WARN(['clang-tidy' was not found in the path. The target 'lint' won't be built.'])
fi
AM_CONDITIONAL([CLANG_TIDY], [test "$clang_tidy" = clang-tidy])

#
# check for libraries
#
AX_LIB_READLINE

#
# check for header files
#

#
# check for typedefs, structures and compiler characteristics
#
AX_CXX_COMPILE_STDCXX_14([noext], [mandatory])
AM_CONDITIONAL([GCC], [test "$CXX" = g++])
AC_CHECK_SIZEOF([double])
if test "$ac_cv_sizeof_double" != 8; then
  AC_MSG_ERROR([Sorry, zoe doesn't work (yet) in platforms where sizeof(double) != 8])
fi
AC_C_BIGENDIAN
if test x"$WORDS_BIGENDIAN" != x; then
  AC_MSG_ERROR([Sorry, zoe doesn't work (yet) on big endian platforms.])
fi

#
# check for conditional options
#

# debug
AC_ARG_ENABLE([debug], 
	      AS_HELP_STRING([--enable-debug], [Enable debugging info]),
	      [debug=yes])
AM_CONDITIONAL([DEBUG], [test "$debug" = yes])

# profile
AC_ARG_ENABLE([profile], 
	      AS_HELP_STRING([--enable-profile], [Enable generation of profile info]),
	      [profile=yes])
AM_CONDITIONAL([PROFILE], [test "$profile" = yes])

# coverage
AC_ARG_ENABLE([coverage], 
	      AS_HELP_STRING([--enable-coverage], [Enable generation of coverage info]),
	      [coverage=yes])
AM_CONDITIONAL([COV], [test "$coverage" = yes])


#
# output files
#
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
