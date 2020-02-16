#!/bin/bash

set -e -x
export LC_ALL=C

srcdir=${srcdir:-$(dirname $0)}
$srcdir/mysqlcheck.py -c bool,,boolean,true:false -v -d alltypes \
    'select `bool` format `bool` "true:false"' \
    "False" "false" \
    "True" "true" \
    "True" "true" \
    "True" "true"

$srcdir/mysqlcheck.py -c bool,,boolean,yay:nay -v -d alltypes \
    'select `bool` format `bool` "yay:nay"' \
    "False" "nay" \
    "True" "yay" \
    "True" "yay" \
    "True" "yay"

