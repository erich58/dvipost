CFLAGS=	-g -Wall -Wmissing-prototypes -D_XOPEN_SOURCE

all:: ltxpost tags test

#	basic functions

HDR=	ltxpost.h
BASE=	message.c alloc.c pos.c

$(BASE:.c=.o): $(HDR)

clean::; rm -f $(BASE:.c=.o)

#	dvi specific functions

DVIHDR=	dvi.h dvicmd.h
DVI=	dvifile.c tfm.c din.c dout.c dvifont.c dvi.c

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

clean::; rm -f tags

#	run test

test:: test.dvi test.ps

test.ps test.dvi test.pre test.post: test.tex dvipost.sty
	latex test
	dvitype test.dvi | sed -f filter.sed > test.pre
	ltxpost -v test.dvi test.dvi 2> test.post
	dvips -o test.ps test.dvi

purge::; rm -f test.ps test.dvi test.pre test.post

#	additional cleaning rules

purge::; rm -f *.log *.aux
clean:: purge

