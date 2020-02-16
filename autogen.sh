#!/bin/bash
export ACLOCAL_FLAGS=
export ACLOCAL_AMFLAGS="-I m4"

aclocal $ACLOCAL_FLAGS
libtoolize --copy --force
autoheader
automake --copy --add-missing --foreign
autoconf
