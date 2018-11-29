#!/bin/sh
DIR=`dirname $0`
ABS_DIR=`readlink -f $DIR`
PROJ_NAME=`basename $ABS_DIR`
rm -rf elecrow
mkdir elecrow
cp *.gtl elecrow/$PROJ_NAME.gtl
cp *.gbl elecrow/$PROJ_NAME.gbl
cp *.gto elecrow/$PROJ_NAME.gto
cp *.gbo elecrow/$PROJ_NAME.gbo
cp *.gts elecrow/$PROJ_NAME.gts
cp *.gbs elecrow/$PROJ_NAME.gbs
cp *.gm1 elecrow/$PROJ_NAME.gml
cp *.drl elecrow/$PROJ_NAME.txt
cd elecrow && zip ../$PROJ_NAME.zip * && cd .. && rm -rf elecrow
