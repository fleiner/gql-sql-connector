#!/bin/bash

export LC_ALL=C
srcdir=${srcdir:-$(dirname $0)}

$srcdir/mysqlcheck.py -c int,,number,General -v -d alltypes \
    'select `int` ORDER BY `int` LIMIT 2' \
    0 0 123123123 123123123

$srcdir/mysqlcheck.py -c int,,number,General -v -d alltypes \
    'select `int` ORDER BY `int` LIMIT 2 OFFSET 1' \
    123123123 123123123 2000000000 2000000000

$srcdir/mysqlcheck.py -c int,,number,General -v -d alltypes \
    'select `int` ORDER BY `int` OFFSET 2' \
    2000000000 2000000000 4294967295 4294967295
