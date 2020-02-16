#!/bin/bash

set -e -x

srcdir=${srcdir:-$(dirname $0)}

$srcdir/postgresqlcheck.py -c varchar,,string,General -v -d alltypes \
    'select `varchar` order by `varchar`' \
    '' \
    '01234567890123456789' \
    "hallo äöü अतुल" \
    'hello'

$srcdir/postgresqlcheck.py -c text,,string,General -v -d alltypes \
    'select `text` order by `text`' \
    '' 'bla' 'small' "something
long
and with
UTF 8
äöü"

$srcdir/postgresqlcheck.py -c char,,string,General -v -d alltypes \
    'select `char` order by `char`' \
    '          ' abcdefghij 'blah      ' 'blah      ' 

$srcdir/postgresqlcheck.py -c tinytext,,string,General -v -d alltypes \
    'select `tinytext` order by `tinytext`' \
    '' '' asdadfasdfasdfasdfasdfasdf tttt

$srcdir/postgresqlcheck.py -c mediumtext,,string,General -v -d alltypes \
    'select `mediumtext` order by `mediumtext`' \
    '' '' asdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdf medium
