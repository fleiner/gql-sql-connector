#!/bin/bash

set -e -x
export LC_ALL=C

srcdir=${srcdir:-$(dirname $0)}
if [ ! -f cgi-bin/mysql.cgi ] ; then rm -f cgi-bin ; ln -s $srcdir cgi-bin ; fi
if [ ! -f html/chart.html ] ; then rm -f html ; ln -s $srcdir html ; fi

python -u -m CGIHTTPServer 8000 
