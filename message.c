/*	message output

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
#include <stdarg.h>

char *pname = "LaTeXPost";
int verboselevel = NOTE;

void message (int level, const char *fmt, ...)
{
	if	(level <= verboselevel)
	{
		va_list list;

		if	(*fmt == '$')
		{
			fmt++;

			if	(*fmt == '!')
			{
				fputs(pname, stderr);
				fmt++;
			}
		}

		va_start(list, fmt);
		vfprintf(stderr, fmt, list);
		va_end(list);
	}
}
