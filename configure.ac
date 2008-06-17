# configure.ac for libdivsufsort

AC_PREREQ(2.59)

AC_INIT([libdivsufsort], [1.2.3], [yiv01157@nifty.com])
AC_CONFIG_SRCDIR([include/divsufsort.h.in])
AC_CONFIG_HEADER([include/config.h])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([-Wall -Werror foreign 1.9 no-define dist-bzip2])

# Library code modified:                              REVISION++
# Interfaces changed/added/removed:   CURRENT++       REVISION=0
# Interfaces added:                             AGE++
# Interfaces removed:                           AGE=0
AC_SUBST(LT_CURRENT, 3)
AC_SUBST(LT_REVISION, 2)
AC_SUBST(LT_AGE, 1)

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Check for host type.
AC_CANONICAL_HOST

# Checks for compiler output filename suffixes.
AC_OBJEXT
AC_EXEEXT

# Check for build configuration.
AM_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
case "$host_os" in
  cygwin*)
    AC_ARG_ENABLE(cygwin, AC_HELP_STRING([--disable-cygwin], [Don't use the CygWin libraries]))
    if test "$enable_cygwin" != "no"; then
      A_CFLAGS=''
    else
      A_CFLAGS='-mno-cygwin'
    fi
    LT_NOUNDEF='-no-undefined'
    ;;
  *) LT_NOUNDEF='' A_CFLAGS='' ;;
esac
AC_SUBST([LT_NOUNDEF])
AC_SUBST([A_CFLAGS])

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([inttypes.h memory.h stddef.h stdint.h stdlib.h string.h strings.h])
AC_CHECK_HEADER([inttypes.h], [AC_SUBST([_HAVE_INTTYPES_H_], [1])], [AC_SUBST([_HAVE_INTTYPES_H_], [0])])
AC_CHECK_HEADER([stdint.h], [AC_SUBST([_HAVE_STDINT_H_], [1])], [AC_SUBST([_HAVE_STDINT_H_], [0])])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STAT

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 include/divsufsort.h
                 include/divsufsort_private.h
                 lib/Makefile
                 examples/Makefile])
AC_OUTPUT
