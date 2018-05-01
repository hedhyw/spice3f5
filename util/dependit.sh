#! /bin/sh -
if test $# -lt 3; then
	echo "usage: $0 <depend prog> <include flags> <cfiles ...>"
	exit 1
fi
DP=$1
shift
INCL=$1
shift
while test $# -gt 0; do
	${DP} ${INCL} $1
	shift
done
