#!/bin/bash

set -e -x
export LC_ALL=C

PS4='+${LINENO}+'
srcdir=${srcdir:-$(dirname $0)}

$srcdir/mysqlcheck.py \
    -c 'year(now()),,number,General' \
    -c 'month(now()),,number,General' \
    -c 'day(now()),,number,General' \
    -v -d alltypes --\
    'select year(now()),month(now()),day(now()) WHERE `varchar` like "%ll%" ORDER by `int` options no_format' \
    $(date +%Y) $(($(date +%m)+0)) $(($(date +%d)+0)) \
    $(date +%Y) $(($(date +%m)+0)) $(($(date +%d)+0))

$srcdir/mysqlcheck.py -c 'smallint,,number,General' \
    -v -d alltypes --\
    'select `smallint` WHERE `varchar` like "%ll%" ORDER by `int` options no_format' \
        30000 32767

$srcdir/mysqlcheck.py -c 'Ints,,number,General' \
    -v -d 'T B' --\
    'select `Ints` WHERE `number` is null order by Ints options no_format' \
        1 3

$srcdir/mysqlcheck.py -c 'Ints,,number,General' \
    -v -d 'T B' --\
    'select `Ints` WHERE `number` is not null order by Ints options no_format' \
        2 4

$srcdir/mysqlcheck.py -c 'smallint,,number,General' \
    -v -d alltypes --\
    'select `smallint` WHERE `varchar` ends with "789" ORDER by `int` options no_format' \
        -32768

$srcdir/mysqlcheck.py -c 'smallint,,number,General' \
    -v -d alltypes --\
    'select `smallint` WHERE `varchar` starts with "h" ORDER by `int` options no_format' \
        30000 32767

$srcdir/mysqlcheck.py -c 'smallint,,number,General' \
    -v -d alltypes --\
    'select `smallint` WHERE `varchar` contains "llo" ORDER by `int` options no_format' \
        30000 32767

$srcdir/mysqlcheck.py -c 'smallint,,number,General' \
    -v -d alltypes --\
    'select `smallint` WHERE `text` matches ".l." ORDER by `int` options no_format' \
        32767


$srcdir/mysqlcheck.py -c '((EINS-ZWEI)+`DREI UND`),,number,General' \
                      -c '(EINS-(ZWEI+`DREI UND`)),,number,General' \
    -v -d T1 --\
    'select EINS-ZWEI+`DREI UND`,EINS-(ZWEI+`DREI UND`) options no_format' \
    -29 -31  -7 -9  32 -30  6 -40

$srcdir/mysqlcheck.py -c '((EINS-ZWEI)+`DREI UND`),,number,General' \
                      -c '(EINS-(ZWEI+`DREI UND`)),,number,General' \
    -v -d T1 --\
    'select EINS-ZWEI+`DREI UND`,EINS-(ZWEI+`DREI UND`) WHERE ZWEI>EINS and VIER>ZWEI options no_format' \
    -29 -31  -7 -9

$srcdir/mysqlcheck.py -c 'EINS,,number,General' \
    -v -d T1 --\
    'select EINS WHERE ZWEI>EINS=false  and VIER>ZWEI=true options no_format' \
    5

$srcdir/mysqlcheck.py -c 'EINS,,number,General' \
    -v -d T1 --\
    'select EINS WHERE not ZWEI>EINS and not not VIER>ZWEI=true options no_format' \
    5

$srcdir/mysqlcheck.py -c '(EINS+(EINS*ZWEI)),,number,General' \
                      -c '((EINS/ZWEI)-ZWEI),,number,General' \
    -v -d T1 --\
    'select EINS+EINS*ZWEI,EINS/ZWEI-ZWEI options no_format' \
    66 -31.9375  36 -10.7273  25 -2.75  175 -23.7083

$srcdir/mysqlcheck.py -c '(EINS+1),,number,General' \
                      -c '(EINS-1),,number,General' \
                      -c '(EINS-(-1)),,number,General' \
                      -c '(EINS+(-1)),,number,General' \
    -v -d T1 --\
    'select EINS++1,EINS-+1,EINS--1,EINS+-1 options no_format' \
    3 1 3 1   4 2 4 2  6 4 6 4  8 6 8 6

