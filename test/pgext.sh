#!/bin/bash

export LC_ALL=C
srcdir=${srcdir:-$(dirname $0)}

$srcdir/postgresqlcheck.py -e -c 'log(int+1),,number,General' -v -d alltypes \
    'select log(`int`+1) ORDER BY `int`' \
    0  0 \
    8.09033962631 8.09034 \
    9.30102999588 9.30103 \
    9.63295986125 9.63296

