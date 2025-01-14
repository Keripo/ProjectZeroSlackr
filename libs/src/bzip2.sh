#!/bin/sh
#
# bzip2 Auto-Compiling Script
# Created by Keripo
# For Project ZeroSlackr
# Last updated: July 7, 2008
#
echo ""
echo "==========================================="
echo ""
echo "bzip2 Auto-Compiling Script"
echo ""
# Cleanup
if [ -d bzip2 ]; then
	echo "> Removing old bzip2 directory..."
	rm -rf bzip2
fi
# Extract source
echo "> Extracting source..."
tar -xf src/bzip2/bzip2-1.0.5.tar.gz
mv bzip2-1.0.5 bzip2
# Compiling
cd bzip2
echo "> Applying iPod patch..."
patch -p0 -t -i ../src/bzip2/bzip2-Makefile.patch >> build.log
echo "> Compiling..."
export PATH=/usr/local/arm-uclinux-tools2/bin:/usr/local/arm-uclinux-elf-tools/bin:/usr/local/arm-uclinux-tools/bin:$PATH
make install PREFIX=$(pwd) >> build.log
echo ""
cd ..
LIB=bzip2/lib/libbz2.a
if [ -e $LIB ]; then
	echo "Fin!"
else
	echo "Error! Library not compiled. File \"$LIB\" not found!"
fi
echo ""
echo "Auto-Compiling script by Keripo"
echo ""
echo "==========================================="