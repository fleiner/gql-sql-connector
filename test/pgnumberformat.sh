#!/bin/bash

set -e -x
locale -a | grep en_US.utf8
locale -a | grep de_CH.utf8
locale -a | grep de_DE.utf8
export LC_ALL=C

srcdir=${srcdir:-$(dirname $0)}
$srcdir/postgresqlcheck.py -c tinyint,,number,General -v -d alltypes \
    'select `tinyint` order by `tinyint`' \
    "-128" "-128" \
    "0" "0" \
    "120" "120" \
    "127" "127"

$srcdir/postgresqlcheck.py -c tinyint,,number,0000 -v -d alltypes \
    'select `tinyint`  order by `tinyint` format `tinyint` "0000"' \
    "-128" "-0128" \
    "0" "0000" \
    "120" "0120" \
    "127" "0127"

$srcdir/postgresqlcheck.py -c smallint,,number,General -v -d alltypes \
    'select `smallint` order by smallint' \
    "-32768" "-32768" \
    "0" "0" \
    "30000" "30000" \
    "32767" "32767"

$srcdir/postgresqlcheck.py -c smallint,,number,@@@ -v -d alltypes \
    'select `smallint` order by `smallint` format `smallint` "@@@"' \
    "-32768" "-32800" \
    "0" "0.00" \
    "30000" "30000" \
    "32767" "32800"

$srcdir/postgresqlcheck.py -c mediumint,,number,General -v -d alltypes \
    'select `mediumint` order by `mediumint`' \
    "0" "0" \
    "0" "0" \
    "8000000" "8000000" \
    "16777215" "16777215"

LC_ALL=en_US.utf8 $srcdir/postgresqlcheck.py -c mediumint,,number,General -v -d alltypes \
    'select `mediumint` order by `mediumint`' \
    "0" "0" \
    "0" "0" \
    "8000000" "8,000,000" \
    "16777215" "16,777,215"

LC_ALL=de_CH.utf8 $srcdir/postgresqlcheck.py -c mediumint,,number,General -v -d alltypes \
    'select `mediumint` order by `mediumint`' \
    "0" "0" \
    "0" "0" \
    "8000000" "8'000'000" \
    "16777215" "16'777'215"

LC_ALL=de_DE.utf8 $srcdir/postgresqlcheck.py -c mediumint,,number,General -v -d alltypes \
    'select `mediumint` order by mediumint' \
    "0" "0" \
    "0" "0" \
    "8000000" "8.000.000" \
    "16777215" "16.777.215"

$srcdir/postgresqlcheck.py -c mediumint,,number,#,## -v -d alltypes \
    'select `mediumint` order by `mediumint` format `mediumint` "#,##"' \
    "0" "0" \
    "0" "0" \
    "8000000" "8,00,00,00" \
    "16777215" "16,77,72,15"

LC_ALL=de_CH.utf8 $srcdir/postgresqlcheck.py -c mediumint,,number,#,## -v -d alltypes \
    'select `mediumint` order by `mediumint` format `mediumint` "#,##"' \
    "0" "0" \
    "0" "0" \
    "8000000" "8'00'00'00" \
    "16777215" "16'77'72'15"

$srcdir/postgresqlcheck.py -c int,,number,General -v -d alltypes \
    'select `int` order by `int`' \
    "0" "0" \
    "123123123" "123123123" \
    "2000000000" "2000000000" \
    "4294967295" "4294967295"

$srcdir/postgresqlcheck.py -c int,,number,@@ -v -d alltypes \
    'select `int`  order by `int`format `int` "@@"' \
    "0" "0.0" \
    "123123123" "120000000" \
    "2000000000" "2000000000" \
    "4294967295" "4300000000"

$srcdir/postgresqlcheck.py -c bigint,,number,General -v -d alltypes \
    'select `bigint` order by `bigint`' \
    "-9223372036854775808" "-9223372036854775808" \
    "0" "0" \
    "9000000000000000000" "9000000000000000000" \
    "9223372036854775807" "9223372036854775807"

$srcdir/postgresqlcheck.py -c "bigint,,number,@@@@@@@@@@;(@)" -v -d alltypes \
    'select `bigint` order by `bigint` format `bigint` "@@@@@@@@@@;(@)"' \
    "-9223372036854775808" "(9223372037000000000)" \
    "0" "0.000000000" \
    "9000000000000000000" "9000000000000000000" \
    "9223372036854775807" "9223372037000000000"

