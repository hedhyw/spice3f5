#! /bin/sh -
# for making the "util/delall.sh" script
if test "$#" -eq 0; then
	echo "usage: $0 version"
	echo "        where version is something like '3f2'"
	exit 1
fi
VERSION="$1"
utildir=`echo $0 | sed -e 's|/[^/]*$||'`
cd $utildir
cd ..
find . -type d -print | cat -n | sort -r | \
	sed -e 's/.*	//' -e 's@^\./@@' -e 's@/@\\@g' -e '/^\.$/d' \
	> /tmp/delall.$$
rm -f util/delall.bat
echo "rem   WARNING: this script deletes all files and"	>  util/delall.bat
echo "rem   directories for Spice${VERSION}."		>> util/delall.bat
echo "rem   Hit ^C now if you do not want to do this."	>> util/delall.bat
echo "rem"						>> util/delall.bat
echo 'rem   You will have to hit '"'y'"' for the "del *.*"'>> util/delall.bat
echo 'rem   commands that follow.'			>> util/delall.bat
echo 'pause'						>> util/delall.bat
sed -e 's/^/del /' -e 's/$/\\*.*/' < /tmp/delall.$$	>> util/delall.bat
echo 'del *.*'						>> util/delall.bat
sed -e 's/^/rmdir /' < /tmp/delall.$$			>> util/delall.bat
echo 'rem'						>> util/delall.bat
echo 'rem   All done removing Spice'			>> util/delall.bat
echo Done
