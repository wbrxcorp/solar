#!/bin/sh
rm -rf elecrow
mkdir elecrow
mv *.gtl elecrow/elecrow.gtl
mv *.gbl elecrow/elecrow.gbl
mv *.gto elecrow/elecrow.gto
mv *.gbo elecrow/elecrow.gbo
mv *.gts elecrow/elecrow.gts
mv *.gbs elecrow/elecrow.gbs
mv *.gm1 elecrow/elecrow.gml
mv *.drl elecrow/elecrow.txt
cd elecrow && zip ../elecrow.zip * && cd .. && rm -rf elecrow
