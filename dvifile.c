/*	reading tools for dvi-files

$Copyright (C) 2002 Erich Fruehstueck

Dvipost is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

Dvipost is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public
License along with dvipost; see the file COPYING.
If not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/

#include "dvipost.h"
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
	df->last_page = -1;
	df->depth = 0;
	df->mdepth = 0;
	df->npages = 0;
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
