#!/bin/sh
# script to build test files for use in testing the install program

# assumes wget

echo "This script builds test files for use in testing the install program."
echo "Please run this from the install/ directory, so directories are set up"
echo "correctly and in the proper location."
echo "Press Enter to start . . ."

read x

wget -O /tmp/$$.cats http://www.freedos.org/cgi-bin/lsm-export.cgi

for cat in `cat /tmp/$$.cats`; do
    wget -O /tmp/$$.cat.$cat http://www.freedos.org/cgi-bin/lsm-export.cgi?cat=$cat

    mkdir -vp pkgs/$cat

    for f in `cat /tmp/$$.cat.$cat` ; do
	echo $cat $f > pkgs/$cat/$f.txt
    done

    rm -fv /tmp/$$.cat.$cat
done

rm -fv /tmp/$$.cats
