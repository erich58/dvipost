CFLAGS=	-g -Wall -Wmissing-prototypes -D_POSIX_SOURCE

all:: ltxpost

OBJ=	ltxpost.o message.o pdf.o dvi.o

$(OBJ): ltxpost.h dvi.h

clean::
	rm -f ltxpost $(OBJ)

ltxpost: $(OBJ)
	$(CC) -o $@ $(OBJ)
