/*	writing tools for dvi-files
*/

#include "ltxpost.h"
#include "dvi.h"
#include <stdarg.h>


void dout_byte (DviFile *df, int c)
{
	if	(!df->ok)
	{
		;
	}
	else if	(putc(c, df->file) == EOF)
	{
		df_fatal(df, "Output error.");
	}
	else	df->pos++;
}

void dout_unsigned (DviFile *df, int val, unsigned len)
{
	if	(--len > 0)
		dout_signed(df, val / 256, len);

	dout_byte(df, val);
}


void dout_signed (DviFile *df, int val, unsigned len)
{
	if	(--len > 0)
		dout_signed(df, val / 256, len);

	dout_byte(df, val);
}


void dout_string (DviFile *df, char *buf, unsigned len)
{
	while (len-- > 0)
		dout_byte(df, *(buf++));
}
