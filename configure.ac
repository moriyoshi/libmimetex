#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libmimetex, 1.71.0, mozo@mozo.jp)
AC_CONFIG_SRCDIR([mimetex.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor memmove memset modf pow sqrt strchr strcspn strncasecmp strrchr strspn strstr strtol])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
