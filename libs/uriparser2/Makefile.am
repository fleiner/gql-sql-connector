AM_CFLAGS=-ansi -Wall -O2 -fPIC

lib_LTLIBRARIES=liburiparser2.la 

liburiparser2_la_SOURCES=uriparser2.c \
			 uriparser/UriParse.c \
			 uriparser/UriParseBase.c \
			 uriparser/UriCommon.c \
			 uriparser/UriIp4Base.c \
			 uriparser/UriIp4.c
 

check_PROGRAMS=test-uriparser2

TESTS=$(check_PROGRAMS)

test_uriparser2_SOURCES=test-uriparser2.c
test_uriparser2_LDADD=liburiparser2.la