$srcdir/mysqlcheck.py -c 'date,,date,General' \
    -v -d alltypes --\
    'select `date` WHERE `date` < date "2000-01-01" ORDER by t1key options no_format' \
    "Date(1234,0,1)" "Date(1,0,1)" "Date(1980,11,31)"

$srcdir/mysqlcheck.py -c 'datetime,,datetime,General' \
    -v -d alltypes --\
    'select `datetime` WHERE `datetime` < datetime "1940-01-01 13:14:15.456" ORDER by t1key options no_format' \
    "Date(1,0,1,0,0,0,0)" "Date(1080,0,1,13,14,15,456)"

$srcdir/mysqlcheck.py -c 'datetime,,datetime,General' \
    -v -d alltypes --\
    'select `datetime` WHERE `datetime` <= datetime "1940-01-01 13:14:15.456" ORDER by t1key options no_format' \
    "Date(1,0,1,0,0,0,0)" "Date(1940,0,1,13,14,15,456)" "Date(1080,0,1,13,14,15,456)"

$srcdir/mysqlcheck.py -c 'time,,timeofday,General' \
    -v -d alltypes --\
    'select `time` WHERE `time` < timeofday "04:05:06.789" ORDER by t1key options no_format' \
    "[0, 0, 0, 0]" "[4, 5, 6, 89]"

$srcdir/mysqlcheck.py -c 'time,,timeofday,General' \
    -v -d alltypes --\
    'select `time` WHERE `time` <= timeofday "04:05:06.789" ORDER by t1key options no_format' \
    "[0, 0, 0, 0]" "[4, 5, 6, 89]" "[4, 5, 6, 789]"

$srcdir/mysqlcheck.py -c 'year(`date`),,number,General' \
                      -c 'month(`date`),,number,General' \
                      -c 'day(`date`),,number,General' \
                      -c '((((hour(`datetime`)*60)+minute(time))*60)+second(`timestamp`)),,number,General' \
                      -c 'millisecond(`datetime`),,number,General' \
    -v -d alltypes --\
    'select year(`date`),month(`date`),day(`date`),(hour(`datetime`)*60+minute(`time`))*60+second(`timestamp`),millisecond(`datetime`) WHERE `bool` ORDER by t1key options no_format' \
    0 0 0  47101 456 \
    1980 12 31 83111 999 \
    2980 1 1 50340 456

$srcdir/mysqlcheck.py -c 'year(`datetime`),,number,General' \
                      -c 'month(`datetime`),,number,General' \
                      -c 'day(`datetime`),,number,General' \
    -v -d alltypes --\
    'select year(`datetime`),month(`datetime`),day(`datetime`) WHERE `bool` ORDER by t1key options no_format' \
    1940 1 1 \
    2980 12 31 \
    1080 1 1 \

$srcdir/mysqlcheck.py -c 'dateDiff(`datetime`%2C `date`),,number,General' \
    -v -d alltypes --\
    'select dateDiff(`datetime`,`date`) ORDER by t1key options no_format' \
    None None 365243 -693961

$srcdir/mysqlcheck.py -c 'toDate(`datetime`),,date,General' \
    -c 'dateDiff(`datetime`%2C datetimesshort),diff,number,General' \
    -v -d alltypes --\
    'select toDate(`datetime`),dateDiff(`datetime`,datetimesshort) ORDER by t1key LABEL dateDiff(`datetime`,datetimesshort) "diff" options no_format' \
        "Date(1,0,1)" None \
        "Date(1940,0,1)" 14579 \
        "Date(2980,11,31)" None \
        "Date(1080,0,1)" None

$srcdir/mysqlcheck.py -c 'upper(text),,string,General' \
                      -c 'lower(text),,string,General' \
    -v -d alltypes --\
    'select upper(text),lower(text) ORDER by t1key options no_format' \
        "" "" \
        "BLA" "bla" \
        "<&SMALL>" "<&small>" \
        "SOMETHING
"'"LONG"'"
AND WITH
UTF 8
ÄÖÜ" \
     "something
"'"long"'"
and with
utf 8
äöü"
