/*	reading tools for dvi-files
*/

#include "ltxpost.h"
#include "dvi.h"
#include <stdarg.h>

DviFile *df_init (DviFile *df, const char *name, FILE *file)
{
	static DviFile buf;

	if	(df == NULL)	df = &buf;

	memset(df, 0, sizeof(*df));
	df->name = name;
	df->file = file;
	df->ok = 1;
	df->pos = 0;
	return df;
}

void df_fatal (DviFile *df, const char *fmt, ...)
{
	va_list list;

	fprintf(stderr, "%s: %s:%d: ", pname, df->name, df->pos);
	va_start(list, fmt);
	vfprintf(stderr, fmt, list);
	va_end(list);
	putc('\n', stderr);
	df->ok = 0;
}


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

void din_trace (DviFile *df, const char *fmt, ...)
{
	if	(df->trace || verboselevel >= STAT)
	{
		va_list list;
		va_start(list, fmt);
		vfprintf(stderr, fmt, list);
		va_end(list);
	}
}
