/*	process pdf file
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"

int process_pdf (const char *id, FILE *in, FILE *out)
{
	int c;

	message(NOTE, "$!: The PDF-Filter is not yet implemented.\n");
	message(NOTE, "$!: No transformation is perfored on input file.\n");

	while ((c = getc(in)) != EOF)
		putc(c, out);

	return 0;
}
