/*	process dvi file
	$Copyright (C) 2002 Erich Fr�hst�ck
	A-3423 St.Andr�/W�rdern, S�dtirolergasse 17-21/5
*/

#include "dvipost.h"
#include "dvi.h"

int process_dvi (FILE *in, FILE *out)
{
	int c;

	c = getc(in);

	if	(c != 2)
	{
		return 1;
	}

	putc(DVI_PRE, out);
	putc(c, out);

	while ((c = getc(in)) != EOF)
		putc(c, out);

	return 0;
}
