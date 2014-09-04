#-*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.61)
#
AC_INIT([rbroker], 0.0.1)


AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config)
AC_CONFIG_HEADERS([src/platform.h])
AM_INIT_AUTOMAKE(foreign)
dnl m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# This lets us use PACKAGE_VERSION in Makefiles
AC_SUBST(PACKAGE_VERSION)

# Take a copy of original flags
MS_ORIG_CFLAGS="${CFLAGS:-none}"
MS_ORIG_CPPFLAGS="${CPPFLAGS:-none}"
MS_ORIG_CXXFLAGS="${CXXFLAGS:-none}"

CFLAGS="${CFLAGS} -D_GNU_SOURCE -g -std=gnu99"

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
AM_PROG_LIBTOOL

# should do custom m4s for this, to allow for non-pkg-config distros, but meh.
PKG_CHECK_MODULES([DAEMON], [libdaemon])
AC_SUBST([DAEMON_CFLAGS])
AC_SUBST([DAEMON_LIBS])

PKG_CHECK_MODULES([CONFIG], [libconfuse])
AC_SUBST([CONFIG_CFLAGS])
AC_SUBST([CONFIG_LIBS])

# For host type checks
AC_CANONICAL_HOST

# Checks for header files.
AC_HEADER_STDC

AC_OUTPUT(Makefile src/Makefile)
