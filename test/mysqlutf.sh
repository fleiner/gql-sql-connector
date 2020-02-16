#!/bin/bash

set -e -x

srcdir=${srcdir:-$(dirname $0)}

$srcdir/mysqlcheck.py -c varchar,,string,General -v -d alltypes \
    'select `varchar`' \
    '' 'hello' "hallo äöü अतुल" \
    '01234567890123456789'

$srcdir/mysqlcheck.py -c text,,string,General -v -d alltypes \
    'select `text`' \
    '' '<&small>' 'bla' "something
"'"long"'"
and with
UTF 8
äöü"

$srcdir/mysqlcheck.py -c char,,string,General -v -d alltypes \
    'select `char`' \
    '' blah blah abcdefghij

$srcdir/mysqlcheck.py -c tinytext,,string,General -v -d alltypes \
    'select `tinytext`' \
    '' '' tttt asdadfasdfasdfasdfasdfasdf

$srcdir/mysqlcheck.py -c mediumtext,,string,General -v -d alltypes \
    'select `mediumtext`' \
    '' '' medium asdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdf
