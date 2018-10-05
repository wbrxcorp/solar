#!/bin/sh
PCB_NAME=esp485-4layers
rm -rf elecrow
mkdir elecrow
cp $PCB_NAME*.gtl elecrow/$PCB_NAME.gtl
cp $PCB_NAME*.g2 elecrow/$PCB_NAME.gl2
cp $PCB_NAME*.g3 elecrow/$PCB_NAME.gl3
cp $PCB_NAME*.gbl elecrow/$PCB_NAME.gbl
cp $PCB_NAME*.gto elecrow/$PCB_NAME.gto
cp $PCB_NAME*.gbo elecrow/$PCB_NAME.gbo
cp $PCB_NAME*.gts elecrow/$PCB_NAME.gts
cp $PCB_NAME*.gbs elecrow/$PCB_NAME.gbs
cp $PCB_NAME*.gm1 elecrow/$PCB_NAME.gml
cp $PCB_NAME*.drl elecrow/$PCB_NAME.txt
cd elecrow && zip ../$PCB_NAME.zip * && cd .. && rm -rf elecrow
