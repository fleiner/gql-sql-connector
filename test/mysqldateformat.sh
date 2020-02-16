#!/bin/bash

set -e -x
locale -a | grep en_US.utf8
locale -a | grep de_CH.utf8
locale -a | grep de_DE.utf8
export LC_ALL=C
srcdir=${srcdir:-$(dirname $0)}

$srcdir/mysqlcheck.py -c datetime,,datetime,General -v -d alltypes \
    'select `datetime`' \
    "Date(1,0,1,0,0,0,0)" "0001/01/01 0:00:00.000" \
    "Date(2980,11,31,23,59,59,999)" "2980/12/31 23:59:59.999" \
    "Date(1940,0,1,13,14,15,456)" "1940/01/01 13:14:15.456" \
    "Date(1080,0,1,13,14,15,456)" "1080/01/01 13:14:15.456"

LC_ALL=en_US.utf8 $srcdir/mysqlcheck.py -c datetime,,datetime,General -v -d alltypes \
    'select `datetime`' \
    "Date(1,0,1,0,0,0,0)" "0001/01/01 0:00:00.000" \
    "Date(2980,11,31,23,59,59,999)" "2980/12/31 23:59:59.999" \
    "Date(1940,0,1,13,14,15,456)" "1940/01/01 13:14:15.456" \
    "Date(1080,0,1,13,14,15,456)" "1080/01/01 13:14:15.456"

LC_ALL=de_DE.utf8 $srcdir/mysqlcheck.py -c datetime,,datetime,General -v -d alltypes \
    'select `datetime`' \
    "Date(1,0,1,0,0,0,0)" "0001/01/01 0:00:00.000" \
    "Date(2980,11,31,23,59,59,999)" "2980/12/31 23:59:59.999" \
    "Date(1940,0,1,13,14,15,456)" "1940/01/01 13:14:15.456" \
    "Date(1080,0,1,13,14,15,456)" "1080/01/01 13:14:15.456"

$srcdir/mysqlcheck.py -c datetime,DAT,datetime,yyyy:MM:dd -v -d alltypes \
    'select `datetime` label `datetime` "DAT" format `datetime` "yyyy:MM:dd"' \
    "Date(1,0,1,0,0,0,0)" "0001:01:01" \
    "Date(2980,11,31,23,59,59,999)" "2980:12:31" \
    "Date(1940,0,1,13,14,15,456)" "1940:01:01" \
    "Date(1080,0,1,13,14,15,456)" "1080:01:01"

$srcdir/mysqlcheck.py -c date,,date,General -v -d alltypes \
    'select `date`' \
    "Date(1234,0,1)" "1234/01/01" \
    "Date(1980,11,31)" "1980/12/31" \
    "Date(1,0,1)" "0001/01/01" \
    "Date(2980,0,1)" "2980/01/01"

$srcdir/mysqlcheck.py -c date,,date,"MM.dd'.'yy" -v -d alltypes \
    'select `date` format `date` "MM.dd'"'"'.'"'"'yy"' \
    "Date(1234,0,1)" "01.01.34" \
    "Date(1980,11,31)" "12.31.80" \
    "Date(1,0,1)" "01.01.01" \
    "Date(2980,0,1)" "01.01.80"

# timestamp not recognized as timestamp, but as string
$srcdir/mysqlcheck.py -c timestamp,,string,General -v -d alltypes \
    'select `timestamp`' \
    "2022-12-31 00:00:00" \
    "1981-01-02 09:10:11" \
    "1970-01-01 00:00:01" \
    "0000-00-00 00:00:00"

$srcdir/mysqlcheck.py -c timestamp,DAT,date,yyyy:MM:dd -v -d alltypes \
    'select `timestamp` label `timestamp` "DAT" format `timestamp` "yyyy:MM:dd"' \
    "Date(2022,11,31)" "2022:12:31" \
    "Date(1981,0,2)" "1981:01:02" \
    "Date(1970,0,1)" "1970:01:01" \
    "Date(1,0,1)" "0001:01:01"

$srcdir/mysqlcheck.py -c timestamp,DAT,datetime,yyyy:MM:dd+ss+mm+HH -v -d alltypes \
    'select `timestamp` label `timestamp` "DAT" format `timestamp` "yyyy:MM:dd+ss+mm+HH"' \
    "Date(2022,11,31,0,0,0,0)" "2022:12:31+00+00+00" \
    "Date(1981,0,2,9,10,11,0)" "1981:01:02+11+10+09" \
    "Date(1970,0,1,0,0,1,0)" "1970:01:01+01+00+00" \
    "Date(1,0,1,0,0,0,0)" "0001:01:01+00+00+00"

$srcdir/mysqlcheck.py -c time,,timeofday,General -v -d alltypes \
    'select `time`' \
    "[0, 0, 0, 0]" "0:00:00.000" \
    "[4, 5, 6, 789]" "4:05:06.789" \
    "[4, 5, 6, 89]" "4:05:06.089" \
    "[23, 59, 59, 999]" "23:59:59.999"

$srcdir/mysqlcheck.py -c time,,timeofday,s:m:H -v -d alltypes \
    'select `time` format `time` "s:m:H"' \
    "[0, 0, 0, 0]" "0:0:0" \
    "[4, 5, 6, 789]" "6:5:4" \
    "[4, 5, 6, 89]" "6:5:4" \
    "[23, 59, 59, 999]" "59:59:23"

# timestamp not recognized as timestamp, but as string
$srcdir/mysqlcheck.py -c year,,string,General -v -d alltypes \
    'select `year`' \
    "2155" \
    "0000" \
    "1983" \
    "1901"

$srcdir/mysqlcheck.py -c year,,date,yy -v -d alltypes \
    'select `year` format `year` "yy"' \
    "Date(2155,0,1)" "55" \
    "Date(1,0,1)" "01" \
    "Date(1983,0,1)" "83" \
    "Date(1901,0,1)" "01"

$srcdir/mysqlcheck.py -c datetimesshort,,datetime,General \
                      -c timeshort,,timeofday,General -v -d alltypes \
    'select datetimesshort,timeshort options no_format' \
    "Date(1967,10,30,11,1,2,0)" "[1, 2, 3, 0]" \
    "Date(2,10,30,0,0,0,0)" "[6, 7, 8, 0]" \
    "Date(1900,0,31,0,0,0,0)" "[2, 4, 5, 0]" \
    "Date(2,10,30,0,0,0,0)" "[10, 11, 12, 0]"
