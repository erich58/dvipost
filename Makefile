CFLAGS=	-g

all:: dvipost

clean::
	rm -f dvipost

OBJ=	dvipost.o

dvipost: $(OBJ)
	$(CC) -o $@ $(OBJ)
