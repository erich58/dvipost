/*	reading tools for dvi-files
*/

#include "ltxpost.h"
#include "dvi.h"
#include <stdarg.h>

DviInput *din_init (DviInput *df, const char *name, FILE *file)
{
	static DviInput buf;

	if	(df == NULL)	df = &buf;

	memset(df, 0, sizeof(*df));
	df->name = name;
	df->file = file;
	df->ok = 1;
	df->pos = 0;
	return df;
}

void din_fatal (DviInput *df, const char *fmt, ...)
{
	va_list list;

	fprintf(stderr, "%s: %s:%d: ", pname, df->name, df->pos);
	va_start(list, fmt);
	vfprintf(stderr, fmt, list);
	va_end(list);
	putc('\n', stderr);
	df->ok = 0;
}


int din_byte (DviInput *df)
{
	int c = getc(df->file);

	if	(c == EOF)
	{
		din_fatal(df, "Unexpected end of file.");
		return 0;
	}

	df->pos++;
	return c;
}


int din_unsigned (DviInput *df, unsigned len)
{
	int val = din_byte(df);

	while (df->ok && --len > 0)
		val = val * 256 + din_byte(df);

	return val;
}


int din_signed (DviInput *df, unsigned len)
{
	int val = din_byte(df);

	if	(val > 127)
		val = 256 - val;

	while (df->ok && --len > 0)
		val = val * 256 + din_byte(df);

	return val;
}


char *din_string (DviInput *df, char *tg, unsigned len)
{
	static char buf[256];
	int n;

	if	(len > 255)
		din_fatal(df, "can't read string of size %d", len);

	if	(tg == NULL)	tg = buf;

	for (n = 0; df->ok && len-- > 0; n++)
		tg[n] = din_byte(df);

	tg[n] = 0;
	return tg;
}

void din_trace (DviInput *df, const char *fmt, ...)
{
	if	(df->trace || verboselevel >= STAT)
	{
		va_list list;
		va_start(list, fmt);
		vfprintf(stderr, fmt, list);
		va_end(list);
	}
}
