#!/bin/sh
DIR=`dirname $0`
ABS_DIR=`readlink -f $DIR`
PROJ_NAME=`basename $ABS_DIR`
rm -rf elecrow
mkdir elecrow
cp $PROJ_NAME*.gtl elecrow/$PROJ_NAME.gtl
#cp $PROJ_NAME*.g2 elecrow/$PROJ_NAME.gl2
#cp $PROJ_NAME*.g3 elecrow/$PROJ_NAME.gl3
cp $PROJ_NAME*.gbl elecrow/$PROJ_NAME.gbl
cp $PROJ_NAME*.gto elecrow/$PROJ_NAME.gto
cp $PROJ_NAME*.gbo elecrow/$PROJ_NAME.gbo
cp $PROJ_NAME*.gts elecrow/$PROJ_NAME.gts
cp $PROJ_NAME*.gbs elecrow/$PROJ_NAME.gbs
cp $PROJ_NAME*.gm1 elecrow/$PROJ_NAME.gml
cp $PROJ_NAME*.drl elecrow/$PROJ_NAME.txt
cd elecrow && zip ../$PROJ_NAME.zip * && cd .. && rm -rf elecrow
