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

void df_trace (DviFile *df, const char *fmt, ...)
{
	if	(df->trace || verboselevel >= STAT)
	{
		va_list list;
		va_start(list, fmt);
		vfprintf(stderr, fmt, list);
		va_end(list);
	}
}
