AC_PREREQ(2.61)
AC_INIT(GQLParse,0.1,support@mensus.net)
LT_INIT
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])


AC_PROG_CXX
AC_LANG(C++)
use14=1
AX_CHECK_COMPILE_FLAG([-std=c++14], [ CXXFLAGS="$CXXFLAGS -std=c++14"], [ use14=0 ])
if test x$use14 = x0 ; then
	AX_CHECK_COMPILE_FLAG([-std=c++11], [ CXXFLAGS="$CXXFLAGS -std=c++11"], [
		AC_MSG_ERROR([sorry, support for C++11 is required])])
fi
AX_APPEND_COMPILE_FLAGS([-Werror])
AX_APPEND_COMPILE_FLAGS([-Wall])
AX_APPEND_COMPILE_FLAGS([-Wextra])
AX_APPEND_COMPILE_FLAGS([-Wpedantic])
AX_APPEND_COMPILE_FLAGS([-Wno-unused-macros])
AX_APPEND_COMPILE_FLAGS([-Wno-padded]) # needed to compile various classes

if test "$CXX" = "clang++" ; then
    AX_APPEND_COMPILE_FLAGS([-Wno-disabled-macro-expansion])
    AX_APPEND_COMPILE_FLAGS([-Wno-reserved-id-macro]) # prevents boost from compiling
    AX_APPEND_COMPILE_FLAGS([-Wno-c++98-compat]) # why needed if c++14 is called for?
    AX_APPEND_COMPILE_FLAGS([-Wno-c++98-compat-pedantic]) # why needed if c++14 is called for?
    AX_APPEND_COMPILE_FLAGS([-Wno-documentation-deprecated-sync]) # needed to compile icu headers
    AX_APPEND_COMPILE_FLAGS([-Wno-documentation-unknown-command]) # needed to compile icu headers
    AX_APPEND_COMPILE_FLAGS([-Wno-documentation]) # needed to compile icu headers
    AX_APPEND_COMPILE_FLAGS([-Wno-exit-time-destructors]) # allow global static strings
    AX_APPEND_COMPILE_FLAGS([-Wno-global-constructors]) # allow global static strings
fi

AX_APPEND_COMPILE_FLAGS([-Weverything])
AX_APPEND_COMPILE_FLAGS([-Wshadow])
AX_APPEND_COMPILE_FLAGS([-Woverloaded-virtual])
AC_DEFINE([MYSQLPP_MYSQL_HEADERS_BURIED], [], [Needed for mysql++])

AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([jsoncpp/json/json.h],[],AC_MSG_ERROR([Couldn't find or include jsoncpp/json/json.h]))
AC_CHECK_HEADERS([mysql++/mysql++.h],[],AC_MSG_ERROR([Couldn't find or include mysql++/mysql++.h]))
AC_CHECK_HEADERS([pqxx/pqxx],[],AC_MSG_ERROR([Couldn't find or include pqxx/pqxx (postgresql)]))
AC_CHECK_HEADERS([boost/algorithm/string.hpp],[],AC_MSG_ERROR([Couldn't find or include boost/algorithm/string.hpp]))
AC_CHECK_HEADERS([unicode/numfmt.h],[],AC_MSG_ERROR([Couldn't find or include unicode/numfmt.h; libicu missing?]))

AC_SUBST(ICULINK,[$(icu-config "--ldflags")])

AC_CONFIG_FILES([Makefile test/Makefile libs/uriparser2/Makefile config.doxygen])

AC_LANG_POP([C++])
AM_PATH_PYTHON
AC_OUTPUT
