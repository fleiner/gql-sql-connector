#!/bin/bash

set -e -x
locale -a | grep en_US.utf8
locale -a | grep de_CH.utf8
locale -a | grep de_DE.utf8
export LC_ALL=C

srcdir=${srcdir:-$(dirname $0)}
$srcdir/mysqlcheck.py -c tinyint,,number,General -v -d alltypes \
    'select `tinyint`' \
    "0" "0" \
    "120" "120" \
    "127" "127" \
    "-128" "-128"

$srcdir/mysqlcheck.py -c tinyint,,number,0000 -v -d alltypes \
    'select `tinyint` format `tinyint` "0000"' \
    "0" "0000" \
    "120" "0120" \
    "127" "0127" \
    "-128" "-0128"

$srcdir/mysqlcheck.py -c smallint,,number,General -v -d alltypes \
    'select `smallint`' \
    "0" "0" \
    "30000" "30000" \
    "32767" "32767" \
    "-32768" "-32768"

$srcdir/mysqlcheck.py -c smallint,,number,@@@ -v -d alltypes \
    'select `smallint` format `smallint` "@@@"' \
    "0" "0.00" \
    "30000" "30000" \
    "32767" "32800" \
    "-32768" "-32800"

$srcdir/mysqlcheck.py -c mediumint,,number,General -v -d alltypes \
    'select `mediumint`' \
    "0" "0" \
    "8000000" "8000000" \
    "16777215" "16777215" \
    "0" "0"

LC_ALL=en_US.utf8 $srcdir/mysqlcheck.py -c mediumint,,number,General -v -d alltypes \
    'select `mediumint`' \
    "0" "0" \
    "8000000" "8,000,000" \
    "16777215" "16,777,215" \
    "0" "0"

LC_ALL=de_CH.utf8 $srcdir/mysqlcheck.py -c mediumint,,number,General -v -d alltypes \
    'select `mediumint`' \
    "0" "0" \
    "8000000" "8'000'000" \
    "16777215" "16'777'215" \
    "0" "0"

LC_ALL=de_DE.utf8 $srcdir/mysqlcheck.py -c mediumint,,number,General -v -d alltypes \
    'select `mediumint`' \
    "0" "0" \
    "8000000" "8.000.000" \
    "16777215" "16.777.215" \
    "0" "0"

$srcdir/mysqlcheck.py -c mediumint,,number,#,## -v -d alltypes \
    'select `mediumint` format `mediumint` "#,##"' \
    "0" "0" \
    "8000000" "8,00,00,00" \
    "16777215" "16,77,72,15" \
    "0" "0"

LC_ALL=de_CH.utf8 $srcdir/mysqlcheck.py -c mediumint,,number,#,## -v -d alltypes \
    'select `mediumint` format `mediumint` "#,##"' \
    "0" "0" \
    "8000000" "8'00'00'00" \
    "16777215" "16'77'72'15" \
    "0" "0"

$srcdir/mysqlcheck.py -c int,,number,General -v -d alltypes \
    'select `int`' \
    "0" "0" \
    "123123123" "123123123" \
    "2000000000" "2000000000" \
    "4294967295" "4294967295"

$srcdir/mysqlcheck.py -c int,,number,@@ -v -d alltypes \
    'select `int` format `int` "@@"' \
    "0" "0.0" \
    "123123123" "120000000" \
    "2000000000" "2000000000" \
    "4294967295" "4300000000"

$srcdir/mysqlcheck.py -c bigint,,number,General -v -d alltypes \
    'select `bigint`' \
    "0" "0" \
    "9000000000000000000" "9000000000000000000" \
    "-9223372036854775808" "-9223372036854775808" \
    "9223372036854775807" "9223372036854775807"

$srcdir/mysqlcheck.py -c "bigint,,number,@@@@@@@@@@;(@)" -v -d alltypes \
    'select `bigint` format `bigint` "@@@@@@@@@@;(@)"' \
    "0" "0.000000000" \
    "9000000000000000000" "9000000000000000000" \
    "-9223372036854775808" "(9223372037000000000)" \
    "9223372036854775807" "9223372037000000000"

$srcdir/mysqlcheck.py -c bigint,,number,General -v -d alltypes \
    'select `bigint`' \
    "0" "0" \
    "9000000000000000000" "9000000000000000000" \
    "-9223372036854775808" "-9223372036854775808" \
    "9223372036854775807" "9223372036854775807"

