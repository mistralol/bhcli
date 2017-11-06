AC_INIT(bhcli, 1.0.0, james@stev.org)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([src/bhcli.cpp])
AC_CONFIG_MACRO_DIR([m4])

LT_INIT
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_PROG_CXX

AX_CXX_COMPILE_STDCXX_11(noext, optional)

m4_ifdef([AX_COMPILER_FLAGS],
	[
		AX_COMPILER_FLAGS([WARN_CFLAGS],[WARN_LDFLAGS])
		AC_SUBST([AM_CXXFLAGS], $WARN_CXXFLAGS)
	],
	[
		AC_SUBST([AM_CXXFLAGS], ['-Wall -Werror -std=c++11'])
	]
)

AC_PROG_LIBTOOL

dnl give error and exit if we don't have pkgconfig
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, [ ], [
  AC_MSG_ERROR([You need to have pkg-config installed!])
])

dnl Check for jsoncpp
PKG_CHECK_MODULES(JSON, [
	jsoncpp >= 0.6.0
], [
	AC_SUBST(JSON_CFLAGS)
	AC_SUBST(JSON_LIBS)
], [
	AC_MSG_ERROR([
		You need to install or upgrade jsoncpp
	])
])

dnl Check for jsoncpp
PKG_CHECK_MODULES(CURL, [
	libcurl >= 0.29.0
], [
	AC_SUBST(CURL_CFLAGS)
	AC_SUBST(CURL_LIBS)
], [
	AC_MSG_ERROR([
		You need to install or upgrade libcurl
	])
])

AC_CHECK_HEADER_STDBOOL()

AC_TYPE_SIZE_T
AC_TYPE_UINT64_T

AC_CHECK_FUNCS([localtime_r])

AC_OUTPUT(Makefile 
	src/Makefile
)


