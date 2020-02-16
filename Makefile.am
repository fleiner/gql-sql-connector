SUBDIRS=libs/uriparser2 . test

MAKEFLAGS=-j
AM_CPPFLAGS=-g -DMYSQLPP_MYSQL_HEADERS_BURIED

ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES=libgqlsql.la

bin_PROGRAMS=gqldb


libgqlsql_la_SOURCES=libgqlparse.cpp \
		       libgqlsql.h \
		       gqlprinter.cpp \
		       libgqldb.cpp \
		       mysqlconnect.cpp \
		       postgresqlconnect.cpp \
		       gqlcgi.cpp

doc/html/index.html: $(libgqlsql_la_SOURCES) \
                     $(gqldb_SOURCES) \
                     $(gqlparse_SOURCES) \
		     gqldb-usage.txt \
		     config.doxygen README.md
	doxygen config.doxygen

doc:    doc/html/index.html

gqldb-usage.txt: gqldb$(EXEEXT)
	./gqldb$(EXEEXT) -h 2> gqldb-usage.txt || true

$(srcdir)/postgresql.pg_type: getpysqlcatalog.py
	$(srcdir)/getpysqlcatalog.py > $(srcdir)/postgresql.pg_type

postgresqlconnect.lo: $(srcdir)/postgresql.pg_type

gqldb_SOURCES=gqldb.cpp libgqlsql.h
gqldb_LDADD=libgqlsql.la -Llibs/uriparser2/.libs -luriparser2 -ljsoncpp -lglog -lmysqlpp -lpqxx @ICULINK@
