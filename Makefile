CFLAGS=	-g -Wall -Wmissing-prototypes -D_POSIX_SOURCE

all:: ltxpost ctags

HDR=	ltxpost.h dvi.h
OBJ=	ltxpost.o message.o pdf.o dvifile.o dvi.o

$(OBJ): $(HDR)

clean::
	rm -f ltxpost $(OBJ)

ltxpost: $(OBJ)
	$(CC) -o $@ $(OBJ)

ctags: $(OBJ:.c=.h)
	ctags *.c *.h
