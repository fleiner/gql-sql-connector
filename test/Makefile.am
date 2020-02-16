
# mysqldump --skip-lock-tables -u gqltest -pgqltest gqltest
check_PROGRAMS=TokenTest ParserTest PrinterTest OnExitTest

TESTS=$(check_PROGRAMS) \
      mysqlutf.sh \
      mysqlboolformat.sh \
      mysqlnumberformat.sh \
      mysqldateformat.sh \
      mysqlexpr.sh \
      mysqlgroup.sh \
      mysqlmultitable.sh \
      mysqlpivot.sh \
      mysqllimit.sh \
      mysqlext.sh \
      pgutf.sh \
      pgboolformat.sh \
      pgnumberformat.sh \
      pgdateformat.sh \
      pgexpr.sh \
      pggroup.sh \
      pgmultitable.sh \
      pgpivot.sh \
      pglimit.sh \
      pgext.sh \
      testoutput.sh \
      gqltest.sh \
      testcgi.sh

export srcdir

AM_CPPFLAGS=-I$(srcdir)/.. -g -DMYSQLPP_MYSQL_HEADERS_BURIED

LDADD=../libgqlsql.la -lgtest -lpthread -ljsoncpp -lmysqlpp -lglog -lpqxx @ICULINK@

TokenTest_SOURCES=TokenTest.cpp

ParserTest_SOURCES=ParserTest.cpp

PrinterTest_SOURCES=PrinterTest.cpp

OnExitTest_SOURCES=OnExitTest.cpp


export VERBOSE=1

dump:
	mysqldump -u gqltest -pgqltest --skip-lock-tables gqltest > $(srcdir)/mysql_gqltest.sql
	PGPASSWORD=gqltest pg_dump -U gqltest -h localhost gqltest > $(srcdir)/pg_gqltest.sql
