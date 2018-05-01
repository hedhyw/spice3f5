#!/bin/sh
for xx in $@; do
	if test "x${xx}" = "x-x"; then
		dox=true;
		continue
	fi
	if test -d "${xx}"; then
		cd ${xx}
		if test -r makedefs; then
			mv -f makedefs makedefs.old
		fi
#
#
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

LIBRARY		= ${xx}
LIB_TARGET	= \$(OBJLIB_DIR)/\$(LIBRARY).a
ENDOFMAKEDEFS

#
#

		if test -n "${dox}"; then
			cat >> makedefs <<ENDOFMAKEDEFSX

INCLUDE		= -I$(INCLUDE_DIR) $(INCX)
CFLAGS		= $(CC_OPT) $(SYS_CFLAGS) $(INTERFACE_OPTS)
CONF_DEP_EXTRA	= $(CONF_XCC)
ENDOFMAKEDEFSX
		fi

		echo *.c | xfmt -1 | sed -e 's/^\(.*\)\.c$/\1.o: \1.c/' \
			>> makedefs
		cd ..
	fi
done
