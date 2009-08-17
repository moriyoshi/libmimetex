#!/bin/sh
libtoolize -c -f -i
aclocal
autoheader
automake -c -a --foreign
autoconf
