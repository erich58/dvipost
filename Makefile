CFLAGS=	-g -Wall -Wmissing-prototypes -D_XOPEN_SOURCE

all:: ltxpost tags test ps

#	basic functions

HDR=	ltxpost.h
BASE=	message.c alloc.c tfm.c

$(BASE:.c=.o): $(HDR)

clean::; rm -f $(BASE:.c=.o)

#	dvi specific functions

DVIHDR=	dvi.h dvicmd.h
DVI=	dvifile.c din.c dout.c dvifont.c dvi.c

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

test:: test.log ptest.dvi ptest.log

test.dvi: test.tex
	latex test
	rm -f test.log test.aux

FILTER=	sed -e '1,/1 $$/d' -e '/^[[]/d' \
	-e 's/[, ]*hh[:=]*[-0-9][0-9]*//g' \
	-e 's/[, ]*vv[:=]*[-0-9][0-9]*//g' \
	-e 's/ ([0-9]*x[0-9]* pixels)//g' \

test.log: test.dvi filter.sed
	dvitype test.dvi | sed -f filter.sed > $@

purge::; rm -f test.log

ptest.dvi ptest.log: test.dvi ltxpost
	ltxpost -v test.dvi ptest.dvi 2> ptest.log

purge::; rm -f ptest.dvi ptest.log

ps:: ptest.ps

ptest.ps: ptest.dvi; dvips -o $@ ptest.dvi

purge::; rm -f ptest.ps

clean:: purge
