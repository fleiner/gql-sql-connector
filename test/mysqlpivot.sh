#!/bin/bash

set -e -x
export LC_ALL=C

srcdir=${srcdir:-$(dirname $0)}

$srcdir/mysqlcheck.py \
    -c "Eng%2C12:00:00.000 sum(salary),Eng%2C12:00:00.000 sum(salary),number,General" \
    -c "Eng%2C13:00:00.000 sum(salary),Eng%2C13:00:00.000 sum(salary),number,General" \
    -c "Marketing%2C13:00:00.000 sum(salary),Marketing%2C13:00:00.000 sum(salary),number,General" \
    -c "Sales%2C12:00:00.000 sum(salary),Sales%2C12:00:00.000 sum(salary),number,General" \
     -v -d pv -- \
    'select sum(salary) pivot dept, lunchTime' \
    1500 1500  600 600  800 800  750 750

$srcdir/mysqlcheck.py \
    -c "Eng sum(salary),Eng sum(salary),number,#,##" \
    -c "Marketing sum(salary),Marketing sum(salary),number,#,##" \
    -c "Sales sum(salary),Sales sum(salary),number,#,##" \
    -v -d pv -- \
    'select sum(salary) pivot dept format sum(salary) "#,##"' \
    2100 21,00  800 8,00 750 7,50

$srcdir/mysqlcheck.py \
    -c dept,,string,General \
    -c "12:00:00.000 sum(salary),12:00:00.000 sum(salary),number,#,###" \
    -c "13:00:00.000 sum(salary),13:00:00.000 sum(salary),number,#,###" \
    -v -d pv -- \
    'select dept, sum(salary) group by dept pivot lunchTime format sum(salary) "#,###"' \
    Eng  1500 1,500 600 600 \
    Sales  750 750  None \
    Marketing  None 800 800

$srcdir/mysqlcheck.py \
    -c lunchTime,,timeofday,General \
    -c "Eng sum(salary),Eng sum(salary),number,General" \
    -c "Marketing sum(salary),Marketing sum(salary),number,General" \
    -c "Sales sum(salary),Sales sum(salary),number,General" \
    -v -d pv -- \
    'select lunchTime, sum(salary) group by lunchTime pivot dept' \
    "[12, 0, 0, 0]" "12:00:00.000" 1500 1500 None 750 750 \
    "[13, 0, 0, 0]" "13:00:00.000" 600 600 800 800 None

$srcdir/mysqlcheck.py \
    -c "Eng sum(salary),Eng PAY,number,General" \
    -c "Marketing sum(salary),Marketing PAY,number,General" \
    -c "Sales sum(salary),Sales PAY,number,General" \
    -c "Eng max(lunchTime),Eng max(lunchTime),timeofday,General" \
    -c "Marketing max(lunchTime),Marketing max(lunchTime),timeofday,General" \
    -c "Sales max(lunchTime),Sales max(lunchTime),timeofday,General" \
    -v -d pv -- \
        'select sum(salary), max(lunchTime) pivot dept label sum(salary) "PAY"' \
    2100 2100 800 800 750 750 \
    "[13, 0, 0, 0]" "13:00:00.000" \
    "[13, 0, 0, 0]" "13:00:00.000" \
    "[12, 0, 0, 0]" "12:00:00.000"
