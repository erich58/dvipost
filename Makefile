CFLAGS=	-g -Wall -Wmissing-prototypes -D_POSIX_SOURCE

all:: ltxpost tags

#	basic functions

HDR=	ltxpost.h
BASE=	message.c

$(BASE:.c=.o): $(HDR)

clean::; rm -f $(BASE:.c=.o)

#	dvi specific functions

DVIHDR=	dvi.h dvicmd.h
DVI=	dvifile.c din.c dout.c dviex.c dvifont.c dvi.c

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

ltxpost: $(OBJ); $(CC) -o $@ $(OBJ)

clean::; rm -f ltxpost

#	rules for ctags file

SRC=	ltxpost.c $(HDR) $(BASE) $(DVIHDR) $(DVI) $(PDFHDR) $(PDF)

tags: $(SRC)
	ctags $(SRC)

clean::; rm -f ctags

