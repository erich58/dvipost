CFLAGS=	-g -Wall -Wmissing-prototypes -D_XOPEN_SOURCE

NAME=	dvipost
ALIAS=	pptex pplatex ltxpost

all:: $(NAME) $(ALIAS) tags test

#	basic functions

HDR=	dvipost.h
BASE=	message.c alloc.c pos.c

$(BASE:.c=.o): $(HDR)

clean::; rm -f $(BASE:.c=.o)

#	dvi specific functions

DVIHDR=	dvi.h dvicmd.h
DVI=	dvifile.c tfm.c din.c dout.c dvifont.c dvi.c dvipost.c

$(DVI:.c=.o): $(HDR) $(DVIHDR)

clean::; rm -f $(DVI:.c=.o)

#	rules for command

main.o: $(HDR) $(DVIHDR)

clean::; rm -f main.o

OBJ=	main.o $(BASE:.c=.o) $(DVI:.c=.o)

$(NAME): $(OBJ); $(CC) $(CFLAGS) -o $@ $(OBJ)

clean::; rm -f $(NAME)

$(ALIAS): $(NAME)
	ln -s $(NAME) $@

clean::; rm -f $(ALIAS)

#	rules for ctags file

SRC=	main.c $(HDR) $(BASE) $(DVIHDR) $(DVI)

tags: $(SRC)
	ctags $(SRC)

clean::; rm -f tags

#	run test

test:: test.dvi test.ps

test.ps test.dvi test.pre test.post: $(NAME) test.tex dvipost.sty
	latex test
	dvitype test.dvi | sed -f filter.sed > test.pre
	$(NAME) -debug test.dvi test.dvi 2> test.post
	dvips -o test.ps test.dvi

purge::; rm -f test.ps test.dvi test.pre test.post

#	additional cleaning rules

purge::; rm -f *.log *.aux
clean:: purge

export:: ltxpost-0.1.tar.gz

ltxpost-0.1.tar.gz:
	cvs export -r V0_1 -d $(@:.tar.gz=) ltxpost
	tar cvzf $@ $(@:.tar.gz=)
	rm -rf $(@:.tar.gz=)

purge::
	rm -f *.tar.gz
