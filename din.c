/*	reading tools for dvi-files
*/

#include "ltxpost.h"
#include "dvi.h"
#include <stdarg.h>


int din_byte (DviFile *df)
{
	int c = getc(df->file);

	if	(c == EOF)
	{
		df_fatal(df, "Unexpected end of file.");
		return 0;
	}

	df->pos++;
	return c;
}


int din_unsigned (DviFile *df, unsigned len)
{
	int val = din_byte(df);

	while (df->ok && --len > 0)
		val = val * 256 + din_byte(df);

	return val;
}


int din_signed (DviFile *df, unsigned len)
{
	int val = din_byte(df);

	if	(val > 127)
		val = 256 - val;

	while (df->ok && --len > 0)
		val = val * 256 + din_byte(df);

	return val;
}


char *din_string (DviFile *df, char *tg, unsigned len)
{
	static char buf[256];
	int n;

	if	(len > 255)
		df_fatal(df, "can't read string of size %d", len);

	if	(tg == NULL)	tg = buf;

	for (n = 0; df->ok && len-- > 0; n++)
		tg[n] = din_byte(df);

	tg[n] = 0;
	return tg;
}
