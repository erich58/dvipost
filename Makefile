# Makefile to build dvipost
# Copyright (C) 2002 Erich Fruehstueck
#
# Dvipost is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# Dvipost is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public
# License along with dvipost; see the file COPYING.
# If not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

intro::
	@echo "Have a look at the Makefile, before running \`make all'"

install::
	@echo "There is nothing to install, \`make all' does all the work"

# C complier

CC=	gcc
CFLAGS=	-g -Wall -Wmissing-prototypes -D_XOPEN_SOURCE

# Directory to install binaries

#BIN=	$(HOME)/bin
#BIN=	/usr/local/bin
#BIN=	/usr/bin
BIN=	.

all::	$(BIN)
$(BIN):; mkdir -p $@

# Directory to install LaTeX styles

#TEXMF=	/usr/share/texmf
#TEXMF=	/usr/share/texmf.local
#TEXMF=	$(HOME)/texmf
TEXMF=	.

LATEX=	$(TEXMF)/tex/latex/misc

all:: $(LATEX)
$(LATEX):; mkdir -p $@

# Install names

NAME=	$(BIN)/dvipost
ALIAS=	$(BIN)/pptex $(BIN)/pplatex $(BIN)/ltxpost
FILES=	$(LATEX)/dvipost.sty

### End of configuration part ###

# generating rules

all:: $(NAME) $(ALIAS) $(FILES) tags test

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

# install style files

all:: $(FILES)

$(LATEX)/dvipost.sty: dvipost.sty
	cp dvipost.sty $@

clean::; rm -f $(FILES)
clean::; -rmdir -p $(LATEX)

#	run test

test:: test.dvi test.ps

test.ps test.dvi test.pre test.post: $(NAME) test.tex dvipost.sty
	latex test
	dvitype test.dvi | sed -f filter.sed > test.pre
	$(NAME) -debug test.dvi test.dvi 2> test.post
	dvips -o test.ps test.dvi

purge::; rm -f test.ps test.dvi test.pre test.post

# additional cleaning rules

purge::; rm -f *.log *.aux
clean:: purge

### local stuff ###

# rules to create archiv

export:: dvipost-0.8.tar.gz

dvipost-0.8.tar.gz:
	cvs export -r V0_8 -d $(@:.tar.gz=) ltxpost
	sed -e '/local stuff/q' $(@:.tar.gz=)/Makefile > .tmp
	mv .tmp $(@:.tar.gz=)/Makefile
	tar cvzf $@ $(@:.tar.gz=)
	rm -rf $(@:.tar.gz=)

purge::
	rm -f *.tar.gz
