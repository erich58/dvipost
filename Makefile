CFLAGS=	-g -Wall -Wmissing-prototypes -D_XOPEN_SOURCE

all:: ltxpost tags test

#	basic functions

HDR=	ltxpost.h
BASE=	message.c alloc.c

$(BASE:.c=.o): $(HDR)

clean::; rm -f $(BASE:.c=.o)

#	dvi specific functions

DVIHDR=	dvi.h dvicmd.h tfm.h
DVI=	dvifile.c din.c dout.c dvifont.c dvi.c tfm.c

$(DVI:.c=.o): $(HDR) $(DVIHDR)

clean::; rm -f $(DVI:.c=.o)

#	pdf specific functions

PDFHDR=	
PDF=	pdf.c

$(PDF:.c=.o): $(HDR) $(PDFHDR)

clean::; rm -f $(PDF:.c=.o)

#	rules for post filter

ltxpost.o: $(HDR) $(DVIHDR) $(PDFHDR)

clean::; rm -f ltxpost.o

OBJ=	ltxpost.o $(BASE:.c=.o) $(DVI:.c=.o) $(PDF:.c=.o)

ltxpost: $(OBJ); $(CC) $(CFLAGS) -o $@ $(OBJ)

clean::; rm -f ltxpost

#	rules for ctags file

SRC=	ltxpost.c $(HDR) $(BASE) $(DVIHDR) $(DVI) $(PDFHDR) $(PDF)

tags: $(SRC)
	ctags $(SRC)

clean::; rm -f ctags

#	run test

test:: test.1 test.2

test.dvi: test.tex
	latex test
	rm -f test.log test.aux

test.1: test.dvi; dvitype test.dvi > test.1
test.2: test.dvi ltxpost; ltxpost -v test.dvi 2> test.2

purge::; rm -f test.1 test.2 test.dvi

clean:: purge
