PROJ_NAME := $(shell basename `readlink -f .`)
ZIP = $(PROJ_NAME).zip

all: $(ZIP)

$(ZIP): $(PROJ_NAME)-F.Cu.gtl $(PROJ_NAME)-B.Cu.gbl $(PROJ_NAME)-F.SilkS.gto $(PROJ_NAME)-B.SilkS.gbo $(PROJ_NAME)-F.Mask.gts $(PROJ_NAME)-B.Mask.gbs $(PROJ_NAME)-Edge.Cuts.gm1 $(PROJ_NAME).drl
	cp $(PROJ_NAME)-F.Cu.gtl $(PROJ_NAME).gtl
	cp $(PROJ_NAME)-B.Cu.gbl $(PROJ_NAME).gbl
	cp $(PROJ_NAME)-F.SilkS.gto $(PROJ_NAME).gto
	cp $(PROJ_NAME)-B.SilkS.gbo $(PROJ_NAME).gbo
	cp $(PROJ_NAME)-F.Mask.gts $(PROJ_NAME).gts
	cp $(PROJ_NAME)-B.Mask.gbs $(PROJ_NAME).gbs
	cp $(PROJ_NAME)-Edge.Cuts.gm1 $(PROJ_NAME).gml
	cp $(PROJ_NAME).drl $(PROJ_NAME).txt
	zip -m $@ $(PROJ_NAME).gtl $(PROJ_NAME).gbl $(PROJ_NAME).gto $(PROJ_NAME).gbo $(PROJ_NAME).gts $(PROJ_NAME).gbs $(PROJ_NAME).gml $(PROJ_NAME).txt

clean:
	rm -f *.zip *.gtl *.g2 *.g3 *.gbl *.gto *.gbo *.gts *.gbs *.gm1 *.drl *.bak *.kicad_pcb-bak *-cache.lib

