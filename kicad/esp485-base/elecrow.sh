#!/bin/sh
rm -rf elecrow
mkdir elecrow
cp *.gtl elecrow/elecrow.gtl
cp *.gbl elecrow/elecrow.gbl
cp *.gto elecrow/elecrow.gto
cp *.gbo elecrow/elecrow.gbo
cp *.gts elecrow/elecrow.gts
cp *.gbs elecrow/elecrow.gbs
cp *.gm1 elecrow/elecrow.gml
cp *.drl elecrow/elecrow.txt
cd elecrow && zip ../elecrow.zip * && cd .. && rm -rf elecrow
