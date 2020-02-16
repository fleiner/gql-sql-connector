#!/bin/bash

set -e -x
export LC_ALL=C

srcdir=${srcdir:-$(dirname $0)}

$srcdir/postgresqlcheck.py -c 'last,,string,General' \
                      -c 'count(own),,number,General' \
                      -c 'max(own),,number,General' \
                      -c 'min(own),,number,General' \
                      -c 'sum(own),,number,General' \
                      -c 'avg(own),,number,General' \
    -v -d GR -- \
    'select last,count(own),max(own),min(own),sum(own),avg(own) group by last order by min(own) desc options no_format' \
    Bee 2 112 22 134 67 \
    Bear 4 44 10 81 20.25

