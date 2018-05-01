#!/bin/sh -
if test $# -le 4; then
	echo '/* This file is generated automatically: do not edit directly */'
fi
echo '/*'
echo ' * Analyses'
echo ' */'
echo '#ifndef TABLES_ONLY'
for xx in $2; do
	echo "#define AN_${xx}"
done
echo '#endif'
echo ''
echo '/*'
echo ' * Devices'
echo ' */'
for xx in $1; do
	echo "#define DEV_${xx}"
done;
echo ''
if test $# -le 4; then
	echo '#define DEVICES_USED' \"$1\"
	echo '#define ANALYSES_USED' \"$2\"
else
	echo '/* DO NOT MODIFY ANYTHING BELOW THIS LINE */'
fi
echo ''
echo "char	Spice_Version[ ] = \"$3\";"
echo "char	Spice_Notice[ ] = \"$4\";"
if test $# -le 4; then
	echo "char	Spice_Build_Date[ ] = \"`date`\";";
else
	echo "char	Spice_Build_Date[ ] = \"\";";
fi
echo ''
exit 0
