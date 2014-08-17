#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([imfenc], [0.8], [markus@odmedia.nl])
AM_INIT_AUTOMAKE([-Wall -Werror])

AC_LANG(C++)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/cli.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([openjp2], [main], [], [AC_MSG_ERROR([libopenjp2 was not found])])
# FIXME: Replace `main' with a function in `-las02':
AC_CHECK_LIB([as02], [main], [], [AC_MSG_ERROR([libas02 was not found])])
# FIXME: Replace `main' with a function in `-lasdcp':
AC_CHECK_LIB([asdcp], [main], [], [AC_MSG_ERROR([libasdcp was not found])])
# FIXME: Replace `main' with a function in `-lavcodec':
AC_CHECK_LIB([avcodec], [main], [], [AC_MSG_ERROR([libavcodec was not found])])
# FIXME: Replace `main' with a function in `-lavformat':
AC_CHECK_LIB([avformat], [main], [], [AC_MSG_ERROR([libavformat was not found])])
# FIXME: Replace `main' with a function in `-lavutil':
AC_CHECK_LIB([avutil], [main], [], [AC_MSG_ERROR([libavutil was not found])])
# FIXME: Replace `main' with a function in `-lboost_fileystem':
# AC_CHECK_LIB([boost_fileystem], [boost::filesystem::remove], [], [AC_MSG_ERROR([libboost_fileystem was not found])])
AC_CHECK_LIB([boost_program_options], [main], [], [AC_MSG_ERROR([libboost_program_options was not found])])
# FIXME: Replace `main' with a function in `-lboost_system':
AC_CHECK_LIB([boost_system], [main], [], [AC_MSG_ERROR([libboost_system was not found])])
# FIXME: Replace `main' with a function in `-lkumu':
AC_CHECK_LIB([kumu], [main], [], [AC_MSG_ERROR([libkumu was not found])])
# FIXME: Replace `main' with a function in `-lswresample':
AC_CHECK_LIB([swresample], [main], [], [AC_MSG_ERROR([libswresample was not found])])
# FIXME: Replace `main' with a function in `-lswscale':
AC_CHECK_LIB([swscale], [main], [], [AC_MSG_ERROR([libswscale was not found])])


# Checks for header files.
AC_CHECK_HEADERS([stdint.h])

# headers for openjpeg2
AC_CHECK_HEADERS([openjpeg-2.1/openjpeg.h], [], [AC_MSG_ERROR([openjpeg.h was not found])])

# headers for ASDCPLIB

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

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_OUTPUT(Makefile src/Makefile)
