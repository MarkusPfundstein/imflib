#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([imfgui], [0.1], [markus@odmedia.nl])
AM_INIT_AUTOMAKE([-Wall -Werror])

AC_LANG(C++)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

AC_CHECK_LIB([as02], [main], [], [AC_MSG_ERROR([libas02 was not found])])
AC_CHECK_LIB([asdcp], [main], [], [AC_MSG_ERROR([libasdcp was not found])])
AC_CHECK_LIB([boost_system], [main], [], [AC_MSG_ERROR([boost_system not installed])])

# check qt stuff
PKG_CHECK_MODULES(QT4, [QtCore QtGui >= 4.4.0], [
  AC_PATH_PROGS(MOC, [moc-qt4 moc], moc,`eval $PKG_CONFIG --variable=exec_prefix QtCore`/bin)
  AC_PATH_PROG(RCC, rcc, rcc,`eval $PKG_CONFIG --variable=exec_prefix QtCore`/bin)
  AC_PATH_PROGS(UIC, [uic-qt4 uic], uic,`eval $PKG_CONFIG --variable=exec_prefix QtCore`/bin)
], [
  AC_MSG_ERROR([Qt 4 library not found])
])

# check xerces-c
PKG_CHECK_MODULES([XERCES], [xerces-c >= 3.0], [], [AC_MSG_ERROR([Xerces > 3.0 not found])])

AC_CHECK_HEADERS([AS\_02.h], [], [AC_MSG_ERROR([AS\_02.h was not found])])
AC_CHECK_HEADERS([KLV.h], [], [AC_MSG_ERROR([KLV.h was not found])]) 
AC_CHECK_HEADERS([KM_fileio.h], [], [AC_MSG_ERROR([KM_fileio.h was not found])]) 
AC_CHECK_HEADERS([KM_memio.h], [], [AC_MSG_ERROR([KM_memio.h was not found])]) 
AC_CHECK_HEADERS([KM_platform.h], [], [AC_MSG_ERROR([KM_platform.h was not found])]) 
AC_CHECK_HEADERS([KM_tai.h], [], [AC_MSG_ERROR([KM_tai.h was not found])]) 
AC_CHECK_HEADERS([KM_xml.h], [], [AC_MSG_ERROR([KM_xml.h was not found])]) 
AC_CHECK_HEADERS([Metadata.h], [], [AC_MSG_ERROR([Metadata.h was not found])]) 
AC_CHECK_HEADERS([MXFTypes.h], [], [AC_MSG_ERROR([MXFTypes.h was not found])]) 
AC_CHECK_HEADERS([AS\_DCP.h], [], [AC_MSG_ERROR([AS\_DCP.h was not found])]) 
AC_CHECK_HEADERS([KM_error.h], [], [AC_MSG_ERROR([KM_error.h was not found])]) 
AC_CHECK_HEADERS([KM_log.h], [], [AC_MSG_ERROR([KM_og.h was not found])])
AC_CHECK_HEADERS([KM_mutex.h], [], [AC_MSG_ERROR([KM_mutex.h was not found])])
AC_CHECK_HEADERS([KM_prng.h], [], [AC_MSG_ERROR([KM_prng.h was not found])])
AC_CHECK_HEADERS([KM_util.h], [], [AC_MSG_ERROR([KM_util.h was not found])])
AC_CHECK_HEADERS([MDD.h], [], [AC_MSG_ERROR([MDD.h was not found])])
AC_CHECK_HEADERS([MXF.h], [], [AC_MSG_ERROR([MXF.h was not found])])
AC_CHECK_HEADERS([PCMParserList.h], [], [AC_MSG_ERROR([PCMParserList.h was not found])])

AC_OUTPUT(Makefile src/Makefile)
