#!/bin/bash

export LC_ALL=C
srcdir=${srcdir:-$(dirname $0)}

$srcdir/mysqlcheck.py -e -c 'log(int+0),,number,General' -v -d alltypes \
    'select log(`int`+0) ORDER BY `int`' \
    None  0 \
    18.6286954127 18.6287 \
    21.4164130175 21.4164 \
    22.1807097777 22.1807

