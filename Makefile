CFLAGS=	-g -Wall -Wmissing-prototypes -D_POSIX_SOURCE

all:: dvipost

OBJ=	dvipost.o

clean::
	rm -f dvipost $(OBJ)

dvipost: $(OBJ)
	$(CC) -o $@ $(OBJ)