$srcdir/mysqlcheck.py -c ubigint,,number,General -v -d alltypes \
    'select `ubigint`' \
    "0" "0" \
    "9000000000000000000" "9000000000000000000" \
    "18446744073709551615" "18446744073709551615" \
    "9223372036854775807" "9223372036854775807"

$srcdir/mysqlcheck.py -c ubigint,,number,#,#### -v -d alltypes \
    'select `ubigint` format `ubigint` "#,####"' \
    "0" "0" \
    "9000000000000000000" "900,0000,0000,0000,0000" \
    "18446744073709551615" "1844,6744,0737,0960,0000" \
    "9223372036854775807" "922,3372,0368,5477,5807"

$srcdir/mysqlcheck.py -c float,,number,General -v -d alltypes -- \
    'select `float`' \
    "12.25" "12.25" \
    "-1.17549e-38" "-1.17549e-38" \
    "3.40282e+38" "3.40282e+38" \
    "1.17549e-38" "1.17549e-38"

LC_ALL=de_CH.utf8 $srcdir/mysqlcheck.py -c float,,number,General -v -d alltypes -- \
    'select `float`' \
    "12.25" "12.25" \
    "-1.17549e-38" "-1.17549e-38" \
    "3.40282e+38" "3.40282e+38" \
    "1.17549e-38" "1.17549e-38"

$srcdir/mysqlcheck.py -c float,,number,@@@% -v -d alltypes -- \
    'select `float` format `float` "@@@%"' \
    "12.25" "1220%" \
    "-1.17549e-38" "-0.00000000000000000000000000000000000118%" \
    "3.40282e+38" "34000000000000000000000000000000000000000%" \
    "1.17549e-38" "0.00000000000000000000000000000000000118%"

$srcdir/mysqlcheck.py -c double,,number,General -v -d alltypes -- \
    'select `double`' \
    "123.125" "123.125" \
    "-2.22507385851e-308" "-2.22507e-308" \
    "1.79769313486e+308" "1.79769e+308" \
    "2.22507385851e-308" "2.22507e-308"

$srcdir/mysqlcheck.py -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` format `double` "@@@E000"' \
    "123.125" "1.23E002" \
    "-2.22507385851e-308" "-2.23E-308" \
    "1.79769313486e+308" "1.80E308" \
    "2.22507385851e-308" "2.23E-308"

LC_ALL=de_CH.utf8 $srcdir/mysqlcheck.py -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` format `double` "@@@E000"' \
    "123.125" "1.23E002" \
    "-2.22507385851e-308" "-2.23E-308" \
    "1.79769313486e+308" "1.80E308" \
    "2.22507385851e-308" "2.23E-308"

LC_ALL=de_DE.utf8 $srcdir/mysqlcheck.py -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` format `double` "@@@E000"' \
    "123.125" "1,23E002" \
    "-2.22507385851e-308" "-2,23E-308" \
    "1.79769313486e+308" "1,80E308" \
    "2.22507385851e-308" "2,23E-308"

LC_ALL=en_US.utf8 $srcdir/mysqlcheck.py -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` format `double` "@@@E000"' \
    "123.125" "1.23E002" \
    "-2.22507385851e-308" "-2.23E-308" \
    "1.79769313486e+308" "1.80E308" \
    "2.22507385851e-308" "2.23E-308"

$srcdir/mysqlcheck.py -l en_US.utf8 -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` format `double` "@@@E000"' \
    "123.125" "1.23E002" \
    "-2.22507385851e-308" "-2.23E-308" \
    "1.79769313486e+308" "1.80E308" \
    "2.22507385851e-308" "2.23E-308"

$srcdir/mysqlcheck.py -l de_DE.utf8 -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` format `double` "@@@E000"' \
    "123.125" "1,23E002" \
    "-2.22507385851e-308" "-2,23E-308" \
    "1.79769313486e+308" "1,80E308" \
    "2.22507385851e-308" "2,23E-308"


$srcdir/mysqlcheck.py -c decimal,,number,General -v -d alltypes -- \
    'select `decimal`' \
    "0" "0" \
    "6789" "6789" \
    "12345.1235" "12345.1" \
    "-99999.9999" "-100000"

$srcdir/mysqlcheck.py -c decimal,,number,@@@@@@@@@@ -v -d alltypes -- \
    'select `decimal` format `decimal` "@@@@@@@@@@"' \
    "0" "0.000000000" \
    "6789" "6789.000000" \
    "12345.1235" "12345.12350" \
    "-99999.9999" "-99999.99990"

