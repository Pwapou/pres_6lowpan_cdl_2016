#sources
SRC= $(wildcard *.*.tex)

ifeq ($(SRC),)
    SRC= $(wildcard *.tex)
endif

IMG= $(wildcard *.dia)


# fichiers generes
DVI= $(SRC:.tex=.dvi)
AUX= $(SRC:.tex=.aux)
LOG= $(SRC:.tex=.log)
NAV= $(SRC:.tex=.nav)
OUT= $(SRC:.tex=.out)
SNM= $(SRC:.tex=.snm)
TOC= $(SRC:.tex=.toc)
VRB= $(SRC:.tex=.vrb)
PDF= $(SRC:.tex=.pdf)
PNG= $(IMG:.dia=.png)

DEPS= $(wildcard *.tex ) ${PNG}

#commandes
RM = rm -f
PDFLATEX = xelatex -shell-escape

all: ${PDF} ${PNG}

%.pdf : %.tex $(DEPS)
	${PDFLATEX} $<
	${PDFLATEX} $<

%.png : %.dia
	dia -e $@ -t cairo-png $<

clean:
	$(RM) \#* *~ $(DVI) $(AUX) $(LOG) $(NAV) $(OUT) $(SNM) $(TOC) $(VRB) $(PNG)

very-clean : clean
	$(RM) $(PDF)
