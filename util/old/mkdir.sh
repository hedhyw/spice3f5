#!/bin/sh -
if test $# = 0; then
	echo "mkdir [-p] dir"
	exit 0
fi
case "$1" in
	-p)	pflag=1
		shift
		;;
esac
case "$1" in
	/*)	cd /
		;;
esac
IFS='@'
set - `echo "$1" | sed 's^//*^@^g'`
IFS=''
while test "$#" != 0; do
	if test ! -d $1; then
		mkdir $1
	fi
	cd $1
	shift
done
exit 0
