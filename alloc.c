/*	memmory allocation

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

void *xalloc (size_t size)
{
	void *p;
	
	if	(size == 0)	return NULL;

	p = malloc(size);

	if	(p == NULL)
	{
		message(ERR, "malloc(%lu) failed.\n", (unsigned long) size);
		exit(EXIT_FAILURE);
	}

	return p;
}

void xfree (void *p)
{
	if	(p)	free(p);
}

char *xcopy (const char *str)
{
	return str ? strcpy(xalloc(strlen(str) + 1), str) : NULL;
}
