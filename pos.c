/*	position table

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

#define	BSIZE	1000

static void pos_expand (PosTab *pos)
{
	if	(pos->size <= pos->dim)
	{
		POS *save = pos->tab;
		pos->size += BSIZE;
		pos->tab = xalloc(pos->size * sizeof(POS));
		memcpy(pos->tab, save, pos->dim * sizeof(POS));
		xfree(save);
	}
}

void pos_init (PosTab *pos)
{
	pos->dim = 0;
}

void pos_add (PosTab *pos, int beg, int end)
{
	if	(beg > end)
	{
		int x = beg;
		beg = end;
		end = x;
	}

	if	(pos->dim)
	{
		POS *last = pos->tab + pos->dim - 1;

		if	(beg <= last->end && end >= last->beg)
		{
			if	(last->end < end)	last->end = end;
			if	(last->beg > beg)	last->beg = beg;

			return;
		}
	}

	pos_expand(pos);
	pos->tab[pos->dim].beg = beg;
	pos->tab[pos->dim].end = end;
	pos->dim++;
}
