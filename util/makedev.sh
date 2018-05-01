#!/bin/sh -
NUMBER=$1
shift
for xx in $@; do
	if test -d "${xx}"; then
		cd ${xx}
		if test -r makedefs; then
			mv -f makedefs makedefs.old
		fi
		cat > makedefs <<ENDOFMAKEDEFS
###########
# Copyright 1991 Regents of the University of California.  All rights reserved.
###########

CFILES		= `echo *.c | xfmt -60 \
	| sed -e 's/\*//g' -e 's/\$/ \\\\/' -e '\$s/ \\\\\$//' \
		-e '2,\$s/^/		  /'`

COBJS		= `echo *.c | xfmt -60 \
	| sed -e 's/\*//g' -e 's/\.c/.o/g' -e 's/\$/ \\\\/' \
		-e '\$s/ \\\\\$//' -e '2,\$s/^/		  /'`

MODULE		= ${xx}
LIBRARY		= dev
MODULE_TARGET	= \$(OBJLIB_DIR)/\$(MODULE)

NUMBER		= ${NUMBER}

ENDOFMAKEDEFS
		echo *.c | xfmt -1 | sed -e 's/^\(.*\)\.c$/\1.o: \1.c/' \
			>> makedefs
		cd ..
	fi
done