$srcdir/postgresqlcheck.py -c bigint,,number,General -v -d alltypes \
    'select `bigint` order by `bigint`' \
    "-9223372036854775808" "-9223372036854775808" \
    "0" "0" \
    "9000000000000000000" "9000000000000000000" \
    "9223372036854775807" "9223372036854775807"

$srcdir/postgresqlcheck.py -c float,,number,General -v -d alltypes -- \
    'select `float` order by `float`' \
    "-1.17549e-38" "-1.17549e-38" \
    "1.17549e-38" "1.17549e-38" \
    "12.25" "12.25" \
    "3.40282e+38" "3.40282e+38"

LC_ALL=de_CH.utf8 $srcdir/postgresqlcheck.py -c float,,number,General -v -d alltypes -- \
    'select `float` order by `float`' \
    "-1.17549e-38" "-1.17549e-38" \
    "1.17549e-38" "1.17549e-38" \
    "12.25" "12.25" \
    "3.40282e+38" "3.40282e+38"

$srcdir/postgresqlcheck.py -c float,,number,@@@% -v -d alltypes -- \
    'select `float` order by `float` format `float` "@@@%"' \
    "-1.17549e-38" "-0.00000000000000000000000000000000000118%" \
    "1.17549e-38" "0.00000000000000000000000000000000000118%" \
    "12.25" "1220%" \
    "3.40282e+38" "34000000000000000000000000000000000000000%"

$srcdir/postgresqlcheck.py -c double,,number,General -v -d alltypes -- \
    'select `double` order by `double`' \
    "-2.22507385851e-308" "-2.22507e-308" \
    "2.22507385851e-308" "2.22507e-308" \
    "123.125" "123.125" \
    "1.79769313486e+308" "1.79769e+308"

$srcdir/postgresqlcheck.py -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` order by `double` format `double` "@@@E000"' \
    "-2.22507385851e-308" "-2.23E-308" \
    "2.22507385851e-308" "2.23E-308" \
    "123.125" "1.23E002" \
    "1.79769313486e+308" "1.80E308"

LC_ALL=de_CH.utf8 $srcdir/postgresqlcheck.py -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` order by `double` format `double` "@@@E000"' \
    "-2.22507385851e-308" "-2.23E-308" \
    "2.22507385851e-308" "2.23E-308" \
    "123.125" "1.23E002" \
    "1.79769313486e+308" "1.80E308"

LC_ALL=de_DE.utf8 $srcdir/postgresqlcheck.py -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` order by `double` format `double` "@@@E000"' \
    "-2.22507385851e-308" "-2,23E-308" \
    "2.22507385851e-308" "2,23E-308" \
    "123.125" "1,23E002" \
    "1.79769313486e+308" "1,80E308"

LC_ALL=en_US.utf8 $srcdir/postgresqlcheck.py -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` order by `double` format `double` "@@@E000"' \
    "-2.22507385851e-308" "-2.23E-308" \
    "2.22507385851e-308" "2.23E-308" \
    "123.125" "1.23E002" \
    "1.79769313486e+308" "1.80E308"

$srcdir/postgresqlcheck.py -l en_US.utf8 -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` order by `double` format `double` "@@@E000"' \
    "-2.22507385851e-308" "-2.23E-308" \
    "2.22507385851e-308" "2.23E-308" \
    "123.125" "1.23E002" \
    "1.79769313486e+308" "1.80E308"

$srcdir/postgresqlcheck.py -l de_DE.utf8 -c double,,number,@@@E000 -v -d alltypes -- \
    'select `double` order by `double` format `double` "@@@E000"' \
    "-2.22507385851e-308" "-2,23E-308" \
    "2.22507385851e-308" "2,23E-308" \
    "123.125" "1,23E002" \
    "1.79769313486e+308" "1,80E308"


$srcdir/postgresqlcheck.py -c decimal,,number,General -v -d alltypes -- \
    'select `decimal` order by `decimal`' \
    "-99999.9999" "-100000" \
    "0" "0" \
    "6789" "6789" \
    "12345.1235" "12345.1"

$srcdir/postgresqlcheck.py -c decimal,,number,@@@@@@@@@@ -v -d alltypes -- \
    'select `decimal` order by `decimal` format `decimal` "@@@@@@@@@@"' \
    "-99999.9999" "-99999.99990" \
    "0" "0.000000000" \
    "6789" "6789.000000" \
    "12345.1235" "12345.12350"

