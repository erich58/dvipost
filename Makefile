CFLAGS=	-g -Wall -Wmissing-prototypes -D_POSIX_SOURCE

all:: dvipost

OBJ=	dvipost.o message.o pdf.o dvi.o

$(OBJ): dvipost.h dvi.h

clean::
	rm -f dvipost $(OBJ)

dvipost: $(OBJ)
	$(CC) -o $@ $(OBJ)
