/*	process pdf file
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "dvipost.h"

int process_pdf (const char *id, FILE *in, FILE *out)
{
	int c;

	putc('%', out);

	while ((c = getc(in)) != EOF)
		putc(c, out);

	return 0;
}
