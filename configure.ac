# -*- Autoconf -*-
AC_PREREQ([2.69])
AC_INIT([Qwertickle], [0.2.0], [aitjcize@gmail.com], [qwertickle])
AM_INIT_AUTOMAKE([foreign -Wall])
AC_CONFIG_SRCDIR([src/qwertickle.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_INSTALL

PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([QWERTICKLE], [
  gtk+-3.0
  ayatana-appindicator3-0.1
  gstreamer-1.0
])

AC_SUBST([QWERTICKLE_CFLAGS])
AC_SUBST([QWERTICKLE_LIBS])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  data/Makefile
  data/$PACKAGE.desktop
])
AC_OUTPUT
