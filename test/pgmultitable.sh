#!/bin/bash

set -e -x
export LC_ALL=C


srcdir=${srcdir:-$(dirname $0)}
$srcdir/postgresqlcheck.py -c T1.EINS,,number,General -v -d alltypes -t T1 \
    'select `T1.EINS` options no_format' \
    "2" \
    "3" \
    "5" \
    "7" 

$srcdir/postgresqlcheck.py -c T1.EINS,,number,General -c smallint,,number,General -v -d alltypes -t T1 -- \
    'select `T1.EINS`,`smallint` WHERE `T1.EINS`=t1key options no_format' \
    "2" "0" \
    "3" "32767" \
    "5" "30000" \
    "7"  "-32768"

$srcdir/postgresqlcheck.py -c T1.EINS,,number,General -c smallint,,number,General -v -d alltypes -t T1 -- \
    'select `T1.EINS`,`smallint` WHERE `T1.EINS`=t1key order by `smallint` options no_format' \
    "7"  "-32768" \
    "2" "0" \
    "5" "30000" \
    "3" "32767"

$srcdir/postgresqlcheck.py -c T1.EINS,,number,@@ -c smallint,,number,#,# -v -d alltypes -t T1 -- \
    'select `T1.EINS`,`smallint` WHERE `T1.EINS`=t1key order by `smallint` format `T1.EINS` "@@", `smallint` "#,#"' \
    7 "7.0" -32768 "-3,2,7,6,8" \
    2 "2.0" 0 "0" \
    5 "5.0" 30000 "3,0,0,0,0" \
    3 "3.0" 32767 "3,2,7,6,7"

$srcdir/postgresqlcheck.py -c T1.EINS,,number,@@ \
                      -c smallint,,number,#,# \
                      -c '(`T1.EINS`+smallint),sum,number,#,###' -v -d alltypes -t T1 -- \
    'select `T1.EINS`,`smallint`,`T1.EINS`+`smallint` WHERE `T1.EINS`=t1key order by `T1.EINS`*`smallint` label `T1.EINS`+`smallint` "sum" format `T1.EINS` "@@", `smallint` "#,#", `T1.EINS`+`smallint` "#,###" options no_values' \
    "7.0" "-3,2,7,6,8" "-32,761" \
    "2.0" "0" "2" \
    "3.0" "3,2,7,6,7" "32,770" \
    "5.0" "3,0,0,0,0" "30,005"
