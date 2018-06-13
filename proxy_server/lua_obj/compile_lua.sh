#!/bin/bash

SPATH=`pwd`

echo do compile lua scripts in path $SPATH

FILELIST (){
filelist=`ls $SPATH`
for filename in $filelist;
do
	if [ -f $filename -a "${filename##*.}" = "lua" ];
	then
		fullname=$SPATH/$filename
		echo compile File: $fullname
		luajit -b $fullname $fullname
	elif [ -d $filename ];
	then
#		echo Directory: $filename
		cd $filename
		SPATH=`pwd`
#		echo $SPATH
		FILELIST
		cd ..
		SPATH=`pwd`
#	else
#		echo "$SPATH/$filename is not a common file."
	fi
done
}

cd $SPATH
FILELIST
echo "Done."
